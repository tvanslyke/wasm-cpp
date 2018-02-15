#include <Python.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char ubyte_t;

static const ubyte_t mask7 = (ubyte_t)(~(((ubyte_t)1u) << 7u));

static int sys_is_little_endian(void)
{
	volatile union {
		uint_least64_t integer;
		ubyte_t bytes[sizeof(uint_least64_t)];
	} endian_check;
	endian_check.integer = 0x01020304;
	return endian_check.bytes[0] == 0x01;
}


static void leb128_parse_error_smallbuffer(void)
{
	PyErr_SetString(PyExc_ValueError, 
		"Provided data buffer exhausted before decoding of leb128 integer finished.");
}

static void leb128_parse_error_badwidth(void)
{
	PyErr_SetString(PyExc_ValueError, 
		"Requested width of leb128 integer to decode exhausted before end of encoding.");
}

static void leb128_parse_error_bigwidth(void)
{
	PyErr_SetString(PyExc_ValueError, 
		"Can only parse leb128 integers up to a width of 64 bits.");
}

const ubyte_t* 
leb128_parse_unsigned(const ubyte_t* begin, const ubyte_t* end, uint_least64_t* dest, size_t width)
{
	if(!(width <= 64))
	{
		leb128_parse_error_bigwidth();
		return NULL;
	}
	assert(begin < end);
	
	const ubyte_t* pos = begin;
	uint_least64_t value = 0;
	uint_least64_t shift = 0;
	uint_least8_t byte = 0;
	for(;;)
	{
		if(pos >= end)
		{
			leb128_parse_error_smallbuffer();
			return NULL;
		}
		else if(shift >= width)
		{
			leb128_parse_error_badwidth();
			return NULL;
		}
		byte = *pos;
		++pos;
		value |= ((uint_least64_t) (byte & mask7)) << shift;
		if(!(byte & (~mask7)))
			break;
		shift += 7;
	}
	if(sys_is_little_endian())
	{
		char buff[sizeof(value)];
		memcpy(buff, &value, sizeof(value));
		char* b = buff;
		char* e = buff + (sizeof(buff) - 1);
		while(b < e)
		{
			char tmp = *b;
			*b = *e;
			*e = tmp;
			++b;
			--e;
		}
		memcpy(&value, buff, sizeof(value));
	}
	*dest = value;
	return pos;
}


const ubyte_t* 
leb128_parse_signed(const ubyte_t* begin, const ubyte_t* end, int_least64_t* dest, size_t width)
{
	if(!(width <= 64))
	{
		leb128_parse_error_bigwidth();
		return NULL;
	}
	assert(begin < end);
	
	const ubyte_t* pos = begin;
	uint_least64_t value = 0;
	uint_least64_t shift = 0;
	uint_least8_t byte = 0;
	for(;;)
	{
		if(!(pos < end))
		{
			leb128_parse_error_smallbuffer();
			return NULL;
		}
		else if(shift > width)
		{
			leb128_parse_error_badwidth();
			return NULL;
		}
		byte = *pos++;
		value |= ((uint_least64_t) (byte & mask7)) << shift;
		shift += 7;
		if((byte & (~mask7)) == 0)
			break;
	}
	
	if((shift < (8 * sizeof(*dest))) && ((byte & 0x40) != 0))
		value |= (~0ull) << shift;
	
	if(sys_is_little_endian())
	{
		char buff[sizeof(value)];
		memcpy(buff, &value, sizeof(value));
		char* b = buff;
		char* e = buff + (sizeof(buff) - 1);
		while(b < e)
		{
			char tmp = *b;
			*b = *e;
			*e = tmp;
			++b;
			--e;
		}
		memcpy(&value, buff, sizeof(value));
	}
	memcpy(dest, &value, sizeof(value));
	return pos;
}



static int leb128_buffer_problems(Py_buffer* buff)
{
	int errcode = 0;
	if(buff->len < 1)
	{
		PyErr_SetString(PyExc_ValueError, 
			"Cannot parse an leb128 integer from a buffer of size < 1.");
		errcode = -2;
	}
	else if(!PyBuffer_IsContiguous(buff, 'C'))
	{
		PyErr_SetString(PyExc_TypeError, 
			"Cannot parse an leb128 integer from a non-C-contiguous buffer.");
		errcode = -3;
	}
	else if(buff->ndim != 1)
	{
		fflush(stdout);
		PyErr_SetString(PyExc_TypeError, 
			"Cannot parse an leb128 integer from a buffer with ndim != 1.");
		errcode = -4;
	}
	return errcode;
}

