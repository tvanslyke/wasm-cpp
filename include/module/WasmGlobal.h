#ifndef MODULE_WASM_GLOBAL_H
#define MODULE_WASM_GLOBAL_H
#include <variant>
#include "wasm_base.h"


struct WasmGlobal
{
	using dependency_type = std::pair<wasm_uint32_t, parse::GlobalType>;
	using variant_type = std::variant<
		dependency_type, // dependency on another global
		wasm_sint32_t,
		wasm_sint64_t,
		wasm_float32_t,
		wasm_float64_t,
		const wasm_sint32_t,
		const wasm_sint64_t,
		const wasm_float32_t,
		const wasm_float64_t
	>;
	using base_type::base_type;

	WasmGlobal() = delete;
	
	WasmGlobal(const parse::GlobalEntry& ent):
		value_(std::in_place_type<wasm_sint32_t>(-1)) // temporary - fix in body
	{
		if(ent.depends)
		{
			value_.emplace<dependency_type>(*ent.depends, global_type(ent));
		}
		else
		{
			std::visit(
				[&](auto val) {
					if(ent.value.is_const)
						value_.emplace<const decltype(val)>(val);
					else
						value_.emplace<decltype(val)>(val);
				},
				ent.value.value
			);
		}
	}

	bool has_dependency() const
	{ return std::holds_alternative<dependency_type>(value_); }

	dependency_type get_dependency()
	{
		assert(has_dependency());
		return std::get<dependency_type>(value_);
	}

	bool is_const() const
	{ return value_.index() > 4u; }

	bool is_mut() const
	{
		auto idx = value_.index();
		return idx <= 4u and idx > 0u;
	}

	template <class T>
	void init_mut(T&& value)
	{
		assert(has_dependency() and "Can't set the global's type after it has been initialized.");
		using type = std::decay_t<T>;
		value_.emplace<std::decay_t<T>>(std::forward<T>(T));
	}

	template <class T>
	void init_const(T&& value)
	{
		assert(has_dependency() and "Can't set the global's type after it has been initialized.");
		using type = std::decay_t<T>;
		value_.emplace<const std::decay_t<T>>(std::forward<T>(T));
	}

	template <class T>
	friend const T& get(const WasmGlobal& self)
	{ return std::get<T>(self); }

	template <class T>
	friend T& get(WasmGlobal& self)
	{ return std::get<T>(self); }

	template <class T>
	friend const T* get_if(const WasmGlobal& self)
	{ return std::get_if<T>(self); }

	template <class T>
	friend T* get_if(WasmGlobal& self)
	{ return std::get_if<T>(self); }

	template <class T>
	friend T read(const WasmGlobal& self)
	{
		if(const auto* p = get_if<const T>(self); static_cast<bool>(p));
			return *p;
		else
			return get<T>(self);
	}

	template <class T>
	friend std::optional<T> read_if(const WasmGlobal& self)
	{
		if(const auto* p = get_if<const T>(self); static_cast<bool>(p));
			return *p;
		if(const auto* p = get_if<T>(self); static_cast<bool>(p));
			return *p;
		return std::nullopt;
	}

private:
	variant_type value_;
};


#endif /* MODULE_WASM_GLOBAL_H */
