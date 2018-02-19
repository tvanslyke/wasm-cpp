import leb128
import constants as consts
import opcodes as ops
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
		has_maximum = bool(self.parse_unsigned_leb128(1))
		initial_size = self.parse_unsigned_leb128(32)
		maximum = None
		if has_maximum:
			maximum = self.parse_unsigned_leb128(32)
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
		return_types = self.parse_array(parse_fn)
		for tp in param_types:
			assert tp in consts.LanguageType.value_types
		for tp in return_types:
			if tp not in consts.LanguageType.value_types:
				raise ValueError("Invalid 'value_type' (typecode={}) encountered "
					"while parsing function signature.".format(hex(tp)))
		param_types = struct.pack('{}b'.format(len(param_types)), *param_types)
		return_types = struct.pack('{}b'.format(len(return_types)), *return_types)
		return param_types, return_types
		
	def parse_global_type(self):
		content_type = consts.LanguageType.parse_value_type(self.view)
		mutable = parse_unsigned_leb128(1)
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
	wasm_ubyte = struct.Struct('=b')
	wasm_sbyte = struct.Struct('=B')
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
		assert code[-1] == ops.End
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
		opcode = ops.Nop
		while self.parser.bytes_remaining > 0:#not ((opcode == ops.End) and (self.labels == -1)):
			opcode = self.parser.parse_byte()
			self.code_dest.append(opcode)
			ImmediatesParser.handlers[opcode](self)
			
		assert self.code_dest[-1] == ops.End
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
		self._leb128_uint32()

	def _function_signature_index(self):
		index = self.parser.parse_signed_leb128(32)
		value = self.function_signatures[index]
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
		
	def _function_index(self):
		index = self.parser.parse_signed_leb128(32)
		value = self.functions[index]
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
		
	def _table_index(self):
		index = self.parser.parse_signed_leb128(32)
		value = self.tables[index]
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
		
	def _memory_index(self):
		index = self.parser.parse_signed_leb128(32)
		value = self.memories[index]
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
		
	def _global_index(self):
		index = self.parser.parse_signed_leb128(32)
		value = self.globals[index]
		self.code_dest.extend(ImmediatesParser.wasm_uint32.pack(value))
		return value
		
	def _reserved(self):
		self._leb128_uint1()

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
		ops.Block: 		1, 
		ops.Loop: 		1, 
		ops.If: 		1, 
		ops.Br: 		4,
		ops.Br_if: 		4,
		ops.Br_table: 		-1,
		ops.Call: 		4, 
		ops.Call_indirect: 	4, 
		ops.Get_local: 		4, 
		ops.Set_local: 		4, 
		ops.Tee_local: 		4, 
		ops.Get_global:		4, 
		ops.Set_global:		4, 

		ops.I32_load: 		8, 
		ops.I64_load: 		8, 
		ops.F32_load: 		8, 
		ops.F64_load: 		8, 

		ops.I32_load: 		8, 
		ops.I32_load: 		8, 
		ops.I32_load: 		8, 
		ops.I32_load: 		8, 
		ops.I32_load:		8, 
		ops.I64_load:		8, 
		ops.F32_load:		8, 
		ops.F64_load:		8, 
		ops.I32_load8_s:	8, 
		ops.I32_load8_u:	8, 
		ops.I32_load16_s:	8, 
		ops.I32_load16_u:	8, 
		ops.I64_load8_s:	8, 
		ops.I64_load8_u:	8, 
		ops.I64_load16_s:	8, 
		ops.I64_load16_u:	8, 
		ops.I64_load32_s:	8, 
		ops.I64_load32_u:	8, 
		ops.I32_store:		8, 
		ops.I64_store:		8, 
		ops.F32_store:		8, 
		ops.F64_store:		8, 
		ops.I32_store8:		8, 
		ops.I32_store16:	8, 
		ops.I64_store8:		8, 
		ops.I64_store16:	8, 
		ops.I64_store32:	8, 

		ops.Current_memory:	1,
		ops.Grow_memory:	1,
	})

	label_struct = struct.Struct('L')

	def __init__(self, code):
		self.label_stack = []
		self.code = code
		assert self.code[-1] == ops.End
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
		self.code[self.index: self.index] = array.array('B', b'\0\0\0\0')
		self.label_stack.append(self.index)

	def _insert_bound_label(self, pos):
		self.code[self.index: self.index] = array.array('B', struct.pack('L', pos))
		self.label_stack.append(None)

	def _bind_label_at(self, at, to):
		assert self.code[at:at + 4] == b'\0\0\0\0'
		self.code[at, at + 4] = array.array('B', struct.pack('L', to))

	def _bind_top_label(self):
		at = self.label_stack.pop()
		self._bind_label_at(at, self.index)
	
	def _skip_block_signature(self):
		self.index += 1
	
	def _skip_label(self):
		self.index += 4

	def _block_handler(self, _):
		self._skip_block_signature()
		self._insert_unbound_label()
		self._skip_label()
	
	def _loop_handler(self, _):
		self._skip_block_signature()
		# insert a label and bind it to the exact next byte
		self._insert_bound_label(self.index + 4)
		self._skip_label()

	def _if_handler(self, _):
		_block_handler()

	def _else_handler(self, _):
		self._skip_block_signature()
		at = self.label_stack.pop()
		self._insert_unbound_label()
		self._skip_label()
		self._bind_label_at(at, self.index)

	def _end_handler(self, _):
		at = self.label_stack.pop()
		self.index += 1
		if at is not None:
			self._bind_label_at(at, self.index)
			
			
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
	ops.Block: 		ImmediatesParser._block, 
	ops.Loop: 		ImmediatesParser._block, 
	ops.If: 		ImmediatesParser._block, 
	ops.End: 		ImmediatesParser._pop_label, 
	ops.Br: 		ImmediatesParser._index, 
	ops.Br_if: 		ImmediatesParser._index, 
	ops.Br_table: 		ImmediatesParser._branch_table, 
	ops.Call: 		ImmediatesParser._function_index, 
	ops.Call_indirect: 	ImmediatesParser._call_indirect, 
	ops.Get_local: 		ImmediatesParser._index, 
	ops.Set_local: 		ImmediatesParser._index, 
	ops.Tee_local: 		ImmediatesParser._index, 
	ops.Get_global:		ImmediatesParser._global_index, 
	ops.Set_global:		ImmediatesParser._global_index, 

	ops.I32_load: 		ImmediatesParser._load_store, 
	ops.I64_load: 		ImmediatesParser._load_store, 
	ops.F32_load: 		ImmediatesParser._load_store, 
	ops.F64_load: 		ImmediatesParser._load_store, 

	ops.I32_load: 		ImmediatesParser._load_store, 
	ops.I32_load: 		ImmediatesParser._load_store, 
	ops.I32_load: 		ImmediatesParser._load_store, 
	ops.I32_load: 		ImmediatesParser._load_store, 
	ops.I32_load:		ImmediatesParser._load_store, 
	ops.I64_load:		ImmediatesParser._load_store, 
	ops.F32_load:		ImmediatesParser._load_store, 
	ops.F64_load:		ImmediatesParser._load_store, 
	ops.I32_load8_s:	ImmediatesParser._load_store, 
	ops.I32_load8_u:	ImmediatesParser._load_store, 
	ops.I32_load16_s:	ImmediatesParser._load_store, 
	ops.I32_load16_u:	ImmediatesParser._load_store, 
	ops.I64_load8_s:	ImmediatesParser._load_store, 
	ops.I64_load8_u:	ImmediatesParser._load_store, 
	ops.I64_load16_s:	ImmediatesParser._load_store, 
	ops.I64_load16_u:	ImmediatesParser._load_store, 
	ops.I64_load32_s:	ImmediatesParser._load_store, 
	ops.I64_load32_u:	ImmediatesParser._load_store, 
	ops.I32_store:		ImmediatesParser._load_store, 
	ops.I64_store:		ImmediatesParser._load_store, 
	ops.F32_store:		ImmediatesParser._load_store, 
	ops.F64_store:		ImmediatesParser._load_store, 
	ops.I32_store8:		ImmediatesParser._load_store, 
	ops.I32_store16:	ImmediatesParser._load_store, 
	ops.I64_store8:		ImmediatesParser._load_store, 
	ops.I64_store16:	ImmediatesParser._load_store, 
	ops.I64_store32:	ImmediatesParser._load_store, 

	ops.Current_memory:	ImmediatesParser._leb128_uint1,
	ops.Grow_memory:	ImmediatesParser._leb128_uint1,
})



CodeParser.handlers = defaultdict((lambda: CodeParser._default_handler), {
	ops.Block:  CodeParser._block_handler,
	ops.Loop:  CodeParser._loop_handler,
	ops.If:  CodeParser._if_handler,
	ops.Else:  CodeParser._else_handler,
	ops.End:  CodeParser._end_handler
})
