#ifndef WASM_VALUE_H
#define WASM_VALUE_H

#include "wasm_base.h"
#include <cstdint>

namespace wasm {

typedef std::uint32_t  wasm_uint32_t;
typedef std::uint64_t  wasm_uint64_t;
typedef std::int32_t   wasm_sint32_t;
typedef std::int64_t   wasm_sint64_t;
typedef float 	       wasm_float32_t;
typedef double 	       wasm_float64_t;

struct WasmValue {
	
	WasmValue() = delete;

	WasmValue(const TaggedWasmValue&) = default;
	WasmValue(TaggedWasmValue&&) = default;

	WasmValue& operator=(const TaggedWasmValue&) = default;
	WasmValue& operator=(TaggedWasmValue&&) = default;

	explicit WasmValue(wasm_sint32_t i32_init):  i32(i32_init) { }
	explicit WasmValue(wasm_sint64_t i64_init):  i64(i64_init) { }
	explicit WasmValue(wasm_float32_t f32_init): f32(f32_init) { }
	explicit WasmValue(wasm_float64_t f64_init): f64(f64_init) { }

	template <class T, class Initializer>
	WasmValue(T WasmValue::* active_member, Initializer&& init):
		WasmValue(static_cast<T>(std::forward<Initializer>(init))
	{
		
	}

	template <class T>
	explicit WasmValue(T WasmValue::* active_member):
		WasmValue(active_member, T(0))
	{
		alignas(alignof(*this)) const char zer[sizeof(*this)] = {0};
		std::memcpy(this, zer, sizeof(*this));
		this->*active_member = 0;
	}

	explicit WasmValue(LanguageType lt)
	{
		switch(lt)
		{
		case LanguageType::i32: this->i32 = 0; break;
		case LanguageType::i64: this->i64 = 0; break;
		case LanguageType::f32: this->f32 = 0; break;
		case LanguageType::f64: this->f64 = 0; break;
		default:
			assert(false);
		}
	}

	template <class T>
	const T& operator->*(T WasmValue::* p) const
	{ return this->*p; }

	template <class T>
	const T& operator->*(const T WasmValue::* p) const
	{ return this->*p; }

	template <class T>
	T& operator->*(T WasmValue::* p)
	{ return this->*p; }

	
	template <class T>
	const T& get(T WasmValue::* p) const
	{ return this->*p; }

	template <class T>
	const T& get(const T WasmValue::* p) const
	{ return this->*p; }

	template <class T>
	T& get(T WasmValue::* p)
	{ return this->*p; }

	union {
		wasm_sint32_t  i32;
		wasm_sint64_t  i64 = 0;
		wasm_float32_t f32;
		wasm_float64_t f64;
		wasm_uint32_t  _u32;
		wasm_uint64_t  _u64;
	};
};

static_assert(std::is_trivially_copyable_v<WasmValue>);

std::ostream& operator<<(std::ostream& os, WasmValue value)
{
	wasm_sint32_t i32;
	wasm_sint64_t i64;
	wasm_float32_t f32;
	wasm_float64_t f64;
	std::memcpy(&i32, &value, sizeof(i32));
	std::memcpy(&i64, &value, sizeof(i64));
	std::memcpy(&f32, &value, sizeof(f32));
	std::memcpy(&f64, &value, sizeof(f64));
	os << "Value(";
	os << "i32 = " << i32;
	os << ", i64 = " << i32;
	os << ", f32 = " << f32;
	os << ", f64 = " << f64;
	os << ')';
	return os;
}

# ifdef __cplusplus

} /* extern "C" */

namespace tp {

// pointers-to-members for abstraction over member access
inline constexpr wasm_sint32_t  WasmValue::* const i32 = &WasmValue::i32;
inline constexpr wasm_sint64_t  WasmValue::* const i64 = &WasmValue::i64;
inline constexpr wasm_float32_t WasmValue::* const f32 = &WasmValue::f32;
inline constexpr wasm_float64_t WasmValue::* const f64 = &WasmValue::f64;

inline constexpr const wasm_sint32_t  WasmValue::* const i32_c = &WasmValue::i32;
inline constexpr const wasm_sint64_t  WasmValue::* const i64_c = &WasmValue::i64;
inline constexpr const wasm_float32_t WasmValue::* const f32_c = &WasmValue::f32;
inline constexpr const wasm_float64_t WasmValue::* const f64_c = &WasmValue::f64;

template <class T>
inline constexpr const auto member()
{
	using type = std::remove_const_t<T>;
	static_assert(
		std::is_same_v<type, std::decay_t<type>
		"T must be a (possible const) non-reference, non-pointer type in wasm::tp::member<T>()."
	);
	static_assert(
		std::disjunction_v<
			std::is_same<type, wasm_sint32_t>,
			std::is_same<type, wasm_sint64_t>,
			std::is_same<type, wasm_float32_t>,
			std::is_same<type, wasm_float64_t>
		>,
		"WasmValue has no member of the given type."
	);
	constexpr bool is_const = not std::is_same_v<T, type>;
	if constexpr(is_const)
	{
		if constexpr(std::is_same_v<type, wasm_sint32_t>)
			return i32_c;
		else if constexpr(std::is_same_v<type, wasm_sint64_t>)
			return i64_c;
		else if constexpr(std::is_same_v<type, wasm_float32_t>)
			return f32_c;
		else if constexpr(std::is_same_v<type, wasm_float64_t>)
			return f64_c;
		else
			assert(false and "Unreachable.");
	}
	else
	{
		if constexpr(std::is_same_v<type, wasm_sint32_t>)
			return i32;
		else if constexpr(std::is_same_v<type, wasm_sint64_t>)
			return i64;
		else if constexpr(std::is_same_v<type, wasm_float32_t>)
			return f32;
		else if constexpr(std::is_same_v<type, wasm_float64_t>)
			return f64;
		else
			assert(false and "Unreachable.");
	}
}

template <class Visitor>
decltype(auto) visit_value_type(Visitor&& vis, LanguageType value_type_v)
{
	switch(value_type_v)
	{
	case LanguageType::i32:
		return std::forward<Visitor>(tp::i32);
	case LanguageType::i64:
		return std::forward<Visitor>(tp::i64);
	case LanguageType::f32:
		return std::forward<Visitor>(tp::f32);
	case LanguageType::f64:
		return std::forward<Visitor>(tp::f64);
	default:
		assert(false);
	}
}

} /* namespace tp */

struct BadWasmValueAccess:
	public std::logic_error
{
	BadWasmValueAccess():
		std::logic_error("Attempt to access inactive member of WasmValue instance.")
	{
		
	}

	virtual ~BadWasmValueAccess() = default;
};

struct TaggedWasmValue {

