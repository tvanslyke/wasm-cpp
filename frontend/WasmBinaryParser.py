import leb128
import constants as consts
from constants import Opcode as ops
from collections import defaultdict
import struct
import array

class WasmBinaryParser:
	
	def __init__(self, data):
		self.data = data
		assert isinstance(data, bytes)
		self.view = data[0:]

	@property	
	def total_bytes(self):
		return len(self.data)
	
	@property	
	def bytes_remaining(self):
		return len(self.view)

	@property	
	def bytes_consumed(self):
		return self.total_bytes - self.bytes_remaining
	
	def _advance(self, count):
		self.view = self.view[count:]
		
	def parse_byte(self):
		value = self.view[0]
		self._advance(1)
		return value
	
	def parse_signed_leb128(self, width):
		value, count = leb128.read_signed(self.view, width)
		self._advance(count)
		return value
	
	def parse_unsigned_leb128(self, width):
		value, count = leb128.read_unsigned(self.view, width)
		self._advance(count)
		return value
	
	def parse_resizable_limits(self):
		has_maximum = self.parse_unsigned_leb128(1)
		assert has_maximum in {0, 1}
		has_maximum = bool(has_maximum)
		initial_size = self.parse_unsigned_leb128(32)
		maximum = None
		if has_maximum:
			maximum = self.parse_unsigned_leb128(32)
			assert maximum >= initial_size
		return initial_size, maximum
	
	def parse_string(self, size = None):
		if size is None:
			size = self.parse_unsigned_leb128(32)
		assert len(self.view) >= size
		value = self.view[:size]
		self._advance(size)
		return value

	def parse_array(self, parse_func, size = None):
		if size is None:
			size = self.parse_unsigned_leb128(32)
		return [parse_func(self) for _ in range(size)]

	def parse_memory_type(self):
		# TODO: Is multiplying by page_size here correct?
		init, maxm = self.parse_resizable_limits()
		init *= consts.LinearMemory.page_size
		if maxm:
			maxm *= consts.LinearMemory.page_size
		return init, maxm

	def _parse_elem_type(self):
		value, count = consts.LanguageType.parse_type(self.view)
		self._advance(count)
		assert value == consts.LanguageType.anyfunc
		return value
	
	def parse_table_type(self):
		typecode = self._parse_elem_type()
		init, maxm = self.parse_resizable_limits()
		return (typecode, init, maxm)

	def parse_func_type(self):
		typecode, count = consts.LanguageType.parse_type(self.view)
		assert typecode == consts.LanguageType.func
		self._advance(count)
		parse_fn = lambda parser: parser.parse_signed_leb128(7)
		param_types = self.parse_array(parse_fn)
		# return_types = self.parse_array(parse_fn)
		return_count = self.parse_unsigned_leb128(1)
		assert return_count in {0, 1}
		return_types = []
		if return_count:
			return_types.append(self.parse_signed_leb128(7))
		for tp in param_types:
			assert tp in consts.LanguageType.value_types
		for tp in return_types:
			if tp not in consts.LanguageType.value_types:
				raise ValueError("Invalid 'value_type' (typecode={}) encountered "
					"while parsing function signature.".format(hex(tp)))
		param_types = struct.pack('={}b'.format(len(param_types)), *param_types)
		return_types = struct.pack('={}b'.format(len(return_types)), *return_types)
		return param_types, return_types
		
	def parse_global_type(self):
		content_type = consts.LanguageType.parse_value_type(self.view)
		mutable = parse_unsigned_leb128(1)
		assert mutable in {0, 1}
		mutable = bool(mutable)
		return content_type, mutable

	def parse_external_kind(self):
		kind = self.parse_unsigned_leb128(8)
		if kind < 0 or kind > 3:
			raise ValueError("invalid external_kind value encountered: {}".format(kind))
		return kind

	def parse_struct(self, struct_object):
		sz = struct_object.size
		result = struct_object.unpack_from(self.view)
		self._advance(sz)
		return result
		
	def parse_structs(self, struct_object, count):
		sz = struct_object.size
		result = [struct_object.unpack_from(self.view) for i in range(count)]
		self._advance(sz * count)
		return result


	def parse_format(self, struct_format, count = None):
		if count is None:
			return self.parse_struct(struct.Struct(struct_format))
		else:
			return self.parse_structs(struct.Struct(struct_format), count)
		
	def parse_initializer_expression(self):
		count = 0
		while self.view[count] != consts.InitExpr.end:
			count += 1
		return self.parse_string(count)

	def peek(self):
		return self.view[0]
	
	def ignore(self, count):
		self.view = self.view[count:]

	@property
	def remaining_data(self):
		return self.view


