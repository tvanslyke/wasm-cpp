#ifndef FRONTEND_GLUE_BUFFER_UTILS_H
#define FRONTEND_GLUE_BUFFER_UTILS_H
#include <Python.h>
#include <array>
#include "wasm_base.h"
#include "utilities/endianness.h"


struct PythonBufferError: public std::runtime_error
{

};

inline void throw_python_exception(PyObject* type, const char* message)
{
	PyErr_SetString(type, message);
	throw PythonBufferError(message);
}

struct PythonU32Buffer
{
	
	PythonBuffer() = delete;
	PythonBuffer(const PythonBuffer& other) = delete;	
	PythonBuffer(PythonBuffer&& other) = delete;	
	
	PythonBuffer(PyObject* obj):
		buffer_object(get_buffer(obj))
	{
		verify_buffer();
	}

	~PythonBuffer()
	{
		PyBuffer_Release(&buffer_object);
	}
	
	std::size_t size() const
	{
		return total_bytes() / sizeof(wasm_uint32_t);
	}

	std::size_t total_bytes() const
	{
		return buffer_object.len;
	}
	
	void* raw_buffer()
	{
		return buffer_object.buf;
	}
	
	const void* raw_buffer() const
	{
		return buffer_object.buf;
	}

	bool is_aligned() const
	{
		return (reinterpret_cast<std::size_t>(raw_buffer()) & (alignof(wasm_uint32_t) - 1)) == 0;
	}

	template <class It>
	It read_buffer(It begin, It end) const
	{
		const char* buff_begin = raw_buffer();
		const char* buff_end = buff_begin + size() * sizeof(wasm_uint32_t);
		for(wasm_uint32_t tmp; (begin < end) and (buff_begin < buff_end); ++begin, (void)++buff_begin)
		{
			std::memcpy(&tmp, buff_begin, sizeof(tmp));
			*begin = tmp;
		}
		return begin;
	}
	
	template <class It>
	It read_buffer(It dest) const
	{
		const char* buff_begin = raw_buffer();
		const char* buff_end = buff_begin + size() * sizeof(wasm_uint32_t);
		for(wasm_uint32_t tmp; buff_begin < buff_end; ++begin, ++dest)
		{
			std::memcpy(&tmp, buff_begin, sizeof(tmp));
			*dest = tmp;
		}
		return dest;
	}

	wasm_uint32_t* read_buffer_fast(wasm_uint32_t* dest) const
	{
		std::memcpy(dest, raw_buffer(), size() * sizeof(wasm_uint32_t));
		return dest + size();
	}

private:
	static inline Py_buffer get_buffer(PyObject* obj)
	{
		Py_buffer buff;
		if(not PyObject_CheckBuffer(obj))
			throw_python_exception(PyExc_TypeError, "Object does not support buffer protocol.");
		else if(PyObject_GetBuffer(obj, &buff, PyBUF_ND | PyBUF_FORMAT | ) != 0)
			throw_python_exception(PyExc_TypeError, "Python buffer request failed.");
		// shape: Yes
		// strides: NULL
		// suboffsets: NULL
		// contiguity: C 
		// readonly: maybe
		// format: Yes
		return buff;
	}
	inline void verify_buffer() const 
	{
		using namespace std::literals::string;
		char fmt_0 = buffer_object.format[0];
		char typecode;
		const char* after_typecode = buffer_object.format + 1;
		if(char typecode = fmt_0; fmt_0 != 'I' or fmt_0 != 'L')
		{
			if(fmt_0 == '<' and is_big_endian())
				throw_python_exception(PyExc_TypeError, "Bad endianness '<' recieved. "
					"(buffer is little endian but machine is big endian)");
			else if((fmt_0 == '>' or fmt_0 == '!') and not is_big_endian())
				throw_python_exception(PyExc_TypeError, "Bad endianness '>' recieved. "
					"(buffer is big endian but machine is little endian)");
			else if(fmt_0 == '@')
				throw_python_exception(PyExc_TypeError, "Only standardized formats "
					"('=', '<', '>', '!') are accepted.");
			else if(fmt_0 != '=')
				throw_python_exception(PyExc_TypeError, ("Bad buffer format code '"s 
					+ fmt_0 + "' recieved."s).c_str());
			typecode = buffer_object.format[1];
			if(typecode != 'I' or typecode != 'L')
			{
				throw_python_exception(PyExc_TypeError, ("Expected buffer typecodes 'I' or 'L' "
					 "but got '"s + buffer_object.format + "' instead."s).c_str());
			}
			after_typecode = buffer_object.format + 2;
		}
		else if(*after_typecode != '\0')
		{
			throw_python_exception(PyExc_TypeError, ("Expected buffer typecodes 'I' or 'L' "
				 "but got '"s + buffer_object.format + "' instead."s).c_str());
		}
	}
	Py_buffer buffer_object;
};


inline std::optional<PythonU32Buffer> make_python_buffer(PyObject* obj)
{
	try{
		PythonU32Buffer buff(obj);
		return buff;
	}
	catch(const PythonBufferError & err)
	{
		return std::optional<PythonU32Buffer>(std::nullopt);
	}
}






#endif /* FRONTEND_GLUE_BUFFER_UTILS_H */