	TaggedWasmValue() = delete;

	TaggedWasmValue(const TaggedWasmValue&) = default;
	TaggedWasmValue(TaggedWasmValue&&) = default;

	TaggedWasmValue& operator=(const TaggedWasmValue&) = default;
	TaggedWasmValue& operator=(TaggedWasmValue&&) = default;

	explicit TaggedWasmValue(wasm_sint32_t i32_init):
		tag_(LanguageType::i32), value_()
	{ value_.i32 = i32_init; }

	explicit TaggedWasmValue(wasm_sint64_t i64_init):
		tag_(LanguageType::i64), value_()
	{ value_.i64 = i64_init; }
	
	explicit TaggedWasmValue(wasm_float32_t f32_init):
		tag_(LanguageType::f32), value_()
	{ value_.f32 = f32_init; }

	explicit TaggedWasmValue(wasm_float64_t f64_init):
		tag_(LanguageType::f64), value_()
	{ value_.f64 = f64_init; }

	template <class T, class Initializer>
	TaggedWasmValue(T WasmValue::* active_member, Initializer&& init):
		tag_(language_type_value_v<T>),
		value_(static_cast<T>(std::forward<Initializer>(init)))
	{
		
	}
	
	template <class T>
	explicit TaggedWasmValue(T WasmValue::* active_member):
		TaggedWasmValue(active_member, T(0))
	{ set_zero(); }

