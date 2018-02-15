#include <Python.h>
#include <utility>
#include "wasm_instruction.h"
#include "wasm_base.h"
#include "leb128.h"
#include <string>

using ubyte_t = unsigned char;
using view_t = std::basic_string_view<std::uint_least32_t>;

template <class Integer>
static std::pair<Integer, std::size_t> parse_leb128(const ubyte_t* begin, const ubyte_t* end)


static const ubyte_t* skip_leb128(const ubyte_t* begin, const ubyte_t* end)
{
	static constexpr const ubyte_t msb_mask = 0b1000'0000;
	
	while(begin < end)
	{
		if(not (*begin++ & msb_mask))
			break;
	}
	if(begin == end)
		return nullptr;
	else
		return begin;
}

static int read_buffer(Py_buffer* buff, view_t& view)
{
	using namespace std::literals::string_literals;
	static std::string errmessage;
	void* buffer_data = buff->buf;
	Py_ssize_t itemsize = buff->itemsize;
	Py_ssize_t ndim = buff->ndim;
	Py_ssize_t len = buff->len;
	const char* format = buff->format;
	const std::uint_least32_t* begin = nullptr;
	const std::uint_least32_t* end = nullptr;
	if(itemsize != 4)
	{
		errmessage = "Bad buffer itemsize " 
			+ std::to_string(buff->itemsize) 
			+ " encountered. (must be 4)";
		PyErr_SetString(PyExc_TypeError, errmessage.c_str());
	}
	else if(ndim < 0 or ndim > 1)
	{
		errmessage = "Bad buffer ndim " 
			+ std::to_string(buff->itemsize) 
			+ " encountered. (must be either 0 or 1)";
		PyErr_SetString(PyExc_TypeError, errmessage.c_str());
	}
	else if((not format) and ((reinterpret_cast<unsigned long>(buffer_data) & alignof(std::uint_least32_t)) != 0))
	{
		PyErr_SetString(PyExc_TypeError, "Buffer format string not provided and the buffer is not aligned to 4 bytes.");
	}
	else if(format and ((format[0] != 'L') or (format[0] != 'I') or (format[1] != '\0')))
	{
		errmessage = "Bad buffer format \""s
			+ format
			+ "\" encountered. (must be either \"L\" or \"I\")"s;
		PyErr_SetString(PyExc_TypeError, errmessage.c_str());
	}
	else if(not PyBuffer_IsContiguous(buff, 'C'))
	{
		PyErr_SetString(PyExc_TypeError, "Non-C-contiguous buffer recieved. (only contiguous buffers are permitted)");
	}
	else
	{
		begin = (const std::uint_least32_t*)(buff->buf);
		end = begin + (len / 4);
	}

	if(begin)
	{
		view = view_t(begin, end - begin);
		return 0;
	}
	else
	{
		return -1;
	}
}



