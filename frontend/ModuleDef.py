import constants as consts
import sys
import struct
import leb128
from warnings import warn as warn_user
from collections import OrderedDict
from WasmBinaryParser import WasmBinaryParser


class ModuleDef:

	section_names = [
		'',
		'Type',
		'Import',
		'Function',
		'Table',
		'Memory',
		'Global',
		'Export',
		'Start',
		'Element',
		'Code',
		'Data'
	]
	known_section_names = set(section_names[1:])
	
	def __init__(self, filepath, name):
		self.name = name 
		self.sections = OrderedDict()
		self.section_count = 0
		self.most_recent_known_section = -1
		with open(filepath, 'rb') as f:
			ModuleDef._verify_header(f)
			file_data = f.read()
			self.parser = WasmBinaryParser(file_data)
			
		while self.parser.bytes_remaining > 0:
			self._read_section()

	
	@staticmethod
	def _verify_header(module_file):
		header_data = module_file.read(8)
		magic_number, version = WasmBinaryParser(header_data).parse_format('<2I')
		assert magic_number == consts.Module.magic_number
		assert version == consts.Module.version

	def _read_section(self):
		section_id = self.parser.parse_signed_leb128(7)
		assert section_id >= 0 and section_id < 12
		is_known_section = section_id > 0
		payload_size = self.parser.parse_unsigned_leb128(32)
		parser_pos = self.parser.bytes_consumed
		if section_id == 0:
			section_name = self.parser.parse_string()
			assert section_name not in ModuleDef.known_section_names
		elif section_id < len(ModuleDef.section_names):
			section_name = ModuleDef.section_names[section_id]
			if section_id <= self.most_recent_known_section:
				most_recent_name = ModuleDef.section_names[self.most_recent_known_section]
				raise ValueError(('Repeat or out-of-order "known section" '
					'encountered in module {}.  {} (id = {}) section encountered '
					'after {} (id = {}) section')
					.format(self.name, section_name, section_id, most_recent_name, 
						self.most_recent_known_section))
			self.most_recent_known_section = section_id
		else:
			raise ValueError("Bad section id encountered while parsing section {} in module {}."
				.format(self.section_count, self.name))
		payload_size -= (self.parser.bytes_consumed - parser_pos)
		data = self.parser.parse_string(payload_size)
		if is_known_section:
			assert not (section_name in self.sections)
			self.sections[section_name] = data
		elif section_name in self.sections:
			self.sections[section_name] = self.sections[section_name] + (data,)
		else:
			self.sections[section_name] = (data,)
		self.section_count += 1
		
	def __getitem__(self, key):
		return self.sections[key]
	
	def __len__(self):
		return self.section_count

	def get_sections(self):
		return self.values()
	
	def get_known_sections(self):
		return tuple(self.sections.get_default(name, b'') for name in ModuleDef.section_names[1:])


		
		