	explicit TaggedWasmValue(LanguageType lt):
		TaggedWasmValue(tp::i64, 0)
	{
		auto visitor = [&](const auto& self, auto p){
			self.value_.*p = 0;
		};
		tp::visit_value_type(visitor, lt);
		tag_ = lt;
	}

	template <class T>
	const T& operator->*(T WasmValue::* p) const
	{ return this->value_.*p; }

	template <class T>
	const T& operator->*(const T WasmValue::* p) const
	{ return this->value_.*p; }

	template <class T>
	T& operator->*(T WasmValue::* p)
	{ return this->value_.*p; }

	template <class T>
	const T& operator->*(T WasmValue::* p) const
	{
		return this->value_.*p;
	}

	template <class T>
	const T& operator->*(const T WasmValue::* p) const
	{ return this->value_.*p; }

	template <class T>
	T& operator->*(T WasmValue::* p)
	{ return this->value_.*p; }

	
	explicit operator WasmValue() const
	{
		WasmValue v;
		auto visitor = [&](const auto& self, auto p){
			v.*p = self.value_.*p;
		};
		tp::visit_value_type(visitor, tag());
		return v;
	}

	void set_zero() {
		constexpr alignas(alignof(value_)) const char buff[sizeof(value_)] = {0};
		std::memcpy(&value_, buff, sizeof(value_));
	}

	template <class T>
	constexpr bool holds() const
	{ return tag() == language_type_value_v<T>; }

	template <class T>
	constexpr bool holds(T WasmValue::* active_member) const
	{ return this->holds<std::decay_t<T>>(); }

	constexpr bool holds(LanguageType tp) const
	{ return tag() == tp; }

	template <LanguageType Tp>
	constexpr bool holds() const
	{ return holds(Tp); }

	template <class T>
	void ensure_holds() const
	{ 
		if(not holds<T>())
			throw BadWasmValueAccess();
	}

	template <class T>
	void ensure_holds(T WasmValue::* active_member) const
	{
		if(not holds(active_member))
			throw BadWasmValueAccess();
	}

	void ensure_holds(LanguageType tp) const
	{
		if(not holds(tp))
			throw BadWasmValueAccess();
	}

	template <LanguageType Tp>
	void ensure_holds() const
	{
		if(not holds<Tp>())
			throw BadWasmValueAccess();
	}

	constexpr LanguageType tag() const
	{ return tag_; }

	template <class T>
	friend const T& get(const TaggedWasmValue& self)
	{
		self.ensure_holds<std::decay_t<T>>();
		return self->*tp::member<type>();
	}

	template <class T>
	friend decltype(auto) get(TaggedWasmValue& self)
	{
		self.ensure_holds<std::decay_t<T>>();
		return self->*tp::member<type>();
	}

	template <class Vis>
	friend decltype(auto) visit(Vis&& vis, const TaggedWasmValue& self)
	{
		return tp::visit_value_type(
			[&](auto WasmValue::* p) -> decltype(auto) {
				return std::invoke(std::forward<Vis>(vis), self->*p)
			},
			self.tag()
		);
	}

	template <class Vis>
	friend decltype(auto) visit(Vis vis, TaggedWasmValue& self)
	{
		return tp::visit_value_type(
			[&](auto WasmValue::* p) -> decltype(auto) {
				return std::invoke(std::forward<Vis>(vis), self->*p)
			},
			self.tag()
		);
	}

private:
	LanguageType tag_;
	WasmValue value_;
};

std::ostream& operator<<(std::ostream& os, const TaggedWasmValue& val)
{ return visit([](auto v) { return os << v; }, val); }



} /* namespace wasm */

#endif /* WASM_VALUE_H */