static int leb128_parse_args(PyObject* args, const char** buffer, unsigned long long* width)
{
	int len = 0;
	if(!PyArg_ParseTuple(args, "y#K", buffer, &len, width))
		return -1;
	else if(!(len > 0))
	{
		PyErr_SetString(PyExc_ValueError, "Buffer of length 0 cannot be decoded.");
		return -2;
	}
	return len;
}


PyObject* parse_unsigned_leb128(PyObject* self, PyObject* args)
{
	const char* buffer = NULL;
	unsigned long long width = 0;
	int len = leb128_parse_args(args, &buffer, &width);
	if(len < 0)
		return NULL;
	const ubyte_t* buff_begin = (const ubyte_t*)buffer;
	const ubyte_t* buff_end = (const ubyte_t*)(buffer + len);

	uint_least64_t value = 0;
	const ubyte_t* pos = leb128_parse_unsigned(buff_begin, buff_end, &value, width);

	// work is done; do cleanup and return but check for failures in construction of result objects
	if(!pos)
		return NULL;
	PyObject* result = PyTuple_New(2);
	if(!result)
		return NULL;
	PyObject* int_value = PyLong_FromUnsignedLongLong(value);
	if(!int_value)
		goto first_conversion_failed;
	PyObject* int_count = PyLong_FromLongLong(pos - buff_begin);
	if(!int_count)
		goto second_conversion_failed;
	PyTuple_SET_ITEM(result, 0, int_value);
	PyTuple_SET_ITEM(result, 1, int_count);
	return result;
    second_conversion_failed:
	Py_DECREF(int_value);
    first_conversion_failed:
	Py_DECREF(result);
	return NULL;
}



PyObject* parse_signed_leb128(PyObject* self, PyObject* args)
{
	const char* buffer;
	unsigned long long width = 0;
	int len = leb128_parse_args(args, &buffer, &width);
	if(len < 0)
		return NULL;
	
	const ubyte_t* buff_begin = (const ubyte_t*)buffer;
	const ubyte_t* buff_end = (const ubyte_t*)(buffer + len);
	int_least64_t value = 0;
	const ubyte_t* pos = leb128_parse_signed(buff_begin, buff_end, &value, width);

	// work is done; do cleanup and return but check for failures in construction of result objects
	if(!pos)
		return NULL;
	PyObject* result = PyTuple_New(2);
	if(!result)
		return NULL;
	PyObject* int_value = PyLong_FromLongLong(value);
	if(!int_value)
		goto first_conversion_failed;
	PyObject* int_count = PyLong_FromLongLong(pos - buff_begin);
	if(!int_count)
		goto second_conversion_failed;
	PyTuple_SET_ITEM(result, 0, int_value);
	PyTuple_SET_ITEM(result, 1, int_count);
	return result;
    second_conversion_failed:
	Py_DECREF(int_value);
    first_conversion_failed:
	Py_DECREF(result);
	return NULL;
}

static PyMethodDef Leb128Methods[] = {
	{
		"read_signed",  
		parse_signed_leb128, 
		METH_VARARGS,
		"Parse a LEB128-encoded signed integer from a bytes-like object and width (in bits) parameter.\n"
		"Returns the decoded integer and the number of bytes in the encoding, both as a python 'int' objects.\n" 
	},
	{
		"read_unsigned",  
		parse_unsigned_leb128, 
		METH_VARARGS,
		"Parse a LEB128-encoded signed uninteger from a bytes-like object and width (in bits) parameter.\n"
		"Returns the decoded integer and the number of bytes in the encoding, both as a python 'int' objects.\n" 
	},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef leb128module = {
    PyModuleDef_HEAD_INIT,
    "leb128",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    Leb128Methods
};

PyMODINIT_FUNC
PyInit_leb128(void)
{
	return PyModule_Create(&leb128module);
}


