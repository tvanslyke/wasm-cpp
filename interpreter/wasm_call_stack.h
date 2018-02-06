#ifndef INTERPRETER_WASM_CALL_STACK_H
#define INTERPRETER_WASM_CALL_STACK_H

#include "interpreter/WasmCallStack.h"
#include "function/wasm_function.h"
#include <memory>
#include <cstddef>

struct wasm_call_stack
{
	wasm_call_stack(std::size_t size):
		impl(WasmCallStack_New(size))
	{
		if(not impl)
			throw std::bad_alloc();
	}

	const opcode_t* code() const
	{ return WasmCallStack_Code(self()); }

	const opcode_t* code_next() 
	{ return WasmCallStack_CodeNext(self()); }

	const opcode_t* code_advance(std::size_t count) 
	{ return WasmCallStack_CodeAdvance(self(), count); }

	const opcode_t* code_jump(const opcode_t* label)
	{ return WasmCallStack_CodeJump(self(), label); }

	bool try_pop_frame() 
	{ return !WasmCallStack_PopFrame(self()); }
		
	void fast_pop_frame() 
	{ WasmCallStack_FastPopFrame(self()); }

	bool push_frame(const wasm_function& func) 
	{ return !WasmCallStack_PushFrame(self(), func.get()); }

	wasm_value_t* locals() 
	{ return WasmCallStack_Locals(self()); }

private:

	const WasmCallStack* self() const
	{ return impl.get(); }

	WasmCallStack* self() 
	{ return impl.get(); }

	struct WasmCallStackDeleter {
		
		void operator()(void* mem) const
		{
			WasmCallStack_Delete(mem);
		}
	};

	std::unique_ptr<WasmCallStack, WasmCallStackDeleter> impl;
};


#endif /* INTERPRETER_WASM_CALL_STACK_H */
