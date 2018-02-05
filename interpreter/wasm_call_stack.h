#ifndef INTERPRETER_WASM_CALL_STACK_H
#define INTERPRETER_WASM_CALL_STACK_H

#include <stddef.h>


typedef struct wasm_call_stack_ wasm_call_stack;


// constructor
wasm_call_stack* WasmCallStack_New(size_t size);


// destructor
wasm_frame* WasmCallStack_Delete(wasm_call_stack* self);


// accesors
const opcode_t* WasmCallStack_Code(const wasm_call_stack* self);


// modifiers
int WasmCallStack_PushFrame(wasm_call_stack* self, const struct wasm_function_storage* func);

void WasmCallStack_FastPopFrame(wasm_call_stack* self);

int WasmCallStack_PopFrame(wasm_call_stack* self);

const opcode_t* WasmCallStack_CodeNext(wasm_frame* self);

const opcode_t* WasmCallStack_CodeAdvance(wasm_frame* self, size_t count);


#endif /* INTERPRETER_WASM_CALL_STACK_H */