class ImmediatesParser:
	
	wasm_uint64 = struct.Struct('=Q')
	wasm_sint64 = struct.Struct('=q')
	wasm_float64 = struct.Struct('=d')
	wasm_uint32 = struct.Struct('=L')
	wasm_sint32 = struct.Struct('=l')
	wasm_float32 = struct.Struct('=f')
	wasm_ubyte = struct.Struct('=B')
	wasm_sbyte = struct.Struct('=b')
	wasm_bool = wasm_sbyte

	def __init__(
		self, 
		code, 
		module_function_signatures,
		module_functions, 
		module_tables, 
		module_memories, 
		module_globals
	):
		assert code[-1] == ops.END
		self.code_src = code
		self.code_dest = array.array('B')
		self.parser = WasmBinaryParser(self.code_src)
		self.function_signatures = module_function_signatures
		self.functions = module_functions
		self.tables = module_tables
		self.memories = module_memories
		self.globals = module_globals
		self.labels = 0

	def parse(self):
		opcode = ops.NOP
		while self.parser.bytes_remaining > 0:
			opcode = self.parser.parse_byte()
			self.code_dest.append(opcode)
			ImmediatesParser.handlers[opcode](self)
			
		assert self.code_dest[-1] == ops.END
		return self.code_dest, self.parser.bytes_consumed

	def _leb128_uint1(self):
		value = self.parser.parse_unsigned_leb128(1)
		self.code_dest.extend(ImmediatesParser.wasm_bool.pack(value))
		return value

	def _leb128_uint7(self):
		value = self.parser.parse_unsigned_leb128(7)
		self.code_dest.extend(ImmediatesParser.wasm_ubyte.pack(value))
		return value

	def _leb128_uint32(self):
		value = self.parser.parse_unsigned_leb128(32)
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
	
	def _leb128_uint64(self):
		value = self.parser.parse_unsigned_leb128(64)
		self.code_dest.extend(ImmediatesParser.wasm_uint64.pack(value))
		return value
	
	def _leb128_sint7(self):
		value = self.parser.parse_signed_leb128(7)
		self.code_dest.extend(ImmediatesParser.wasm_sbyte.pack(value))
		return value

	def _leb128_sint32(self):
		value = self.parser.parse_signed_leb128(32)
		self.code_dest.extend(ImmediatesParser.wasm_sint32.pack(value))
		return value
	
	def _leb128_sint64(self):
		value = self.parser.parse_signed_leb128(64)
		self.code_dest.extend(ImmediatesParser.wasm_sint64.pack(value))
		return value
	
	def _float32(self):
		value = self.parser.parse_struct(ImmediatesParser.wasm_float32)
		self.code_dest.extend(ImmediatesParser.wasm_float32.pack(value))
		return value
	
	def _float64(self):
		value = self.parser.parse_struct(ImmediatesParser.wasm_float64)
		self.code_dest.extend(ImmediatesParser.wasm_float64.pack(value))
		return value
	
	def _index(self):
		return self._leb128_uint32()

	def _map_index(self, mapping):
		index = self._index()
		index = mapping[index]
		self.code_dest[-4:] = array.array('B', struct.pack('=L', index))
		return index
	
	def _function_signature_index(self):
		return self._map_index(self.function_signatures)
		
	def _function_index(self):
		return self._map_index(self.functions)
		
	def _table_index(self):
		return self._map_index(self.tables)
		
	def _memory_index(self):
		return self._map_index(self.memories)
		
	def _global_index(self):
		return self._map_index(self.globals)
		
	def _reserved(self):
		return self._leb128_uint1()

	def _push_label(self):
		self.labels += 1

	def _pop_label(self):
		self.labels -= 1

	def _block(self):
		self._leb128_sint7()
		self._push_label()
		
	def _branch_table(self):
		count = self._leb128_uint32()
		for _ in range(count):
			self._leb128_uint32()
		# once more for the default case
		self._leb128_uint32()

	def _call_indirect(self):
		self._function_signature_index()
		self._reserved()

	def _load_store(self):
		self._leb128_uint32()
		self._leb128_uint32()

	def _default(self):
		pass
	


