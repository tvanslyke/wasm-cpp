#include <Python.h>
#include "interpreter/wasm_interpreter.h"



struct WasmDebugger:
	public PyObject
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	PyObject* value_stack_view()
	{
		return PyMemoryView_FromMemory(
			reinterpret_cast<char*>(runtime.value_stack.data()), runtime.value_stack.size(), PyBUF_READ
		);
	}
	
	PyObject* value_stack_snapshot()
	{
		return PyBytes_FromStringAndSize(
			reinterpret_cast<char*>(runtime.value_stack.data()), runtime.value_stack.size()
		);
	}
	
	PyObject* value_stack_snapshot()
	{
		return PyBytes_FromStringAndSize(
			reinterpret_cast<char*>(runtime.value_stack.data()), runtime.value_stack.size()
		);
	}
	
	bool step()
	{
		return runtime.eval();
	}
	
	opcode_t current_opcode()
	{
		return runtime.
	}
	



	wasm_runtime& runtime;
	WasmProgramStack& stack;
};



