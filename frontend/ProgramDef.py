from ModuleDef import ModuleDef
from WasmBinaryParser import WasmBinaryParser, finalized_code
from IdDict import IdDict
from collections import defaultdict, namedtuple
import constants as consts
import array as array

class ResizableLimits:
	
	def __init__(self, initial_size, maximum_size = None):
		self._initial_size = initial_size
		self._maximum_size = maximum_size

	def __eq__(self, other):
		return (isinstance(other, ResizableLimits)
			and self.initial_size == other.initial_size
			and self.maximum_size == other.maximum_size)

	@property	
	def initial_size(self):
		return self._initial_size

	@property	
	def maximum_size(self):
		return self._maximum_size


class Importable:

	def __init__(self, truetype, tp):
		assert isinstance(tp, truetype)
		self._type = tp
	
	@property	
	def type(self):
		return self._type

	def assign_from(self, other):
		assert type(self) == type(other)
		members = vars(other)
		members.pop('_type')
		for member, value in vars(other):
			setattr(self, member, value)
		return self
	
class Function(Importable):

	class Type:
		
		def __init__(self, signature_id):
			self._signature = signature_id
		
		def __eq__(self, other):
			return (isinstance(other, Function.Type) 
				and self.signature == other.signature)

		@property
		def signature(self):
			return self._signature
	
		@property
		def kind(self):
			return 3


	def __init__(self, tp, locals_types = None, code = None):
		super(Function, self).__init__(Function.Type, tp)
		self.locals_types = locals_types
		self.code = code
	
	@property	
	def kind(self):
		return 0

	def define(self, locals_types, code):
		assert self.locals_types is None
		assert self.code is None
		self.locals_types = bytes(bytearray(locals_types))
		self.code = bytes(code)

	def serialize(self):
		fmt = "=L=L{}s".format(len(self.code))
		return struct.pack(self.type.signature, len(self.locals_types), self.code)
		

class Table(Importable):

	class Type(ResizableLimits):
		
		def __init__(self, elem_type, initial_size, max_size = None):
			super(Table.Type, self).__init__(initial_size, max_size)
			self._elem_type = elem_type
		
		def __eq__(self, other):
			return (isinstance(other, Table.Type) 
				and self.elem_type == other.elem_type
				and super(Table.Type, self) == super(Table.Type, other))

		@property
		def elem_type(self):
			return self._elem_type

		@property	
		def kind(self):
			return 1

	def __init__(self, tp):
		super(Table, self).__init__(Table.Type, tp)
		self.table = [None] * self.type.initial_size

	@property	
	def kind(self):
		return 1

	def initialize_segment(self, offset, length, values):
		assert len(table) >= (offset + length)
		assert offset >= 0
		assert all(elem is None for elem in self.table[offset: offset + length])
		self.table[offset: offset + length] = values
	
	def serialize(self):
		serialized_elems = array.array('q', ((-1 if item is None else item) for item in self.table)).tobytes()
		fmt = "=qb{}s".format(len(serialized_elems))
		maxm_size = self.type.maximum_size
		if maxm_size is None:
			maxm_size = -1
		return struct.pack(fmt, maxm_size, self.type.elem_type, serialized_elems)
		



class Memory(Importable):
	
	class Type(ResizableLimits):
		
		def __init__(self, *args, **kwargs):
			super(Memory.Type, self).__init__(*args, **kwargs)
		
		@property	
		def kind(self):
			return 2

	def __init__(self, tp):
		super(Memory, self).__init__(Memory.Type, tp)
		page_size = consts.LinearMemory.page_size 
		self.memory = bytearray(self.type.initial_size * page_size)
	
	@property	
	def kind(self):
		return 2

	def initialize_segment(self, offset, length, values):
		assert len(table) >= (offset + length)
		assert offset >= 0
		# TODO: a more rigorous check for overlapping initializers
		assert all((elem == 0) for elem in self.memory[offset: offset + length])
		self.memory[offset: offset + length] = values

	def serialize(self):
		fmt = "=q{}s".format(len(self.memory))
		maxm_size = self.type.maximum_size
		if maxm_size is None:
			maxm_size = -1
		return struct.pack(self.type.initial_size, maxm_size, len(self.memory), self.memory)