class CodeParser:
	

	immediates_sizes = defaultdict(int, {
		ops.BLOCK: 		1, 
		ops.LOOP: 		1, 
		ops.IF: 		1, 
		ops.BR: 		4,
		ops.BR_IF: 		4,
		ops.BR_TABLE: 		None,
		ops.CALL: 		4, 
		ops.CALL_INDIRECT: 	4, 
		ops.GET_LOCAL: 		4, 
		ops.SET_LOCAL: 		4, 
		ops.TEE_LOCAL: 		4, 
		ops.GET_GLOBAL:		4, 
		ops.SET_GLOBAL:		4, 

		ops.I32_LOAD: 		8, 
		ops.I64_LOAD: 		8, 
		ops.F32_LOAD: 		8, 
		ops.F64_LOAD: 		8, 

		ops.I32_LOAD8_S:	8, 
		ops.I32_LOAD8_U:	8, 
		ops.I32_LOAD16_S:	8, 
		ops.I32_LOAD16_U:	8, 
		
		ops.I64_LOAD8_S:	8, 
		ops.I64_LOAD8_U:	8, 
		ops.I64_LOAD16_S:	8, 
		ops.I64_LOAD16_U:	8, 
		ops.I64_LOAD32_S:	8, 
		ops.I64_LOAD32_U:	8, 
		
		ops.I32_STORE:		8, 
		ops.I64_STORE:		8, 
		ops.F32_STORE:		8, 
		ops.F64_STORE:		8, 
		
		ops.I32_STORE8:		8, 
		ops.I32_STORE16:	8, 
		ops.I64_STORE8:		8, 
		ops.I64_STORE16:	8, 
		ops.I64_STORE32:	8, 
	
		ops.I32_CONST: 		4,
		ops.I64_CONST:		8,
		ops.F32_CONST:		4,
		ops.F64_CONST:		8,

		ops.CURRENT_MEMORY:	1,
		ops.GROW_MEMORY:	1,
	})

	label_struct = struct.Struct('=L')
	index_struct = struct.Struct('=L')

	def __init__(self, code):
		self.label_stack = []
		self.code = code
		assert self.code[-1] == ops.END
		self.index = 0
		
	def parse(self):
		while self.index < len(self.code):
			opcode = self.code[self.index]
			try:
				CodeParser.handlers[opcode](self, opcode)
			except IndexError:
				assert len(self.code) - 1 ==  self.index
				break
		return self.code
	
	def _insert_unbound_label(self):
		self.code[self.index: self.index] = array.array('B', (0, 0, 0, 0))
		self.label_stack.append(self.index)

	def _insert_bound_label(self, pos):
		self.code[self.index: self.index] = array.array('B', CodeParser.label_struct.pack(pos))
		self.label_stack.append(None)

	def _bind_label_at(self, at, to):
		if any(self.code[at:at + 4]):
			raise RuntimeError("Internal Error: Attempt to bind label that has already been bound (preceding opcode is {}).".format(hex(self.code[at - 1])))
		assert to >= at
		offset = to - at
		self.code[at: at + 4] = array.array('B', CodeParser.label_struct.pack(offset))

	def _skip_block_signature(self):
		self.index += 1
	
	def _skip_label(self):
		self.index += 4

	def _block_handler(self, _):
		self._skip_block_signature()
		self._insert_unbound_label()
		self._skip_label()
	
	def _loop_handler(self, _):
		label_pos = self.index - 1
		self._skip_block_signature()
		# insert a label and bind it to the position of the loop instruction
		self._insert_bound_label(label_pos)
		self._skip_label()

	def _if_handler(self, _):
		self._block_handler(_)

	def _else_handler(self, _):
		at = self.label_stack.pop()
		assert at is not None
		self._insert_unbound_label()
		self._skip_label()
		self._bind_label_at(at, self.index)

	def _end_handler(self, _):
		at = self.label_stack.pop()
		self.index += 1
		if at is not None:
			self._bind_label_at(at, self.index)
			
	def _br_table_handler(self, _):
		count = CodeParser.index_struct.unpack_from(self.code, self.index)
		self.index += (count + 2) * CodeParser.index_struct.size

	def _default_handler(self, opcode):
		self.index += 1
		self.index += CodeParser.immediates_sizes[opcode]
		

