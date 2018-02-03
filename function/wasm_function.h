#ifndef WASM_FUNCTION_H
#define WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include "wasm_value.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <random>
#include "utilities/hash_combine.h"

using func_sig_id_t = std::size_t;

struct late_registration_error: 
	public std::runtime_error
{
	late_registration_error():
		std::runtime_error(
				"Attempt to register function "
				"with registrar that has disposed "
				"of it's contents!")
	{
		
	}
};


struct FunctionSignatureRegistrar
{
	using typecode_t = std::underlying_type_t<wasm_language_type>;
	using sig_string_t = std::basic_string<typecode_t>;
	using sig_string_view_t = std::basic_string_view<typecode_t>;


	template <class TypeString>
	func_sig_id_t get_signature(TypeString&& ts, std::size_t param_count)
	{
		if(id_count == 0)
			throw late_registration_error();
		FuncSig sig(sig_string_view_t(ts), param_count);
		if(auto pos = signatures.find(sig); pos != signatures.end())
			return pos->second;
		else
		{
			sig.types = std::forward<TypeString>(ts);
			auto [pos, occured] signatures.emplace(std::move(sig), id_count++);
			assert(occured);
			backmap.emplace(pos->second, &(pos->first));
			return pos->second;
		}
	}

	std::size_t get_parameter_count_for(func_sig_id_t sig_id) const
	{
		auto pos = backmap.find(sig_id);
		if(pos == backmap.end())
		{
			throw_bad_signature_access();
			return 0;
		}
		else
		{
			return pos->second->param_count;
		}
	}
	
	std::size_t get_return_count_for(func_sig_id_t sig_id) const
	{
		auto pos = backmap.find(sig_id);
		if(pos == backmap.end())
		{
			throw_bad_signature_access();
			return 0;
		}
		else
		{
			return std::get<0>(pos->second->types).size() - pos->second->param_count;
		}
	}

	void dispose(bool debug_mode = false) 
	{
		// keep signatures around if in hypothetical future "debug mode".
		if(not debug_mode)
			signatures.clear();
		id_count = 0;
	}
private:
	void throw_bad_signature_access(func_sig_id_t id)
	{
		throw std::out_of_range("Attempt to access non-existent "
					"function signature information with id " 
					+ std::to_string(sig_id));
	}

	struct FuncSig{
		FuncSig():
			types(sig_string_t{}), param_count(0), 
			hash_v(compute_hash(types, param_count))
		{
			
		}

		template <class TypeString>
		FuncSig(TypeString&& ts, std::size_t pc):
			types(std::forward<TypeString>(ts)), param_count(pc), 
			hash_v(compute_hash(types, param_count))
		{
			
		}

		sig_string_view_t view() const noexcept
		{
			if(types.holds_alternative<sig_string_view_t>())
				return std::get<sig_string_view_t>();
			else 
				return std::get<sig_string_t>();
		}
		static std::size_t compute_hash(sig_string_view_t types, std::size_t param_count)
		{
			std::size_t t_hash = types_hasher(types);
			std::size_t p_hash = param_count_hasher(param_count);
			return hash_combine(t_hash, p_hash);
		}
		std::variant<sig_string_t, sig_string_view_t> types;
		const std::size_t param_count;
		const std::size_t hash_v;
	};

	struct FuncSigHasher {
		std::size_t operator()(const FuncSig& sig) const
		{
			return sig.hash_v;
		}
	};

	struct FuncSigEqual {
		
		bool operator()(const FuncSig& left, const FuncSig& right) const
		{
			return left.param_count == right.param_count 
				and left.view() == right.view();
		}
	};

	static constexpr std::hash<sig_string_view_t> types_hasher;
	static constexpr std::hash<std::size_t> param_count_hasher;
	using signature_defs_t = std::unordered_map<FuncSig, const func_sig_id_t, FuncSigHasher, FuncSigEqual>;
	using backmap_t = std::unordered_map<func_sig_id_t, const FuncSig*>;

	signature_defs_t signatures;
	signature_defs_t backmap;
	func_sig_id_t id_count = 1;
};


struct wasm_function_storage;
struct wasm_function
{
	using opcode_t = std::underlying_type_t<wasm_instruction>;
	using code_string_t = std::basic_string<opcode_t>;
	wasm_function(const code_string_t& code_str, func_sig_id_t sig, std::size_t nparams, std::size_t nlocals);
	const opcode_t* code() const;
	std::size_t code_size() const;
	func_sig_id_t signature() const;
	std::size_t return_count() const;
	std::size_t locals_count() const;
	std::size_t parameter_count() const;
private:
	struct WasmFunctionDeleter {
		void operator()(const void* func_storage) const;
	};
	using storage_ptr_t = std::unique_ptr<const wasm_function_storage, WasmFunctionDeleter>;
	storage_ptr_t func_storage;
};


#endif /* WASM_FUNCTION_H */
