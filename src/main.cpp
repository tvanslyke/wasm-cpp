#include "frontend/create_program.h"
#include "interpreter/wasm_interpreter.h"
#include "interpreter/WasmProgramStack.h"
#include <iomanip>

void escape_value(void* value)
{
	asm volatile("" : : "g"(value) : "memory");
}

int main(int argc, const char* argv[])
{
	std::size_t stack_size = 1000;
	std::vector<wasm_value_t> stack(stack_size, zero_wasm_value());
	wasm_program_state program_state = create_program(argc, argv);
	WasmProgramStack call_stack(stack.data(), stack.data() + stack.size());
	wasm_runtime interpreter(
		program_state, 
		call_stack
	);
	
	wasm_runtime cpy(interpreter);
	try
	{
		while(interpreter.eval())
		{
			/* LOOP */
		}
	} 
	catch(const trap_error&)
	{
		while(call_stack.frame_count() > 0)
		{
			std::cerr << call_stack.debug_frame() << std::endl;
			if(call_stack.frame_count() == 1)
				break;
			call_stack.return_op();
		}
		throw;
	}
	auto nreturns = (call_stack.const_stack_pointer() - stack.data());
	assert(nreturns == 0 or nreturns == 1);
	if(nreturns)
		return stack[0].s32;
	else
		return 0;
}