def finalized_code(code, module):
	code, bytecount = ImmediatesParser(
		code,
		module.function_signatures,
		module.functions, 
		module.tables, 
		module.memories, 
		module.globals
	).parse()
	return CodeParser(code).parse(), bytecount


ImmediatesParser.handlers = defaultdict(lambda: ImmediatesParser._default, {
	ops.BLOCK: 		ImmediatesParser._block, 
	ops.LOOP: 		ImmediatesParser._block, 
	ops.IF: 		ImmediatesParser._block, 
	ops.END: 		ImmediatesParser._pop_label, 
	ops.BR: 		ImmediatesParser._index, 
	ops.BR_IF: 		ImmediatesParser._index, 
	ops.BR_TABLE: 		ImmediatesParser._branch_table, 

	ops.CALL: 		ImmediatesParser._function_index, 
	ops.CALL_INDIRECT: 	ImmediatesParser._call_indirect, 
	ops.GET_LOCAL: 		ImmediatesParser._index, 
	ops.SET_LOCAL: 		ImmediatesParser._index, 
	ops.TEE_LOCAL: 		ImmediatesParser._index, 
	ops.GET_GLOBAL:		ImmediatesParser._global_index, 
	ops.SET_GLOBAL:		ImmediatesParser._global_index, 

	ops.I32_LOAD: 		ImmediatesParser._load_store, 
	ops.I64_LOAD: 		ImmediatesParser._load_store, 
	ops.F32_LOAD: 		ImmediatesParser._load_store, 
	ops.F64_LOAD: 		ImmediatesParser._load_store, 

	ops.I32_LOAD: 		ImmediatesParser._load_store, 
	ops.I32_LOAD: 		ImmediatesParser._load_store, 
	ops.I32_LOAD: 		ImmediatesParser._load_store, 
	ops.I32_LOAD: 		ImmediatesParser._load_store, 
	ops.I32_LOAD:		ImmediatesParser._load_store, 
	ops.I64_LOAD:		ImmediatesParser._load_store, 
	ops.F32_LOAD:		ImmediatesParser._load_store, 
	ops.F64_LOAD:		ImmediatesParser._load_store, 
	ops.I32_LOAD8_S:	ImmediatesParser._load_store, 
	ops.I32_LOAD8_U:	ImmediatesParser._load_store, 
	ops.I32_LOAD16_S:	ImmediatesParser._load_store, 
	ops.I32_LOAD16_U:	ImmediatesParser._load_store, 
	ops.I64_LOAD8_S:	ImmediatesParser._load_store, 
	ops.I64_LOAD8_U:	ImmediatesParser._load_store, 
	ops.I64_LOAD16_S:	ImmediatesParser._load_store, 
	ops.I64_LOAD16_U:	ImmediatesParser._load_store, 
	ops.I64_LOAD32_S:	ImmediatesParser._load_store, 
	ops.I64_LOAD32_U:	ImmediatesParser._load_store, 
	ops.I32_STORE:		ImmediatesParser._load_store, 
	ops.I64_STORE:		ImmediatesParser._load_store, 
	ops.F32_STORE:		ImmediatesParser._load_store, 
	ops.F64_STORE:		ImmediatesParser._load_store, 
	ops.I32_STORE8:		ImmediatesParser._load_store, 
	ops.I32_STORE16:	ImmediatesParser._load_store, 
	ops.I64_STORE8:		ImmediatesParser._load_store, 
	ops.I64_STORE16:	ImmediatesParser._load_store, 
	ops.I64_STORE32:	ImmediatesParser._load_store, 

	ops.I32_CONST: 		ImmediatesParser._leb128_sint32, 
	ops.I64_CONST:		ImmediatesParser._leb128_sint64, 
	ops.F32_CONST:		ImmediatesParser._float32, 
	ops.F64_CONST:		ImmediatesParser._float64, 

	ops.CURRENT_MEMORY:	ImmediatesParser._leb128_uint1,
	ops.GROW_MEMORY:	ImmediatesParser._leb128_uint1,
})



CodeParser.handlers = defaultdict((lambda: CodeParser._default_handler), {
	ops.BLOCK:  	CodeParser._block_handler,
	ops.LOOP:  	CodeParser._loop_handler,
	ops.IF:  	CodeParser._if_handler,
	ops.ELSE:  	CodeParser._else_handler,
	ops.END:  	CodeParser._end_handler,
	ops.BR_TABLE:  	CodeParser._br_table_handler,
})
