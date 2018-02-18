#include "frontend/create_program.h"
#include "interpreter/wasm_interpreter.h"


int main(int argc, const char* argv[])
{
	std::size_t call_stack_size = 1000;
	std::size_t control_flow_stack_size = 1000;
	std::size_t value_stack_size = 1000;
	wasm_program_state program = create_program(argc, argv);
	wasm_call_stack call_stack(call_stack_size);
	wasm_control_flow_stack control_flow_stack(control_flow_stack_size);
	std::vector<wasm_value_t> value_stack(value_stack_size, wasm_value_t{wasm_uint64_t(0)});
	wasm_runtime interpreter(program, call_stack, control_flow_stack, value_stack.data());

	return 0;
}
