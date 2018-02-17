import argparse
import sys
from ProgramDef import ProgramDef
from pathlib import Path
from collections import defaultdict
from warnings import warn
import itertools

class ProgramOptions:

	def __init__(self, parsed_args):
		self.module_pathes = []
		self.all_modules = {}
		self.main = parsed_args.main
		self.include_pathes = [Path(path) for path in parsed_args.include_pathes]
		imports = ProgramOptions._parse_imports(parsed_args.imports)
		for file_name, true_name in parsed_args.imports:
			self._add_module(file_name, true_name)
		if not (self.main in self.all_modules):
			self._add_module(self.main, self.main)
		self._validate_modules()
		
		
	def _add_module(self, file_name, true_name):
		if true_name in self.all_modules:
			raise RuntimeError("Attempt to import module '{}' twice.".format(true_name))
		self.all_modules[true_name] = []
		here = Path('.')
		if file_name == self.main:
			self.main = true_name
		full_path = here / file_name
		if full_path.is_file():
			self.all_modules[true_name].append(full_path)
		for include_path in self.include_pathes:
			full_path = here / file_name
			if full_path.is_file():
				self.all_modules[true_name].append(full_path)

	@staticmethod	
	def _parse_imports(import_names):
		imports = []
		is_rename = False
		for name in import_names:
			if is_rename:
				imports[-1] = (imports[-1], name)
				is_rename = False
			elif name == 'as':
				is_rename = True
			else:
				imports[-1] = (imports[-1], imports[-1])
				imports.append(name)
		if is_rename:
			raise RuntimeError('Incomplete import "{} as" needs name after "as".'.format(imports[-1]))
		elif imports and (not isinstance(imports[-1], tuple)):
			imports[-1] = (imports[-1], imports[-1])
		return imports
	
	def _validate_modules(self, warn_bad_files = True):
		for module_name, pathes in self.all_modules.items():
			invalid_files = tuple(path for path in pathes if not ProgramOptions._is_module_file(path))
			pathes[:] = (path for path in pathes if ProgramOptions._is_module_file(path))
			if len(pathes) > 1:
				raise RuntimeError("Conflicting module imports with name '{}': {}"
					.format(module_name, tuple(pathes)))
			elif len(pathes) == 0:
				raise RuntimeError("No valid WebAssembly modules files found for "
					"module with name '{}'.  The following files were found but "
					"are not valid .wasm files: {}".format(module_name, invalid_files))
			elif warn_bad_files and invalid_files:
				warn("The following file were found for module '{}' but will be ignored "
					"because they are not valid .wasm files: {}".format(module_name, invalid_files))
			assert len(pathes) == 1
			self.all_modules[module_name] = pathes[0]
		
				
	@staticmethod
	def _is_module_file(path):
		with open(path, 'rb') as wasm_module:
			return wasm_module.read(4) == b'\0asm'
		
				


def make_argument_parser():
	def to_bytes(string):
		return string.encode('utf8')
	parser = argparse.ArgumentParser(
		prog = 'wasm-cpp',
		usage = 'TODO',
	)
	parser.add_argument('main', type=str)
	parser.add_argument('-I', '--include-pathes', nargs='+', type=to_bytes, default=[])
	parser.add_argument('-i', '--imports', nargs='+', type=to_bytes, default = [])
	parser.add_argument('-F', '--full-path-names', default=False, action='store_const', const=True)
	parser.add_argument('-S', '--strip-suffix', default=False, action='store_const', const=True)
	return parser

def make_program(parser):
	options = ProgramOptions(parser)
	all_modules = options.all_modules
	modules = ((all_modules[options.main], options.main),)
	modules += tuple((path, name) for name, path in all_modules.items() if name != options.main)
	return ProgramDef(*modules)
	

def main():
	parser = make_argument_parser()
	result = parser.parse_args(sys.argv[1:])
	program = make_program(result)
	program._define_program()


if __name__ == '__main__':
	main()