class Global(Importable):
	
	class Type:
		
		def __init__(self, typecode, mutable):
			assert typecode in consts.LanguageType.value_types
			self._typecode = typecode
			self._mutable = mutable
			
		
		def __eq__(self, other):
			return (isinstance(other, Global.Type) 
				and self.typecode == other.typecode
				and self.mutable == other.mutable)

		@property	
		def typecode(self):
			return self._typecode
		
		@property	
		def mutable(self):
			return self._mutable

		@property	
		def kind(self):
			return 3


	def __init__(self, tp, initial_value = None):
		super(Global, self).__init__(Global.Type, tp)
		self._initial_value = initial_value
		self.__deps = []

	def define(self, value):
		assert isinstance(tp, Global.Type)
		assert LanguageType.matches(self.type.typecode, value)
		assert self.initial_value is None
		assert isinstance(value, bytes) or LanguageType.matches(self.type.typecode, value)
		self.initial_value = value
		for other_global in self.__deps:
			other_global.define(self.initial_value)
		self.__deps.clear()

	def try_define(self, value_or_global):
		assert self.initial_value is None
		if isinstance(value_or_global, Global):
			assert self.type == value_or_global.type
			assert value_or_global is not self
			other_global.add_dependency(self)
			return False
		else:
			self.define(initial_value)
			self.initial_value = value_or_global
			return True
			

	def add_dependency(self, other_global):
		if self.initial_value is None:
			self.__deps.append(other_global)
		else:
			other_global.define(self.type, self.initial_value)
	
	@property	
	def kind(self):
		return 3

	@property
	def initial_value(self):
		if self._initial_value is None:
			raise AttributeError('"initial_value" attribute in Global instance has not been initialized yet.')
		else:
			return initial_value
	
	def assign_from(self, other):
		assert self.type == other.type
		assert self.initial_value is None
		self._initial_value = other.initial_value
		if isinstance(self.initial_value, Global):
			self.initial_value.add_dependency(self)

	_typecode_map = {
		-0x01: (b'l', b'L'),
		-0x02: (b'q', b'Q'),
		-0x03: b'f',
		-0x01: b'd',
	}	
	def serialize(self):
		fmt = b"=?=c={}"
		fmt_chr = Global._typecode_map[self.type.typecode]
		if isinstance(fmt_chr, tuple):
			assert isinstance(self.initial_value, int)
			if self.initial_value < 0:
				fmt_chr = fmt_chr[0]
			else:
				fmt_chr = fmt_chr[1]
		else:
			assert isinstance(self.initial_value, float)
		
		fmt.format(fmt_chr)
		return struct.pack(self.type.mutable, fmt_chr, self.initial_value)


class Module:

	def __init__(self, module_def, program_def):
		self.module_def = module_def
		self._program_def = program_def
		self.function_signatures = None
		self.functions = [] 
		self.tables = []
		self.memories = []
		self.globals = []
		self.index_spaces = (
			self.functions,
			self.tables,
			self.memories,
			self.globals
		)
		self.import_counts = array.array('L', (0, 0, 0, 0))

	def __repr__(self):
		return ('Module(name={}, num_function_signatures={}, '
			'num_functions={}, num_tables={}, num_memories={} '
			'num_globals={})'.format(self.module_def.name,
				len(self.function_signatures), 
				len(self.functions), 
				len(self.tables), 
				len(self.memories), 
				len(self.globals),
			)
		)

	@property
	def function_import_count(self):
		return self.import_counts[0]

	@property
	def table_import_count(self):
		return self.import_counts[1]

	@property
	def memory_import_count(self):
		return self.import_counts[2]

	@property
	def global_import_count(self):
		return self.import_counts[3]

	def add_import(self, imported_value):
		self.index_spaces[imported_value.kind].append(imported_value)
		self.import_counts[imported_value.kind] += 1

	def get_index(self, kind, index):
		return self.index_spaces[kind][index]

	def set_index(self, kind, index, value):
		self.index_spaces[kind][index] = value

	@property
	def name(self):
		return self.module_def.name

	def __getitem__(self, section_name):
		try:
			return self.module_def[section_name]
		except KeyError:
			return b''