static int finalize_code_impl(PyObject* args)
{
	PyObject* bytearray;
	Py_buffer buffers[4];
	if(not PyArg_ParseTuple(args, "Yy*y*y*y*", 
		&bytearray, 
		buffers, 
		buffers + 1,
		buffers + 2,
		buffers + 3))
	{
		return -1;
	}
	struct ScopeGuard{ 
		
		PyObject* bytearray;	
		Py_buffer* buffers[4];
		~ScopeGuard()
		{
			PyBuffer_Release(buffers[3]);
			PyBuffer_Release(buffers[2]);
			PyBuffer_Release(buffers[1]);
			PyBuffer_Release(buffers[0]);
			Py_XDECREF(bytearray);
		}
	} scope_guard{
		bytearray, 
		buffers + 0,
		buffers + 1,
		buffers + 2,
		buffers + 3
	};
	view_t functions, tables, memories, globals;
	if(read_buffer(buffers + 0, functions)
		or read_buffer(buffers + 1, tables)
		or read_buffer(buffers + 2, memories)
		or read_buffer(buffers + 3, globals))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

static PyObject* finalize_code(PyObject* self, PyObject* args)
{
	if(finalize_code_impl(args) != 0)
		return NULL;
	Py_RETURN_NONE;
}

struct CodeParser
{
	CodeParser()
	{
		// TODO: implement
	}



private:
	template <class Integer>
	std::pair<Integer, std::size_t> 
	read_leb128_immediate(std::size_t width = sizeof(Integer) * CHAR_BIT) const
	{
		constexpr bool is_signed = std::is_signed_v<Integer>;
		using value_type = std::conditional<is_signed, std::int_least64_t, std::uint_least64_t>;
		value_type value;
		const opcode_t* endpos;
		if constexpr(is_signed)
			endpos = leb128_parse_signed(code.begin() + idx, code.end(), &value, width);
		else
			endpos = leb128_parse_unsigned(code.begin() + idx, code.end(), &value, width);
		if(not endpos)
			throw std::runtime_error("Error occured while parsing leb128 integer.  See python exception for details.");
		return {value, endpos - (begin + idx)};
	}
	
	template <class Integer>
	Integer read_leb128_immediate_and_advance(std::size_t width = sizeof(Integer) * CHAR_BIT) 
	{
		auto [value, count] = read_leb128_immediate<Integer>(width);
		assert(count_total() - idx >= count);
		idx += count;
		return value;
	}

	template <class Integer>
	Integer replace_leb128_immediate(std::size_t width = sizeof(Integer) * CHAR_BIT)
	{
		
	}

	std::size_t count_total() const
	{
		return end - begin;
	}

	std::size_t count_remaining() const
	{
		return count_total() - idx;
	}

	std::basic_string<opcode_t> code;
	view_t functions;
	view_t tables;
	view_t memories;
	view_t globals;
	std::size_t idx;
	std::stack<std::size_t, std::vector<std::size_t>> unbound_labels;
};



template <class DestIt>
static void replace_immediates(const ubyte_t* begin, const ubyte_t* end, DestIt dest)
{
	std::uint_least64_t uval;
	std::int_least64_t  sval;
	while(pos < code.size())
	{
		using namespace wasm_opcode;
		auto opcode_value = code[pos++];
		switch(opcode_value)
		{
		case BLOCK:
			leb128_
			replace_block_signature();
			push_label();
			break;
		case LOOP:
			replace_block_signature();
			break;
		case BR: 	[[fallthrough]]
		case BR_IF:
			replace_leb128_uint32();
			break;
		case BR_TABLE:
			replace_branch_table();
			break;
		case IF:
			replace_block_signature();
			push_label();
			break;
		case ELSE:
			bind_else_block();
			break;
		case END:
			bind_top_label();
			break;
		case I32_CONST:
			replace_leb128_immediate(leb128_decode_sint32);
			break;
		case I64_CONST:
			replace_leb128_immediate(leb128_decode_sint64);
			break;
		case F32_CONST:
			skip_immediate(float());
			break;
		case F64_CONST:
			skip_immediate(double());
			break;
		case GET_LOCAL: 	[[fallthrough]]
		case SET_LOCAL: 	[[fallthrough]]
		case TEE_LOCAL: 
			replace_leb128_immediate(leb128_decode_uint32);
			break;
		case GET_GLOBAL:	[[fallthrough]]
		case SET_GLOBAL:
			replace_local_index(mappings.globals);
			break;
		case CALL:
			replace_local_index(mappings.functions);
			break;
		case CALL_INDIRECT:
			replace_local_index(mappings.tables);
			skip_immediate(pos);
			break;
		case I32_LOAD:		[[fallthrough]] 
		case I64_LOAD:		[[fallthrough]]
		case F32_LOAD:		[[fallthrough]]
		case F64_LOAD:		[[fallthrough]]
		case I32_LOAD8_S:	[[fallthrough]]
		case I32_LOAD8_U:	[[fallthrough]]
		case I32_LOAD16_S:	[[fallthrough]]
		case I32_LOAD16_U:	[[fallthrough]]
		case I64_LOAD8_S:	[[fallthrough]]
		case I64_LOAD8_U:	[[fallthrough]]
		case I64_LOAD16_S: 	[[fallthrough]]
		case I64_LOAD16_U:	[[fallthrough]]
		case I64_LOAD32_S:	[[fallthrough]]
		case I64_LOAD32_U:	[[fallthrough]]
		case I32_STORE: 	[[fallthrough]]
		case I64_STORE:		[[fallthrough]]
		case F32_STORE:		[[fallthrough]]
		case F64_STORE:		[[fallthrough]]
		case I32_STORE8:	[[fallthrough]]	
		case I32_STORE16:	[[fallthrough]]
		case I64_STORE8:	[[fallthrough]]
		case I64_STORE16:	[[fallthrough]]
		case I64_STORE32:	
			replace_leb128_immediate(leb128_decode_uint32);
			replace_leb128_immediate(LEB128_Decoder<wasm_ptr_t>{});
			break;
		case GROW_MEMORY:
		case CURRENT_MEMORY:
			skip_immediate(wasm_uint8_t());
			break;
		default:
			// opcodes without immediate operands
			if (wasm_instruction_dne(opcode_value))
			{
				// bad opcode :(
				std::ostringstream ss;
				ss << "Invalid opcode, " << std::hex << opcode_value << ", encountered.";
				throw std::runtime_error(ss.str());
			}
		} /* end switch */
	} /* end loop */
}














static PyMethodDef CParseMethods[] = {
	{
		"finalize_code",  
		finalize_code, 
		METH_VARARGS,
		"Finalize function code according to program-wide function, table, memory, and global indices."
	},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef cparsemodule = {
    PyModuleDef_HEAD_INIT,
    "cparse",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    CParseMethods
};

PyMODINIT_FUNC
PyInit_cparse(void)
{
	return PyModule_Create(&cparsemodule);
}






