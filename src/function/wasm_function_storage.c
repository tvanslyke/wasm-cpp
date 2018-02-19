#include "function/wasm_function_storage.h"
#include <stdlib.h>
#include <string.h>

typedef uint_least8_t opcode_t;

typedef struct wasm_function_storage
{
	const size_t code_size;
	const size_t signature;
	const size_t locals_count;
	const size_t param_count;
	const size_t return_count;
	const opcode_t code[];
} function_storage_t;

static const size_t code_offset = offsetof(function_storage_t, code);

const function_storage_t*
FunctionStorage_New(const opcode_t* code_begin, const opcode_t* code_end, size_t sig, 
		size_t nlocals, size_t nparams, size_t nreturns)
{
	const size_t code_size = code_end - code_begin;
	const size_t code_bytes = code_size * sizeof(opcode_t);
	const function_storage_t init = {
		.code_size = code_size,
		.signature = sig,
		.locals_count = nlocals,
		.param_count = nparams,
		.return_count = nreturns,
	};
	void* storage = malloc(sizeof(function_storage_t) + code_bytes);
	if(!storage)
		return NULL;
	void* code_start = ((char*)storage) + code_offset;
	memcpy(storage, &init, sizeof(init));
	memcpy(code_start, code_begin, code_bytes);
	return storage;
}


void FunctionStorage_Delete(const function_storage_t* storage)
{
	// explicitly cast the const away to make the compiler not whine
	free((void*)storage);
}

const opcode_t* FunctionStorage_Code(const function_storage_t* storage)
{
	return storage->code;
}

size_t FunctionStorage_CodeSize(const function_storage_t* storage)
{
	return storage->code_size;
}

size_t FunctionStorage_Signature(const function_storage_t* storage)
{
	return storage->signature;
}


size_t FunctionStorage_LocalsCount(const function_storage_t* storage)
{
	return storage->locals_count;
}

size_t FunctionStorage_ParamCount(const function_storage_t* storage)
{
	return storage->param_count;
}

size_t FunctionStorage_ReturnCount(const function_storage_t* storage)
{
	return storage->return_count;
}