class ProgramDef:
	
	Types = (Function, Table, Memory, Global)
	IdMapTuple = namedtuple("IdMapTuple", "functions, tables, memories, globals")
	__init_expr_opcodes = {
		consts.InitExpr.get_global: (lambda parser, module: module.globals[parser.parse_unsigned_leb128(32)]), 
		consts.InitExpr.i32_const:  (lambda parser, module: parser.parse_signed_leb128(32)), 
		consts.InitExpr.i64_const:  (lambda parser, module: parser.parse_signed_leb128(64)), 
		consts.InitExpr.f32_const:  (lambda parser, module: parser.parse_format('f')), 
		consts.InitExpr.f64_const:  (lambda parser, module: parser.parse_format('d')), 
	}
	


	def __init__(self, main_module, *imported_modules):
		self.modules = ((ModuleDef(*main_module),) + 
			tuple(ModuleDef(*module) for module in imported_modules))
		self.modules = tuple(Module(module_def, self) for module_def in self.modules)
		self.module_names = {module.name for module in self.modules}
		self.function_signatures = IdDict()
		self.exports = defaultdict(lambda: defaultdict(None))
		self.imports = self.exports
		self.global_variables = []
		self.start_function = None
		self.id_maps = ProgramDef.IdMapTuple(IdDict(), IdDict(), IdDict(), IdDict())


	@property 
	def main(self):
		return self.modules[0]


	def _define_program(self):
		initialize_sequence = (
			ProgramDef._read_type_section,
			ProgramDef._read_import_section,
			ProgramDef._read_function_section,
			ProgramDef._read_table_section,
			ProgramDef._read_memory_section,
			ProgramDef._read_global_section,
			ProgramDef._read_element_section,
			ProgramDef._read_data_section
		)
		finalize_sequence = (
			ProgramDef._read_code_section,
		)
		for read_section in initialize_sequence:
			for module in self.modules:
				read_section(self, module)
		self._read_start_section()
		self._map_modules()
		for read_section in finalize_sequence:
			for module in self.modules:
				read_section(self, module)

	def _get_section_parser(self, module, section_name):
		data = module[section_name]
		if not data:
			return None
		else:
			return WasmBinaryParser(data)

	def _read_type_section(self, module):
		assert (module.function_signatures is None)
		parser = self._get_section_parser(module, "Type")
		if parser is not None:
			count = parser.parse_unsigned_leb128(32)
			get_sig = lambda: self.function_signatures[parser.parse_func_type()]
			module.function_signatures = tuple(get_sig() for _ in range(count))
		else:
			module.function_signatures = tuple()

	def _read_import_section(self, module):
		parser = self._get_section_parser(module, "Import")
		if parser is not None:
			count = parser.parse_unsigned_leb128(32)
			for _ in range(count):
				module_name = parser.parse_string()
				field_name = parser.parse_string()
				kind = parser.parse_format('B')
				typedef = self._read_import_type(module, parser, kind)
				self._add_module_import(module, module_name, field_name, typedef)

	def _read_import_type(self, module, parser, kind):
		if kind == consts.Kind.Function:
			return Function.Type(module.function_signatures[parser.parse_unsigned_leb128(32)])
		elif kind == consts.Kind.Table:
			return Table.Type(*parser.parse_table_type())
		elif kind == consts.Kind.Memory:
			return Table.Type(*parser.parse_memory_type())
		elif kind == consts.Kind.Global:
			return Global.Type(*parser.parse_global_type())
		else:
			raise ValueError("Bad 'external_kind' (={}) encountered.".format(kind))
	
	def _add_module_import(self, module, module_name, field_name, typdef):
		default = ProgramDef.Types[typedef.kind](typedef)
		module_imports = self.imports[module_name]
		sz = len(module_imports)
		actual = module_imports.set_default(field_name, default)
		if defualt.type != actual.type:
			raise ValueError("Mismatch WASM types in imported name {}.{}.".format(module_name, field_name))
		module.add_import(actual)

	def _read_function_section(self, module):
		parser = self._get_section_parser(module, "Function")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		assert module.function_import_count == len(module.functions)
		funcs = module.functions
		sigs = module.function_signatures
		sz = len(funcs)
		# extend functions list
		make_func = lambda: Function(Function.Type(sigs[parser.parse_unsigned_leb128(32)]))
		funcs[sz:sz + count] = (make_func() for _ in range(count))
	
	def _read_table_section(self, module):
		parser = self._get_section_parser(module, "Table")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		assert module.table_import_count == len(module.tables)
		tables = module.tables
		sz = len(tables)
		make_table = lambda: Table(Table.Type(*parser.parse_table_type()))
		tables[sz:sz + count] = (make_table() for _ in range(count))
	
	def _read_memory_section(self, module):
		parser = self._get_section_parser(module, "Memory")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		assert module.memory_import_count == len(module.memories)
		memories = module.memories
		sz = len(memories)
		make_memory = lambda: Memory(Memory.Type(*parser.parse_memory_type()))
		memories[sz:sz + count] = (make_memory() for _ in range(count))
	
	def _read_global_section(self, module):
		parser = self._get_section_parser(module, "Global")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		assert module.global_import_count == len(module.globals)
		globs = module.globals
		sz = len(globs)
		def make_global():
			tp = Global(Global.Type(parser.parse_global_type()))
			init = parser.parse_initializer_expression()
			value = self._eval_init_expr(module, init)
			return Global(tp, initial_value=value)
		globs[sz:sz + count] = (make_global() for _ in range(count))
		
		
	def _eval_init_expr(self, module, expr):
		parser = WasmBinaryParser(expr)
		opcode = parser.parse_format('B')
		value = ProgramDef.__init_expr_opcodes[opcode](parser, module)
		# initializer has to end with the 'END' instruction
		assert parser.parse_format('B') == InitExpr.end
		try:
			value = value.initial_value
		except AttributeError:
			pass
		return value
		
	def _read_export_section(self, module):
		
		parser = self._get_section_parser(module, "Export")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		for i in range(count):
			field_name = parser.parse_string()
			kind = parser.parse_format('B')
			index = parser.parse_unsigned_leb128(32)
			self.resolve_import(module, field_name, kind, index)

	def _resolve_export(self, module, field_name, kind, index):
		
		default = module.get_index(kind, index)
		actual = self.exports[module.name].set_default(field_name, default)
		if actual is not default:
			actual.assign_from(default)
			module.set_index(kind, index, actual)
	
	def _read_start_section(self):
		parser = self._get_section_parser(self.modules[0], "Start")
		if parser is None:
			raise ValueError("Main module {} has no start function!".format(self.modules[0].name))
		self.start_function = self.modules[0].functions[parser.parse_unsigned_leb128(32)]
		
	def _read_element_section(self, module):
		parser = self._get_section_parser(module, "Element")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		for i in range(count):
			index = parser.parse_unsigned_leb128(32)
			offset = parser.parse_initializer_expression()
			offset = self._eval_init_expr(offset)
			segment_len = parser.parse_unsigned_leb128(32)
			indices = (parser.parse_unsigned_leb128(32) for _ in range(segment_len))
			functions = (module.functions[ind] for ind in indices)
			module.tables[index].initialize_segment(offset, segment_len, functions)


	def _read_code_section(self, module):
		parser = self._get_section_parser(module, "Code")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		for i in range(count):
			body_size = parser.parse_unsigned_leb128(32)
			body_parser = WasmBinaryParser(parser.view[:body_size])
			assert parser.view[body_size - 1] == consts.Opcode.END
			local_entries = body_parser.parse_unsigned_leb128(32)
			locals_types = bytearray()
			for j in range(local_entries):
				nlocals = body_parser.parse_unsigned_leb128(32)
				sz = len(locals_types)
				typecode = body_parser.parse_signed_leb128(7)
				if not (typecode in consts.LanguageType.value_types):
					raise ValueError("Invalid 'value_type' (typecode={}) encountered "
						"while parsing function local variables definition.".format(hex(typecode)))
				locals_types[sz: sz + nlocals] = (typecode for _ in range(nlocals))
			code, count = finalized_code(body_parser.remaining_data, module)
			offset = module.functions[i + module.function_import_count]
			self.functions[offset].define(locals_types, code)
			parser.ignore(body_size)
			
	def _read_data_section(self, module):
		parser = self._get_section_parser(module, "Data")
		if parser is None:
			return None
		count = parser.parse_unsigned_leb128(32)
		for i in range(count):
			index = parser.parse_unsigned_leb128(32)
			offset = parser.parse_initializer_expression()
			offset = self._eval_init_expr(offset)
			segment_len = parser.parse_unsigned_leb128(32)
			data = parser.parse_string(segment_len)
			module.memories[index].initialize_segment(offset, segment_len, data)

	def _map_modules(self):
		all_functions = IdDict()
		all_tables = IdDict()
		all_memories = IdDict()
		all_globals = IdDict()
		for module in self.modules:
			module.functions = array.array('L', (all_functions[f] for f in module.functions))
			module.tables = array.array('L', (all_tables[t] for t in module.tables))
			module.memories = array.array('L', (all_memories[m] for m in module.memories))
			module.globals = array.array('L', (all_globals[g] for g in module.globals))
		
		self.start_function = all_functions[self.start_function]
		self.functions = tuple(sorted(all_functions.keys(), key=lambda k: all_functions[k]))
		del all_functions
		self.tables = tuple(sorted(all_tables.keys(), key=lambda k:all_tables[k]))
		del all_tables
		self.memories = tuple(sorted(all_memories.keys(), key=lambda k:all_memories[k]))
		del all_memories
		self.globals = tuple(sorted(all_globals.keys(), key=lambda k:all_globals[k]))
		del all_globals
		
		# TODO: something better than this...
		# map module-local table elements (function indices) to their program-wide offsets
		for module in self.modules:
			for table_index in module.tables:
				table = self.tables[table_index].table
				for i, offset in enumerate(table):
					if offset is not None:
						table[i] = module.functions[offset]



	
