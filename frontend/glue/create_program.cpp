#include <Python.h>
#include <memory>
#include "wasm_base.h"
#include "function/wasm_function.h"
#include "module/wasm_table.h"
#include "module/wasm_linear_memory.h"
#include "module/wasm_program_state.h"

struct PyObjectDeleter {
	void operator()(void* obj) const
	{
		Py_XDECREF(((PyObject*)obj));
	}
};
struct PyObjectFastDeleter {
	void operator()(void* obj) const
	{
		Py_DECREF(((PyObject*)obj));
	}
};

struct PythonException: public std::runtime_error
{

};

static void throw_python_exception(PyObject* type, const char* message)
{
	PyErr_SetString(type, message);
	throw PythonError(message);
}

static void propagate_python_exception(PyObject* type, const char* message)
{
	throw PythonException("Exception thrown by CPython C API.");
}

using PythonObject = std::unique_ptr<PyObject, PyObjectDeleter>;
using PythonFastObject = std::unique_ptr<PyObject, PyObjectFastDeleter>;




static void read_index_space(PyObject* array_object, wasm_uint32_t* dest)
{
	PythonU32Buffer buffer(array_object);
	buffer.read_buffer_fast(dest);
}


void read_program_def(PyObject* prog_def)
{
	
}



template <class Deserializer>
static auto read_serializable(PyObject* serializable, Deserializer deserializer)
{
	PythonObject bytes_obj = PyObject_CallMethod(function, "serialize", nullptr);
	if(not bytes_obj)
		propagate_python_excepion();
	else if(not PyBytes_Check(bytes_obj))
		throw_python_exception(PyExc_TypeError, "Expected object of type 'bytes' but got something else.");
	Py_ssize_t len = PyBytes_Size(function);
	const char* data = PyBytes_AsString(function);
	if(not data)
		propagate_python_excepion();
	return deserializer(data, data + len);
}

template <class WasmObject, class Reader>
static std::vector<WasmObject> read_wasm_objects(Reader reader, PyObject* program_def, const char* attr_name)
{
	using namespace std::literals::string;
	std::vector<WasmObjects> wasm_objects;
	PythonObject py_objects = PyObject_GetAttrString(program_def, attr_name);
	if(not py_objects.get())
	{
		propagate_python_exception();
	}
	else if(not PyTuple_CheckExact(py_objects))
	{
		std::string message = "Attribute '"s + attr_name + "' must be a tuple instance."s;
		throw_python_exception(PyExc_TypeError, message.c_str());
	}
	auto len = PyTuple_GET_SIZE(py_objects);
	wasm_objects.reserve(len);
	for(Py_ssize_t i = 0; i < len; ++i)
	{
		WasmObject object(reader(PyTuple_GET_ITEM(py_objects, i)));
		wasm_objects.push_back(std::move(object));
	}
	return wasm_objects;
}


/***** FUNCTIONS *****/
static wasm_function deserialize_function(const char* begin, const char* end)
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	assert(end - begin >= std::ptrdiff_t(2 * sizeof(wasm_uint32_t) + 1);
	wasm_uint32_t sig_id, nlocals;
	std::memcpy(&sig_id, std::exchange(begin, begin + sizeof(sig_id)), sizeof(sig_id));
	std::memcpy(&nlocals, std::exchange(begin, begin + sizeof(nlocals)), sizeof(nlocals));
	auto code_begin = static_cast<const opcode_t*>(begin);
	auto code_end = static_cast<const opcode_t*>(end);
	return wasm_function(code_begin, code_end, sig_id, nlocals);
}

static wasm_function read_function(PyObject* function)
{ return read_serializable(function, deserialize_function); }

static std::vector<wasm_function> read_functions(PyObject* program_def)
{ return read_wasm_objects<wasm_function>(read_function, program_def, "functions"); }


/***** TABLES *****/

