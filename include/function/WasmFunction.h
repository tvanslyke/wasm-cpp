#ifndef FUNCTION_WASM_FUNCTION_H
#define FUNCTION_WASM_FUNCTION_H
#include "WasmInstruction.h"
#include <string>

namespace wasm {

struct WasmFunction
{
	using wasm_external_kind_type = WasmFunctionSignature;
	using opcode_type = wasm::opc::OpCode;
	using string_type = std::basic_string<opcode_type>;
	using code_view_type = std::basic_string_view<opcode_type>;

	WasmFunction(const WasmFunctionSignature& sig, parse::FunctionBody&& body):
		name_(""),
		sig_(sig),
		code_(std::move(body.code)),
		locals_(body.locals.begin(), body.locals.end())
	{
		body.locals.clear(); // release this memory sooner rather than later.
	}
		
	friend std::string_view name(const WasmFunction& f)
	{ return f.name_; }

	friend code_view_type code(const WasmFunction& f)
	{ return f.code_; }

	friend const WasmFunctionSignature& signature(const WasmFunction& f) 
	{ return f.signature_; }

	friend gsl::span<const LanguageType> locals(const WasmFunction& f)
	{ return gsl::span<const LanguageType>(locals_.data(), locals_.size()); }

	
	friend void assign_name(WasmFunction& f, std::string&& name)
	{
		assert(name_.empty());
		f.name_ = std::move(name);
	}
private:
	std::string name_;
	const WasmFunctionSignature sig_;
	const std::basic_string<opcode_type> code_;
	const SimpleVector<LanguageType> locals_;
};

auto return_count(const WasmFunction& f)
{ return return_count(signature(f)); }

auto param_count(const WasmFunction& f)
{ return param_count(signature(f)); }

auto locals_count(const WasmFunction& f) 
{ return locals(f).size(); }

bool matches(const WasmFunction& f, const WasmFunctionSignature& sig)
{ return signature(f) == sig; }


std::ostream& operator<<(std::ostream& os, const WasmFunction& f)
{
	os << "(func $" << name(f) << ' ' << signature(f) << " (\n";
	auto codeseq = code(f);
	parse::write_code(os, begin(code), end(code), 2);
	os << "  )\n";
	os << ')';
	return os;
}

std::ostream& write_declaration(std::ostream& os, const WasmFunction& f)
{
	return os << "(func $" << name(f) << ' ' << signature(f) << ')';
}
} /* namespace wasm */

#endif /* FUNCTION_WASM_FUNCTION_H */
