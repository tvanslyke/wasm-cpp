#ifndef WASM_FUNCTION_STORAGE_H
#define WASM_FUNCTION_STORAGE_H

#ifdef __cplusplus
# include <cstddef>
# include <type_traits>
# include "wasm_instruction.h"
using opcode_t = std::underlying_type_t<wasm_instruction>;
using std::size_t;
extern "C" {
#else
# include <stddef.h>
# include <stdint.h>
typedef uint_least8_t opcode_t;
#endif

struct wasm_function_storage;

const struct wasm_function_storage*
FunctionStorage_New(const opcode_t* code_begin, const opcode_t* code_end, 
		size_t nparams, size_t nlocals, size_t nreturns, size_t sig);

void FunctionStorage_Delete(const struct wasm_function_storage* storage);


// accessors
const opcode_t* FunctionStorage_Code(const struct wasm_function_storage* storage);
size_t FunctionStorage_CodeSize(const struct wasm_function_storage* storage);
size_t FunctionStorage_Signature(const struct wasm_function_storage* storage);
size_t FunctionStorage_ReturnCount(const struct wasm_function_storage* storage);
size_t FunctionStorage_LocalsCount(const struct wasm_function_storage* storage);
size_t FunctionStorage_ParameterCount(const struct wasm_function_storage* storage);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WASM_FUNCTION_STORAGE_H */
