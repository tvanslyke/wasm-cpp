#include "function/wasm_function.h"
#include "function/wasm_function_storage.h"

wasm_function::wasm_function(const code_string_t& code_str, func_sig_id_t sig, 
				std::size_t nparams, std::size_t nlocals, std::size_t nreturns):
	func_storage(FunctionStorage_New(
		code_str.data(), code_str.data() + code_str.size(),
		nparams, nlocals, nreturns, sig))
{
	if(not func_storage)
		throw std::bad_alloc();
}

const opcode_t* wasm_function::code() const
{
	return FunctionStorage_Code(func_storage.get());
}

std::size_t wasm_function::code_size() const
{
	return FunctionStorage_CodeSize(func_storage.get());
}

func_sig_id_t wasm_function::signature() const
{
	return FunctionStorage_Signature(func_storage.get());
}

std::size_t wasm_function::return_count() const
{
	return FunctionStorage_ReturnCount(func_storage.get());
}

std::size_t wasm_function::locals_count() const
{
	return FunctionStorage_LocalsCount(func_storage.get());
}

std::size_t wasm_function::parameter_count() const
{
	return FunctionStorage_ParameterCount(func_storage.get());
}

void wasm_function::WasmFunctionDeleter::operator()(const void* func_storage) const
{
	using storage_t = const wasm_function_storage*;
	FunctionStorage_Delete(static_cast<storage_t>(func_storage));
}



