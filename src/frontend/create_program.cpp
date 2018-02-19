#include <Python.h>
#include <memory>
#include <string>
#include "wasm_base.h"
#include "function/wasm_function.h"
#include "module/wasm_table.h"
#include "module/wasm_linear_memory.h"
#include "module/wasm_program_state.h"
#include "frontend/leb128/leb128.h"

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
	using std::runtime_error::runtime_error;
};

static void throw_python_exception(PyObject* type, const char* message)
{
	PyErr_SetString(type, message);
	throw PythonException(message);
}

#define PROPAGATE_PYTHON_EXCEPTION() \
	throw PythonException(std::string("Exception thrown by CPython C API. (create_program.cpp in ") + __PRETTY_FUNCTION__ + std::string(":") + std::to_string(__LINE__) + ")")

struct PythonObject:
	public std::unique_ptr<PyObject, PyObjectDeleter>
{
	using std::unique_ptr<PyObject, PyObjectDeleter>::unique_ptr;
	operator PyObject*()
	{
		return this->get();
	}

	operator const PyObject*() const
	{
		return this->get();
	}
	
	template <class MaybePythonObject>
	operator const MaybePythonObject* () const
	{
		error_if_not_pyobject<MaybePythonObject>();
		return (MaybePythonObject*)get();
	}
	
	template <class MaybePythonObject>
	operator MaybePythonObject* ()
	{
		error_if_not_pyobject<MaybePythonObject>();
		return (MaybePythonObject*)get();
	}

private:
	template <class MaybePythonObject>
	static void error_if_not_pyobject()
	{
		using python_base_type = std::decay_t<decltype(std::declval<MaybePythonObject>().ob_base)>; // error if not a PyObject
		static_assert(std::is_same_v<python_base_type, PyObject> 
			or std::is_same_v<python_base_type, PyVarObject>,
			"Attempt to cast instance of PythonObject to pointer to class which does not derive from PyObject.");
	}
};

using PythonFastObject = std::unique_ptr<PyObject, PyObjectFastDeleter>;




template <class Deserializer>
static auto read_serializable(PyObject* serializable, Deserializer deserializer)
{
	PythonObject bytes_obj(PyObject_CallMethod(serializable, "serialize", nullptr));
	if(not bytes_obj)
		PROPAGATE_PYTHON_EXCEPTION();
	else if(not PyBytes_Check(bytes_obj))
		throw_python_exception(PyExc_TypeError, "Expected object of type 'bytes' but got something else.");
	Py_ssize_t len = PyBytes_Size(bytes_obj);
	const char* data = PyBytes_AsString(bytes_obj);
	if(not data)
		PROPAGATE_PYTHON_EXCEPTION();
	return deserializer(data, data + len);
}

