#ifndef INTERPRETER_WASM_CALL_STACK_IMPL_H
#define INTERPRETER_WASM_CALL_STACK_IMPL_H

#include "wasm_value.h"
# include "function/wasm_function_storage.h"
#ifdef __cplusplus
# include <cstddef>
# include "wasm_instruction.h"
using opcode_t = wasm_opcode::wasm_opcode_t;
using std::size_t;
extern "C" {
#else
# include <stddef.h>
# include <stdint.h>
typedef uint_least8_t opcode_t;
#endif


typedef struct WasmCallStack_ WasmCallStack;


// constructor
WasmCallStack* WasmCallStack_New(size_t size);


// destructor
void WasmCallStack_Delete(void* self);


// accesors
const opcode_t* WasmCallStack_Code(const WasmCallStack* self);


// modifiers
int WasmCallStack_PushFrame(WasmCallStack* self, const struct wasm_function_storage* func);

void WasmCallStack_FastPopFrame(WasmCallStack* self);

int WasmCallStack_PopFrame(WasmCallStack* self);

const opcode_t* WasmCallStack_CodeNext(WasmCallStack* self);

const opcode_t* WasmCallStack_CodeAdvance(WasmCallStack* self, size_t count);


const opcode_t* WasmCallStack_CodeJump(WasmCallStack* self, const opcode_t* label);


wasm_value_t* WasmCallStack_Locals(WasmCallStack* self);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INTERPRETER_WASM_CALL_STACK_IMPL_H */
