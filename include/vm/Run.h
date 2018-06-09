#ifndef VM_RUN_H
#define VM_RUN_H

#include "vm/CallStack.h"

namespace wasm {


template <class ValueType>
struct RunContext {

	RunContext(WasmModule& module, gsl::span<char> space):
		stack_resource_(space),
		call_stack_(stack_resource_),
		module_(module)
	{
		
	}

	std::size_t stack_usage() const
	{ return resource().inspect().size(); }

	std::size_t stack_capacity() const
	{ return resource().capacity(); }

	const WasmModule& module() const
	{ return module_; }

private:
	const StackResource& resource() const
	{ return stack_resource_; }

	StackResource& resource()
	{ return stack_resource_; }

	StackResource stack_resource_;
	CallStack<ValueType> call_stack_;
	WasmModule& module_;
};

} /* namespace wasm */




#endif /* VM_RUN_H */