template <class WasmObject, class Reader>
static std::vector<WasmObject> read_wasm_objects(Reader reader, PyObject* program_def, const char* attr_name)
{
	using namespace std::literals::string_literals;
	std::vector<WasmObject> wasm_objects;
	PythonObject py_objects(PyObject_GetAttrString(program_def, attr_name));
	if(not py_objects.get())
	{
		PROPAGATE_PYTHON_EXCEPTION();
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
	assert(end - begin >= std::ptrdiff_t(2 * sizeof(wasm_uint32_t) + 1));
	wasm_uint32_t sig_id, nlocals, param_count, return_count;
	std::memcpy(&sig_id, std::exchange(begin, begin + sizeof(sig_id)), sizeof(sig_id));
	std::memcpy(&nlocals, std::exchange(begin, begin + sizeof(nlocals)), sizeof(nlocals));
	std::memcpy(&param_count, std::exchange(begin, begin + sizeof(param_count)), sizeof(param_count));
	std::memcpy(&return_count, std::exchange(begin, begin + sizeof(return_count)), sizeof(return_count));
	auto code_begin = reinterpret_cast<const opcode_t*>(begin);
	auto code_end = reinterpret_cast<const opcode_t*>(end);
	return wasm_function(code_begin, code_end, sig_id, nlocals, param_count, return_count);
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
	assert(end - begin >= std::ptrdiff_t(sizeof(long long) + sizeof(typecode)));
	std::memcpy(&max_size, std::exchange(begin, begin + sizeof(max_size)), sizeof(max_size));
	std::memcpy(&typecode, std::exchange(begin, begin + sizeof(typecode)), sizeof(typecode));
	assert(((end - begin) % sizeof(long long)) == 0);
	offsets.resize((end - begin) / sizeof(long long));
	for(auto& offset: offsets)
	{
		long long value;
		std::memcpy(&value, std::exchange(begin, begin + sizeof(value)), sizeof(value));
		if(value < 0)
			offset = std::numeric_limits<std::decay_t<decltype(offset)>>::max();
		else
			offset = static_cast<std::decay_t<decltype(offset)>>(value);
	}
	if(max_size < 0)
		return wasm_table(std::move(offsets), typecode, std::optional<std::size_t>(std::nullopt));
	else
		return wasm_table(std::move(offsets), typecode, std::optional<std::size_t>(max_size));
}

static wasm_table read_table(PyObject* table)
{ return read_serializable(table, deserialize_table); }

static std::vector<wasm_table> read_tables(PyObject* program_def)
{ return read_wasm_objects<wasm_table>(read_table, program_def, "tables"); }


/***** MEMORIES *****/

static wasm_linear_memory deserialize_memory(const char* begin, const char* end)
{
	long long max_size;
	assert(end - begin >= std::ptrdiff_t(sizeof(max_size)));
	std::memcpy(&max_size, std::exchange(begin, begin + sizeof(max_size)), sizeof(max_size));
	std::vector<wasm_byte_t> memory(reinterpret_cast<const wasm_byte_t*>(begin), reinterpret_cast<const wasm_byte_t*>(end));
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

static std::pair<wasm_value_t, bool> deserialize_global(const char* begin, const char* end)
{
	using namespace std::literals::string_literals;
	assert(end - begin >= std::ptrdiff_t(sizeof(wasm_sint64_t) + 1));
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
{ return read_serializable(global, deserialize_global); }

static std::pair<std::vector<wasm_value_t>, std::vector<bool>> read_globals(PyObject* program_def)
{ 
	auto global_variables = read_wasm_objects<std::pair<wasm_value_t, bool>>(read_global, program_def, "globals"); 
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
	PythonObject offset(PyObject_GetAttrString(program_def, "start_function"));
	if(not offset)
		PROPAGATE_PYTHON_EXCEPTION();
	else if(not PyLong_Check(offset))
		throw_python_exception(PyExc_TypeError, "'start_function' must be an int.");
	std::size_t index = PyLong_AsSize_t(offset);
	if((index == ((std::size_t)-1)) and PyErr_Occurred())
		PROPAGATE_PYTHON_EXCEPTION();
	
	return index;
}

static wasm_program_state read_program_def(PyObject* program_def)
{
	auto functions = read_functions(program_def);
	auto tables = read_tables(program_def);
	auto memories = read_memories(program_def);
	auto [globals, mutabilities] = read_globals(program_def);
	std::size_t start_fn = read_start_function(program_def);
	return wasm_program_state(
		std::move(functions),
		std::move(tables),
		std::move(memories),
		std::move(globals),
		std::move(mutabilities),
		start_fn
	);
}

constexpr const char* python_script_install_dir = "/home/tim/Projects/wasm/frontend/";

wasm_program_state create_program(int argc, const char** argv)
{
	assert(argc > 0);
	struct PyRuntimeScopeGuard { 
	
		PyRuntimeScopeGuard() 
		{ Py_InitializeEx(0); }
		
		~PyRuntimeScopeGuard() 
		{ Py_Finalize(); }
	};
	if(PyImport_AppendInittab("leb128", PyInit_leb128) != 0)
		throw std::runtime_error("Failed to add CPython extension module leb128 to the CPython runtime.");
	PyRuntimeScopeGuard py_runtime;
	try
	{
		{// add directory containing internal scripts to python path
			PyObject* sys_path = PySys_GetObject("path");
			assert(sys_path);
			PythonObject new_path(PyUnicode_FromString(python_script_install_dir));
			if(not new_path)
				PROPAGATE_PYTHON_EXCEPTION();
			else if(0 != PyList_Append(sys_path, new_path))
				PROPAGATE_PYTHON_EXCEPTION();
		}
		PythonObject program_args(PyTuple_New(argc - 1));
		if(not program_args)
			PROPAGATE_PYTHON_EXCEPTION();
		for(Py_ssize_t i = 0; i < (argc - 1); ++i)
		{
			PythonObject arg_string(PyUnicode_FromString(argv[i + 1]));
			if(not arg_string)
				PROPAGATE_PYTHON_EXCEPTION();
			PyTuple_SET_ITEM(program_args, i, arg_string.release());
		}
		// TODO: initialize modules here
		PythonObject pymain_module(PyImport_ImportModule("wasm_cpp_pymain"));
		if(not pymain_module)
			PROPAGATE_PYTHON_EXCEPTION();
		PythonObject pymain_main_function(PyObject_GetAttrString(pymain_module, "main"));
		if(not pymain_main_function)
			PROPAGATE_PYTHON_EXCEPTION();
		PythonObject program_def(PyObject_CallFunctionObjArgs(pymain_main_function.get(), program_args.get(), nullptr));
		if(not program_def)
			PROPAGATE_PYTHON_EXCEPTION();
		return read_program_def(program_def);
	}
	catch(const std::exception&)
	{
		if(PyErr_Occurred())
			PyErr_Print();
		throw;
	}
}