static wasm_table deserialize_table(const char* begin, const char* end)
{
	std::vector<std::size_t> offsets;
	long long max_size;
	signed char typecode;
	assert(end - begin >= std::ptrdiff_t(sizeof(long long) + sizeof(typecode));
	std::memcpy(&max_size, std::exchange(begin, begin + sizeof(max_size)), sizeof(max_size));
	std::memcpy(&typecode, std::exchange(begin, begin + sizeof(typecode)), sizeof(typecode));
	assert(((end - begin) % sizeof(long long)) == 0)
	offsets.resize((end - begin) / sizeof(long long));
	for(auto& offset: offsets)
	{
		long long value;
		std::memcpy(&value, std::exchange(begin, begin + sizeof(value)), sizeof(value));
		if(value < 0)
			offset = std::numeric_limits<decltype(offset)>::max();
		else
			offset = static_cast<decltype(offset)>(value);
	}
	if(max_size < 0)
		return wasm_table(std::move(offsets), typecode, std::optional<std::size_t>(std::nullopt));
	else
		return wasm_table(std::move(offsets), typecode, std::optional<std::size_t>(max_size));
}

static wasm_table read_table(PyObject* table)
{ return read_serializable(table, deserialize_table); }

static std::vector<wasm_table> read_tables(PyObject* program_def)
{ return read_wasm_objects<wasm_table>(read_table, program_def, "table"); }


/***** MEMORIES *****/

static wasm_linear_memory deserialize_memory(const char* begin, const char* end)
{
	long long max_size;
	assert(end - begin >= std::ptrdiff_t(sizeof(max_size));
	std::memcpy(&max_size, std::exchange(begin, begin + sizeof(max_size)), sizeof(max_size));
	std::vector<wasm_byte_t> memory(static_cast<const wasm_byte_t*>(begin), static_cast<const wasm_byte_t*>(end));
	if(max_size < 0)
		return wasm_linear_memory(std::move(memory), std::optional<std::size_t>(std::nullopt));
	else
		return wasm_linear_memory(std::move(memory), std::optional<std::size_t>(max_size));
}

static wasm_linear_memory read_memory(PyObject* memory)
{ return read_serializable(memory, deserialize_memory); }

static std::vector<wasm_linear_memory> read_memories(PyObject* program_def)
{ return read_wasm_objects<wasm_linear_memory>(read_memory, program_def, "memories"); }


/***** GLOBALS *****/

static std::pair<wasm_value, bool> deserialize_global(const char* begin, const char* end)
{
	using namespace std::literals::string;
	assert(end - begin >= std::ptrdiff_t(sizeof(wasm_sint64_t) + 1);
	wasm_value_t value{wasm_uint64_t(0)};
	bool is_mutable;
	char typecode;
	
	std::memcpy(&is_mutable, std::exchange(begin, begin + sizeof(is_mutable)), sizeof(is_mutable));
	std::memcpy(&typecode, std::exchange(begin, begin + sizeof(typecode)), sizeof(typecode));
	switch(typecode)
	{
	case 'l':
		std::memcpy(&(value.s32), std::exchange(begin, begin + sizeof(value.s32)), sizeof(value.s32));
		break;
	case 'L':
		std::memcpy(&(value.u32), std::exchange(begin, begin + sizeof(value.u32)), sizeof(value.u32));
		break;
	case 'q':
		std::memcpy(&(value.s64), std::exchange(begin, begin + sizeof(value.s64)), sizeof(value.s64));
		break;
	case 'Q':
		std::memcpy(&(value.u64), std::exchange(begin, begin + sizeof(value.u64)), sizeof(value.u64));
		break;
	case 'f':
		std::memcpy(&(value.u32), std::exchange(begin, begin + sizeof(value.u32)), sizeof(value.u32));
		break;
	case 'd':
		std::memcpy(&(value.s64), std::exchange(begin, begin + sizeof(value.s64)), sizeof(value.s64));
		break;
	default:
		throw std::runtime_error("Bad type format code '"s + typecode + "' encountered while deserializing global"s);
	}
	return {value, is_mutable};
}

static std::pair<wasm_value_t, bool> read_global(PyObject* global)
{ return read_serializable(global, deserialize_memory); }

static std::pair<std::vector<wasm_value_t>, std::vector<bool>> read_globals(PyObject* program_def)
{ 
	auto global_variables = read_wasm_objects<std::pair<wasm_value_t, bool>>(read_global, program_def, "global"); 
	std::vector<wasm_value_t> values;
	std::vector<bool> mutabilities;
	values.reserve(global_variables.size());
	mutabilities.reserve(global_variables.size());
	for(auto& pair: global_variables)
	{
		values.push_back(pair.first);
		mutabilities.push_back(pair.second);
	}
	return {values, mutabilities};
}

static std::size_t read_start_function(PyObject* program_def)
{
	PythonObject offset = PyObject_GetAttrString(program_def, "start_function");
	if(not offset)
		propagate_python_exception();
	else if(not PyLong_Check(offset))
		throw_python_exception(PyExc_TypeError, "'start_function' must be an int.");
	std::size_t index = PyLong_AsSize_t(offset);
	if((index == ((std::size_t)-1)) and PyErr_Occurred())
		propagate_python_exception();
	
	return index;
}

static wasm_program_state read_program_def(PyObject* program_def)
{
	auto funtions = read_functions(program_def);
	auto tables = read_tables(program_def);
	auto memories = read_memories(program_def);
	auto [globals, mutabilities] = read_globals(program_def);
	std::size_t start_fn = read_start_function(program_def);
	return WasmProgramState(
		std::move(functions),
		std::move(tables),
		std::move(memories),
		std::move(globals),
		std::move(mutabilities),
		start_fn
	);
}



wasm_program_state create_program(int argc, const char** argv)
{
	assert(argc > 0);
	struct PyRuntimeScopeGuard { 
	
		PyRuntimeScopeGuard() 
		{ Py_InitializeEx(0); }
		
		~PyRuntimeScopeGuard() 
		{ Py_Finalize(); }
	};
	PyRuntimeScopeGuard py_runtime;
	PythonObject program_args = PyTuple_New(argc - 1);
	if(not program_args)
		propagate_python_exception();
	PythonObject arg_string;
	for(Py_ssize_t i = 0; i < (argc - 1); ++i)
	{
		arg_string = PyUnicode_FromString(argv[i + 1]);
		if(not arg_string)
			propagate_python_exception();
		PyTuple_SET_ITEM(program_args, arg_string.release());
	}
	// TODO: initialize modules here
	PythonObject pymain_module = PyImport_ImportModule("wasm_cpp_pymain");
	if(not pymain_module)
		propagate_python_exception();
	PythonObject pymain_main_function = PyObject_GetAttrString(pymain_module, "main");
	if(not pymain_main_function)
		propagate_python_exception();
	PythonObject program_def = PyObject_CallFunction(pymain_main_function, "O", program_args);
	if(not program_def)
		propagate_python_exception();
	return read_program_def(program_def);
}

