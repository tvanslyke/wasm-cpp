#ifndef MODULE_WASM_GLOBAL_H
#define MODULE_WASM_GLOBAL_H
#include <variant>
#include <sstream>
#include "wasm_base.h"

namespace wasm {

struct BadGlobalAccess:
	public ValidationError<std::exception>
{
	using base_type = ValidationError<std::exception>;
	using base_type::base_type;

};

struct ConstGlobalWriteError:
	BadGlobalAccess
{
	ConstGlobalWriteError():
		ConstGlobalWriteError("Attempt to write immutable global.")
	{
		
	}
};

struct GlobalTypeMismatch:
	BadGlobalAccess
{
private:
	static std::string msg(LanguageType actual, LanguageType attempted)
	{
		std::ostringstream s;
		s << "Attempt to access global variable of type ";
		s << actual;
		s << " as a value of type ";
		s << attempted;
		s << '.';
	}
public:
	GlobalTypeMismatch(LanguageType actual, LanguageType attempted)
		GlobalTypeMismatch(msg(actual, attempted))
	{
		
	}
};

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
	using wasm_external_kind_type = parse::GlobalType;
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

	parse::GlobalType get_type() const
	{
		if(has_dependency())
			return get_dependency().type();
		else
			return parse::GlobalType(get_language_type(), is_const())
	}

	LanguageType get_language_type() const
	{
		if(has_dependency())
			return get_dependency().second.type();
		return static_cast<LanguageType>(
			-1 * (std::ptrdiff_t(value_.index()) - 1)
		);
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

	void init_dep(const WasmGlobal& dep)
	{
		assert(this->has_dependency());
		assert(not other.has_dependency());
		assert(this->get_language_type() == dep.get_language_type());
		if(get_type().is_const())
		{
			switch(this->get_language_type())
			{
			case LanguageType::i32:
				value_.emplace<const wasm_sint32_t>(read<wasm_sint32_t>(dep));
				break;
			case LanguageType::i64:
				value_.emplace<const wasm_sint64_t>(read<wasm_sint64_t>(dep));
				break;
			case LanguageType::f32:
				value_.emplace<const wasm_float32_t>(read<wasm_float32_t>(dep));
				break;
			case LanguageType::f64:
				value_.emplace<const wasm_float64_t>(read<wasm_float64_t>(dep));
				break;
			default:
				assert(false);
			}
		}
		else
		{
			switch(this->get_language_type())
			{
			case LanguageType::i32:
				value_.emplace<wasm_sint32_t>(read<wasm_sint32_t>(dep));
				break;
			case LanguageType::i64:
				value_.emplace<wasm_sint64_t>(read<wasm_sint64_t>(dep));
				break;
			case LanguageType::f32:
				value_.emplace<wasm_float32_t>(read<wasm_float32_t>(dep));
				break;
			case LanguageType::f64:
				value_.emplace<wasm_float64_t>(read<wasm_float64_t>(dep));
				break;
			default:
				assert(false);
			}
		}
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

	explicit operator TaggedWasmValue() const
	{
		return std::visit(
			[](auto v) {
				if constexpr(std::is_same_v<decltype(v), dependency_type>)
					throw BadGlobalAccess("Attempt to access global before it has been initialized.");
				else
					return static_cast<TaggedWasmValue>(v);
			},
			value_
		);
	}

	explicit operator WasmValue() const
	{
		return static_cast<WasmValue>(static_cast<TaggedWasmValue>(*this));
	}
private:
	variant_type value_;
};

bool matches(const WasmGlobal& self, const parse::GlobalType& tp)
{ return self.get_type() == tp; }

void set(WasmGlobal& self, TaggedWasmValue v)
{
	if(self.get_language_type() != v.tag())
		throw GlobalTypeMismatch(self.get_language_type(), v.tag());
	return set(self, static_cast<WasmValue>(v));
}

void set(WasmGlobal& self, WasmValue v)
{
	if(self.has_dependency())
		throw BadGlobalAccess("Attempt to write global before it has been initialized.");
	if(self.is_const())
		throw ConstGlobalWriteError();
	tp::visit_value_type(
		[&](auto WasmValue::* p) {
			auto value = v.*p;
			using type = decltype(value);
			get<type>(self) = value;
		},
		self.get_language_type()
	);
}

template <class U>
void set(WasmGlobal& self, U WasmValue::* p, U value)
{ set(self, TaggedWasmValue(p, value)); }

void set(WasmValue& self, const WasmGlobal& g)
{ self = static_cast<WasmValue>(g); }

void set(TaggedWasmValue& self, const WasmGlobal& g)
{
	if(g.get_language_type() != self.tag())
		throw GlobalTypeMismatch(self.get_language_type(), v.tag());
	set(self, static_cast<TaggedWasmValue>(self));
}



template <class T>
const std::decay_t<T>& operator->*(const WasmGlobal& self, T WasmValue::* p) {
	return read<std::decay_t<T>>(self);
};

template <class T>
const std::decay_t<T>& operator->*(WasmGlobal& self, const T WasmValue::* p) {
	return std::as_const(self)->*p;
};

template <class T>
T& operator->*(WasmGlobal& self, T WasmValue::* p) {
	return get<T>(self);
};

} /* namespace wasm */
#endif /* MODULE_WASM_GLOBAL_H */
