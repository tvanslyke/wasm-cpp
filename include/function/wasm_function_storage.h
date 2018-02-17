#ifndef WASM_FUNCTION_STORAGE_H
#define WASM_FUNCTION_STORAGE_H

#ifdef __cplusplus
# include <cstddef>
# include <type_traits>
# include "wasm_instruction.h"
using opcode_t = wasm_opcode::wasm_opcode_t;
using std::size_t;
extern "C" {
#else
# include <stddef.h>
# include <stdint.h>
typedef uint_least8_t opcode_t;
#endif

struct wasm_function_storage;


// constructor
const struct wasm_function_storage*
FunctionStorage_New(const opcode_t* code_begin, const opcode_t* code_end, size_t sig, size_t nlocals);

// destructor
void FunctionStorage_Delete(const struct wasm_function_storage* storage);


// accessors
const opcode_t* FunctionStorage_Code(const struct wasm_function_storage* storage);
size_t FunctionStorage_CodeSize(const struct wasm_function_storage* storage);
size_t FunctionStorage_Signature(const struct wasm_function_storage* storage);
size_t FunctionStorage_LocalsCount(const struct wasm_function_storage* storage);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WASM_FUNCTION_STORAGE_H */
