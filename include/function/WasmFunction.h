#ifndef FUNCTION_WASM_FUNCTION_H
#define FUNCTION_WASM_FUNCTION_H
#include "wasm_instruction.h"
#include <string>


struct WasmFunction
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	using code_t = std::basic_string<opcode_t>;
	using type_sequence_t = std::basic_string<signed char>;
	WasmFunction(
		code_t&& code_string, 
		std::size_t sig,
		std::size_t params, 
		std::size_t returns,
		std::size_t locals,
		std::string&& name_str
	):
		code_(std::move(code_string)),
		signature_id_(sig),
		param_count_(params),
		return_count_(returns),
		locals_count_(locals),
		name_(name_str)
	{
		
	}
		
	const opcode_t* code() const
	{ return code_.c_str(); }

	std::size_t code_size() const
	{ return code_.size(); }

	std::size_t signature() const
	{ return signature_id_; }

	std::size_t param_count() const
	{ return param_count_; }

	std::size_t return_count() const
	{ return return_count_; }

	std::size_t locals_count() const
	{ return locals_count_; }

	const std::string& name() const
	{ return name_; }

private:
	const std::basic_string<opcode_t> code_;
	const std::size_t signature_id_;
	const std::size_t param_count_;
	const std::size_t return_count_;
	const std::size_t locals_count_;
	const std::string name_;

};


#endif /* FUNCTION_WASM_FUNCTION_H */
