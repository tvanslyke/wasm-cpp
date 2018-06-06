#ifndef VM_ARITH_H
#define VM_ARITH_H

#include <functional>
#include <type_traits>
#include <sstream>
#include <cmath>
#include <cfenv>
#include "vm/op/errors.h"

namespace wasm::ops::arith {

template <class Integer>
std::make_unsigned_t<Integer> to_unsigned(Integer value)
{
	static_assert(std::is_integral_v<Integer>);
	static_assert(std::is_signed_v<Integer>);
	return reinterpret_cast<const std::make_unsigned_t<Integer>&>(value);
}

template <class Integer>
std::make_signed_t<Integer> to_signed(Integer value)
{
	static_assert(std::is_integral_v<Integer>);
	static_assert(std::is_unsigned_v<Integer>);
	return reinterpret_cast<const std::make_signed_t<Integer>&>(value);
}

wasm_sint64_t sign_extend(wasm_sint32_t value)
{
	constexpr wasm_uint64_t signbit = wasm_uint64_t(1u) << 31u;
	return to_signed((to_unsigned(value) ^ signbit) - signbit);
}

wasm_uint64_t sign_extend_u(wasm_sint32_t value)
{ return to_unsigned(sign_extend(value)); }

wasm_sint32_t truncate(wasm_sint64_t value)
{
	constexpr wasm_uint64_t low_mask = ~wasm_uint32_t(0);
	wasm_uint32_t bits = to_unsigned(value);
	return to_signed(bits);
}

template <class T>
void ensure_nonzero(T value)
{
	if(value == T(0))
		throw DivisionByZeroError();
}

namespace detail {

template <class Fn>
auto decorate_signed(Fn fn) {
	return [=](auto ... args) {
		return to_signed(std::invoke(fn, to_unsigned(args)...));
	};
}

template <class Fn, class Dec>
auto decorate_return(Fn fn, Dec dec) {
	return [=](auto ... args) {
		return dec(fn(args...));
	};
}

template <class Function>
auto bool_to_i32(Function func)
{
	return [=](auto... args) -> wasm_sint32_t {
		static_assert(std::is_invocable_v<Function, decltype(args)...>);
		static_assert(std::is_same_v<bool, std::invoke_result_t<Function, decltype(args)...>>);
		return static_cast<wasm_sint32_t>(std::invoke(func, args...));
	};
}
} /* namesapce detail */


template <class T, class = std::enable_if_t<std::is_integral_v<T>>>
auto iabs_u(T value)
{
	if constexpr(std::is_signed_v<T>)
	{
		auto uvalue = to_unsigned(value);	
		if(value < 0)
			return std::make_pair((~uvalue) + 1u, true);
		return std::make_pair(uvalue, false);
	}
	else
	{
		return std::make_pair(value, false);
	}
}



inline const auto i32_add = detail::decorate_signed(std::plus<wasm_uint32_t>{});
inline const auto i64_add = detail::decorate_signed(std::plus<wasm_uint64_t>{});

inline const auto i32_sub = detail::decorate_signed(std::minus<wasm_uint32_t>{});
inline const auto i64_sub = detail::decorate_signed(std::minus<wasm_uint64_t>{});

inline const auto i32_mul = detail::decorate_signed(std::multiplies<wasm_uint32_t>{});
inline const auto i64_mul = detail::decorate_signed(std::multiplies<wasm_uint64_t>{});

inline const auto i32_div_s = [](wasm_sint32_t l, wasm_sint32_t r) {
	constexpr wasm_uint32_t negative_one = ~wasm_uint32_t(0);
	constexpr wasm_uint32_t i32_min = ~wasm_uint32_t(negative_one >> 1u);
	ensure_nonzero(r);
	auto [l_u, l_neg] = iabs_u(l);
	auto [r_u, r_neg] = iabs_u(r);
	if(r_neg and l_neg and r_u == 1u and ((~l_u) + 1u) == i32_min)
		throw OverflowError("Trap: Division of most negative i32 value by -1.");
	auto quot = l_u / r_u;
	if(l_neg != r_neg)
		return to_signed(quot) * -1;
	else
		return to_signed(quot);
};

inline const auto i64_div_s = [](wasm_sint64_t l, wasm_sint64_t r) {
	constexpr wasm_uint64_t negative_one = ~wasm_uint64_t(0);
	constexpr wasm_uint64_t i64_min = ~wasm_uint64_t(negative_one >> 1u);
	ensure_nonzero(r);
	auto [l_u, l_neg] = iabs_u(l);
	auto [r_u, r_neg] = iabs_u(r);
	if(r_neg and l_neg and r_u == 1u and ((~l_u) + 1u) == i64_min)
		throw OverflowError("Trap: Division of most negative i64 value by -1.");
	auto quot = l_u / r_u;
	if(l_neg != r_neg)
		return to_signed(quot) * -1;
	else
		return to_signed(quot);
};

inline const auto i32_div_u = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	ensure_nonzero(r);
	return to_signed(to_unsigned(l) / to_unsigned(r));
};

inline const auto i64_div_u = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	ensure_nonzero(r);
	return to_signed(to_unsigned(l) / to_unsigned(r));
};

inline const auto i32_rem_s = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	try
	{
		wasm_sint32_t quot = i32_div_s(l, r);
		return to_signed(
			to_unsigned(l) - to_unsigned(quot) * to_unsigned(r)
		);
	}
	catch(const OverflowError&)
	{
		return 0;
	}
};

inline const auto i64_rem_s = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	try
	{
		wasm_sint64_t quot = i64_div_s(l, r);
		return to_signed(
			to_unsigned(l) - to_unsigned(quot) * to_unsigned(r)
		);
	}
	catch(const OverflowError&)
	{
		return 0;
	}
};

inline const auto i32_rem_u = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	ensure_nonzero(r);
	return to_signed(to_unsigned(l) % to_unsigned(r));
};
inline const auto i64_rem_u = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	ensure_nonzero(r);
	return to_signed(to_unsigned(l) % to_unsigned(r));
};

inline const auto i32_and = std::bit_and<wasm_int32_t>{};
inline const auto i64_and = std::bit_and<wasm_int64_t>{};

inline const auto i32_or = std::bit_or<wasm_int32_t>{};
inline const auto i64_or = std::bit_or<wasm_int64_t>{};

inline const auto i32_xor = std::bit_xor<wasm_int32_t>{};
inline const auto i64_xor = std::bit_xor<wasm_int64_t>{};

inline const auto i32_shl = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	auto shift = to_unsigned(r) % 32u;
	return to_signed(to_unsigned(l) << shift);
};
inline const auto i64_shl = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	auto shift = to_unsigned(r) % 64u;
	return to_signed(to_unsigned(l) << shift);
};

inline const auto i32_shr_u = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	auto shift = to_unsigned(r) % 32u;
	return to_signed(to_unsigned(l) >> shift);
};
inline const auto i64_shr_u = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	auto shift = to_unsigned(r) % 64u;
	return to_signed(to_unsigned(l) >> shift);
};

inline const auto i32_shr_s = [](wasm_sint32_t l, wasm_sint32_t r) -> wasm_sint32_t {
	constexpr auto high_bit = ~((~wasm_uint32_t(0u)) >> 1u);
	auto shift = to_unsigned(r) % 32u;
	auto mask = wasm_uint32_t(0u);
	auto l_u = to_unsigned(l);
	if(static_cast<bool>(l_u & high_bit))
		mask = ~(~wasm_uint32_t(0u)) >> shift;
	return to_signed((l_u >> shift) | mask);
};
inline const auto i64_shr_s = [](wasm_sint64_t l, wasm_sint64_t r) -> wasm_sint64_t {
	constexpr auto high_bit = ~((~wasm_uint64_t(0u)) >> 1u);
	auto shift = to_unsigned(r) % 64u;
	auto mask = wasm_uint64_t(0u);
	auto l_u = to_unsigned(l);
	if(static_cast<bool>(l_u & high_bit))
		mask = ~(~wasm_uint64_t(0u)) >> shift;
	return to_signed((l_u >> shift) | mask);
};

namespace detail {

template <class T>
struct RotateRight {
	static_assert(std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>>);
	T operator()(T value, T shift) const
	{
		constexpr std::size_t bit_count = CHAR_BIT * sizeof(T);
		shift %= bit_count;
		return (value >> shift) | (value << (bit_count - shift));
	}
};

template <class T>
struct RotateLeft {
	static_assert(std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>>);
	T operator()(T value, T shift) const
	{
		constexpr std::size_t bit_count = CHAR_BIT * sizeof(T);
		shift %= bit_count;
		return (value << shift) | (value >> (bit_count - shift));
	}
};

template <class T>
struct CountLeadingZeros {
	static_assert(std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>>);
	T operator()(T value) const
	{
		constexpr std::size_t bit_count = CHAR_BIT * sizeof(T);
		constexpr T high_bit = T(1u) << (bit_count - 1u);
		if(value == 0u)
			return bit_count;
#ifdef __GNUC__
		if constexpr(std::is_same_v<T, unsigned int>)
			return __builtin_clz(value);
		else if constexpr(std::is_same_v<T, unsigned long>)
			return __builtin_clzl(value);
		else if constexpr(std::is_same_v<T, unsigned long long>)
			return __builtin_clzll(value);
#endif
		for(std::size_t i = 0; i < bit_count; ++i)
		{
			if(static_cast<bool>(high_bit & value))
				return i;
		}
		assert(false and "Unreachable.");
	}
};

template <class T>
struct CountTrailingZeros {
	static_assert(std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>>);
	T operator()(T value) const
	{
		constexpr std::size_t bit_count = CHAR_BIT * sizeof(T);
		if(value == 0u)
			return bit_count;
#ifdef __GNUC__
		if constexpr(std::is_same_v<T, unsigned int>)
			return __builtin_ctz(value);
		else if constexpr(std::is_same_v<T, unsigned long>)
			return __builtin_ctzl(value);
		else if constexpr(std::is_same_v<T, unsigned long long>)
			return __builtin_ctzll(value);
#endif
		for(std::size_t i = 0; i < bit_count; ++i)
		{
			if(static_cast<bool>(high_bit & 1u))
				return i;
		}
		assert(false and "Unreachable.");
	}
};

template <class T>
struct PopulationCount {
	static_assert(std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>>);
	T operator()(T value) const
	{
		constexpr std::size_t bit_count = CHAR_BIT * sizeof(T);
		// GCC and Clang both emit the same assembly for this as a __builtin_popcount()
		return std::bitset<bit_count>(value).count();
	}
};

} /* namespace detail */

inline const auto i32_rotl = detail::decorate_signed(detail::RotateLeft<wasm_uint32_t>{});
inline const auto i64_rotl = detail::decorate_signed(detail::RotateLeft<wasm_uint64_t>{});

inline const auto i32_rotr = detail::decorate_signed(detail::RotateRight<wasm_uint32_t>{});
inline const auto i64_rotr = detail::decorate_signed(detail::RotateRight<wasm_uint64_t>{});

inline const auto i32_clz = detail::decorate_signed(detail::CountLeadingZeros<wasm_uint32_t>{});
inline const auto i64_clz = detail::decorate_signed(detail::CountLeadingZeros<wasm_uint64_t>{});

inline const auto i32_ctz = detail::decorate_signed(detail::CountTrailingZeros<wasm_uint32_t>{});
inline const auto i64_ctz = detail::decorate_signed(detail::CountTrailingZeros<wasm_uint64_t>{});

inline const auto i32_popcnt = detail::decorate_signed(detail::PopulationCount<wasm_uint32_t>{});
inline const auto i64_popcnt = detail::decorate_signed(detail::PopulationCount<wasm_uint64_t>{});

inline const auto i32_eqz = [](wasm_sint32_t x) { return static_cast<wasm_sint32_t>(x == 0); };
inline const auto i64_eqz = [](wasm_sint64_t x) { return static_cast<wasm_sint64_t>(x == 0); };

namespace detail {

template <class Int>
std::size_t get_bit_count(Int i)
{
	static_assert(std::is_integral_v<Int>);
	return CHAR_BIT * sizeof(i);
}

template <class Int>
bool sign(Int sint)
{
	static_assert(std::conjunction_v<std::is_integral<Int>, std::is_signed<Int>>);
	auto uint = to_unsigned(sint);
	return static_cast<bool>(uint >> (get_bit_count(uint) - 1u));
}

template <class Int>
inline const auto int_equal = [](Int l, Int r) -> Int {
	static_assert(std::conjunction_v<std::is_integral<Int>, std::is_signed<Int>>);
	return to_unsigned(l) == to_unsigned(r);
};

template <class Int>
inline const auto int_not_equal = [](Int l, Int r) -> Int {
	return not int_equal(l, r);
};

template <class Int>
inline const auto int_less = [](Int l, Int r) -> Int {
	static_assert(std::conjunction_v<std::is_integral<Int>, std::is_signed<Int>>);
	if constexpr(detail::system_is_twos_complement)
	{
		return l < r;
	}
	else
	{
		auto l_u = to_unsigned(l);
		auto r_u = to_unsigned(r);
		bool l_sign = sign(l_u);
		bool r_sign = sign(r_u);
		auto l_magn = l_sign ? l_u : (~l_u) + 1u;
		auto r_magn = r_sign ? r_u : (~r_u) + 1u;
		if(l_sign)
			if(r_sign)
				return l_u > r_u;
			else
				return true;
		else
			if(r_sign)
				return false;
			else
				return l_u < r_u;
	}
};

template <class Int>
inline const auto int_less_equal = [](Int l, Int r) -> Int {
	return (to_unsigned(l) == to_unsigned(r)) or int_less(l, r);
};

template <class Int>
inline const auto int_greater = [](Int l, Int r) -> Int {
	return int_less(r, l);
};

template <class Int>
inline const auto int_greater_equal = [](Int l, Int r) -> Int {
	return (to_unsigned(r) == to_unsigned(l)) or int_less(r, l);
};

template <class Comparator>
auto wrap_unsigned_comp(Comparator comp)
{
	return [=](auto left, auto right) -> wasm_sint32_t{
		static_assert(std::is_same_v<decltype(left), decltype(right)>);
		using type = decltype(left);
		static_assert(std::is_integral_v<type>);
		static_assert(std::is_signed_v<type>);
		return to_signed(static_cast<wasm_uint32_t>(
			std::invoke(comp, to_unsigned(left), to_unsigned(right))
		));
	};
}

} /* namespace detail */

inline const auto i32_eq = detail::int_equal<wasm_sint32_t>;
inline const auto i32_ne = detail::int_not_equal<wasm_sint32_t>;
inline const auto i32_lt_s = detail::int_less<wasm_sint32_t>;
inline const auto i32_le_s = detail::int_less_equal<wasm_sint32_t>;
inline const auto i32_gt_s = detail::int_greater<wasm_sint32_t>;
inline const auto i32_ge_s = detail::int_greater_equal<wasm_sint32_t>;
inline const auto i32_lt_u = detail::wrap_unsigned_comp(std::less<wasm_uint32_t>{});
inline const auto i32_le_u = detail::wrap_unsigned_comp(std::less_equal<wasm_uint32_t>{});
inline const auto i32_gt_u = detail::wrap_unsigned_comp(std::greater<wasm_uint32_t>{});
inline const auto i32_ge_u = detail::wrap_unsigned_comp(std::greater_equal<wasm_uint32_t>{});

inline const auto i64_eq = detail::int_equal<wasm_sint64_t>;
inline const auto i64_ne = detail::int_not_equal<wasm_sint64_t>;
inline const auto i64_lt_s = detail::int_less<wasm_sint64_t>;
inline const auto i64_le_s = detail::int_less_equal<wasm_sint64_t>;
inline const auto i64_gt_s = detail::int_greater<wasm_sint64_t>;
inline const auto i64_ge_s = detail::int_greater_equal<wasm_sint64_t>;
inline const auto i64_lt_u = detail::wrap_unsigned_comp(std::less<wasm_uint64_t>{});
inline const auto i64_le_u = detail::wrap_unsigned_comp(std::less_equal<wasm_uint64_t>{});
inline const auto i64_gt_u = detail::wrap_unsigned_comp(std::greater<wasm_uint64_t>{});
inline const auto i64_ge_u = detail::wrap_unsigned_comp(std::greater_equal<wasm_uint64_t>{});

/// Floating Point

inline const auto float32_add = std::plus<wasm_float32_t>;
inline const auto float64_add = std::plus<wasm_float64_t>;

inline const auto float32_sub = std::minus<wasm_float32_t>;
inline const auto float64_sub = std::minus<wasm_float64_t>;

inline const auto float32_mul = std::multiplies<wasm_float32_t>;
inline const auto float64_mul = std::multiplies<wasm_float64_t>;

inline const auto float32_div = std::divides<wasm_float32_t>;
inline const auto float64_div = std::divides<wasm_float64_t>;

inline const auto float32_sqrt = [](wasm_float32_t v) { return std::sqrt(v); };
inline const auto float64_sqrt = [](wasm_float64_t v) { return std::sqrt(v); };

inline const auto float32_min = [](wasm_float32_t l, wasm_float32_t r) { return std::min(l, r); };
inline const auto float64_min = [](wasm_float64_t l, wasm_float64_t r) { return std::min(l, r); };

inline const auto float32_max = [](wasm_float32_t l, wasm_float32_t r) { return std::max(l, r); };
inline const auto float64_max = [](wasm_float64_t l, wasm_float64_t r) { return std::max(l, r); };

inline const auto float32_ceil = [](wasm_float32_t v) { return std::ceil(v); };
inline const auto float64_ceil = [](wasm_float64_t v) { return std::ceil(v); };

inline const auto float32_floor = [](wasm_float32_t v) { return std::floor(v); };
inline const auto float64_floor = [](wasm_float64_t v) { return std::floor(v); };

namespace detail {
	struct FloatNearbyRoundingGuard {
		Guard():
			mode(getround())
		{
			if(mode != FE_TONEAREST)
				setround(mode);
		}
		~Guard() {
			if(mode != FE_TONEAREST)
				setround(mode);
		}
	private:
		static void setround(const int m)
		{
			// does this function ever actually fail?  an assert feels more
			// appropriate than throwing or letting the error pass silently.
			int err = std::fesetround(FE_TONEAREST);
			assert(err == 0 and "std::fesetround() failed unexpectedly.");
		}
		static int getround()
		{
			// does this function ever actually fail?  an assert feels more
			// appropriate than throwing or letting the error pass silently.
			int m = std::fegetround();
			assert(m >= 0 and "std::fegetround() failed unexpectedly.");
			return m;
		}
		const int mode;
	};

} /* namespace detail */

inline const auto float32_nearest = [](wasm_float32_t v) {
	FloatNearbyRoundingGuard guard;
	return std::nearbyint(v);
};
inline const auto float64_nearest = [](wasm_float64_t v) {
	FloatNearbyRoundingGuard guard;
	return std::nearbyint(v);
};

inline const auto float32_neg = [](wasm_float32_t v) {
	return std::isnan(v) ? v : (-wasm_float32_t(0)) - v;
};
inline const auto float64_neg = [](wasm_float64_t v) {
	return std::isnan(v) ? v : (-wasm_float64_t(0)) - v;
};

inline const auto float32_copysign = [](wasm_float32_t l, wasm_float64_t r) { return std::copysign(l, r); };
inline const auto float64_copysign = [](wasm_float64_t l, wasm_float64_t r) { return std::copysign(l, r); };

namespace detail {


} /* namespace detail */

inline const auto f32_eq = detail::bool_to_i32(std::equal_to<wasm_float32_t>{});
inline const auto f32_ne = detail::bool_to_i32(std::not_equal_to<wasm_float32_t>{});
inline const auto f32_lt = detail::bool_to_i32(std::less<wasm_float32_t>{});
inline const auto f32_le = detail::bool_to_i32(std::less_equal<wasm_float32_t>{});
inline const auto f32_gt = detail::bool_to_i32(std::greater<wasm_float32_t>{});
inline const auto f32_ge = detail::bool_to_i32(std::greater_equal<wasm_float32_t>{});

inline const auto f64_eq = detail::bool_to_i32(std::equal_to<wasm_float64_t>{});
inline const auto f64_ne = detail::bool_to_i32(std::not_equal_to<wasm_float64_t>{});
inline const auto f64_lt = detail::bool_to_i32(std::less<wasm_float64_t>{});
inline const auto f64_le = detail::bool_to_i32(std::less_equal<wasm_float64_t>{});
inline const auto f64_gt = detail::bool_to_i32(std::greater<wasm_float64_t>{});
inline const auto f64_ge = detail::bool_to_i32(std::greater_equal<wasm_float64_t>{});


/// Conversions

inline const auto i32_wrap_i64 = [](wasm_sint64_t value) { return truncate(value); };

inline const auto i64_extend_s_i32 = [](wasm_sint32_t value) { return sign_extend(value); };
inline const auto i64_extend_u_i32 = [](wasm_sint32_t value) {
	return to_signed(wasm_uint64_t(to_unsigned(value)));
};

namespace detail {

template <class Integral>
inline const auto truncate_checked = [](auto value) {
	static_assert(std::is_floating_point_v<decltype(value)>);
	static_assert(std::is_integral_v<Integral>);
	using float_type = decltype(value);
	static constexpr const std::size_t bit_count = CHAR_BIT * sizeof(value);
	if(std::isnan(value) or std::isinf(value))
		throw BadTruncation();
	if constexpr(std::is_signed_v<Integral>)
	{
		const float_type minm = -std::exp2(float_type(bit_count - 1)) - 1;
		const float_type maxm = std::exp2(float_type(bit_count - 1));
		if(value > minm and value < maxm)
		{
			std::make_unsigned_t<Integral> abs_val = std::fabs(truncated);
			assert(static_cast<float_type>(abs_val) == truncated);
			if(std::signbit(truncated))
				abs_val = (~abs_val) + 1u;
			return reinterpret_cast<const Integral&>(abs_val);
		}
		throw BadTruncation();
	}
	else
	{
		const float_type minm = -1;
		const float_type maxm = std::exp2(float_type(bit_count));
		if(truncated > minm and truncated < maxm)
		{
			auto result = static_cast<Integral>(truncated);
			assert(static_cast<float_type>(result) == truncated);
			return result;
		}
		throw BadTruncation();
	}
};

} /* namespace detail */

inline const auto i32_trunc_s_f32 = [](wasm_float32_t value) {
	return detail::truncate_checked<wasm_sint32_t>(value); 
};
inline const auto i32_trunc_s_f64 = [](wasm_float64_t value) {
	return detail::truncate_checked<wasm_sint32_t>(value); 
};
inline const auto i64_trunc_s_f32 = [](wasm_float32_t value) {
	return detail::truncate_checked<wasm_sint64_t>(value); 
};
inline const auto i64_trunc_s_f64 = [](wasm_float64_t value) {
	return detail::truncate_checked<wasm_sint64_t>(value); 
};

inline const auto i32_trunc_u_f32 = [](wasm_float32_t value) {
	return to_signed(detail::truncate_checked<wasm_uint32_t>(value)); 
};
inline const auto i32_trunc_u_f64 = [](wasm_float64_t value) {
	return to_signed(detail::truncate_checked<wasm_uint32_t>(value)); 
};
inline const auto i64_trunc_u_f32 = [](wasm_float32_t value) {
	return to_signed(detail::truncate_checked<wasm_uint64_t>(value)); 
};
inline const auto i64_trunc_u_f64 = [](wasm_float64_t value) {
	return to_signed(detail::truncate_checked<wasm_uint64_t>(value)); 
};

inline const auto f32_demote_f64 = [](wasm_float32_t value) {
	return static_cast<wasm_float64_t>(value); 
};
inline const auto f64_promote_f32 = [](wasm_float64_t value) {
	return static_cast<wasm_float32_t>(value); 
};

inline const auto f32_convert_s_i32 = [](wasm_sint32_t value) {
	constexpr auto sign_bit = wasm_uint32_t(1u) << 31u;
	auto uvalue = to_unsigned(value);
	neg = static_cast<bool>(uvalue & sign_bit);
	wasm_float32_t mag = neg ? (~uvalue) + 1u : uvalue;
	wasm_float32_t sign = neg ? -1.0 : 1.0;
	return std::copysign(mag, sign);
};
inline const auto f32_convert_s_i64 = [](wasm_sint64_t value) {
	constexpr auto sign_bit = wasm_uint64_t(1u) << 63u;
	auto uvalue = to_unsigned(value);
	neg = static_cast<bool>(uvalue & sign_bit);
	wasm_float32_t mag = neg ? (~uvalue) + 1u : uvalue;
	wasm_float32_t sign = neg ? -1.0 : 1.0;
	return std::copysign(mag, sign);
};
inline const auto f64_convert_s_i32 = [](wasm_sint32_t value) {
	constexpr auto sign_bit = wasm_uint32_t(1u) << 31u;
	auto uvalue = to_unsigned(value);
	neg = static_cast<bool>(uvalue & sign_bit);
	wasm_float64_t mag = neg ? (~uvalue) + 1u : uvalue;
	wasm_float64_t sign = neg ? -1.0 : 1.0;
	return std::copysign(mag, sign);
};
inline const auto f64_convert_s_i64 = [](wasm_sint64_t value) {
	constexpr auto sign_bit = wasm_uint64_t(1u) << 63u;
	auto uvalue = to_unsigned(value);
	neg = static_cast<bool>(uvalue & sign_bit);
	wasm_float64_t mag = neg ? (~uvalue) + 1u : uvalue;
	wasm_float64_t sign = neg ? -1.0 : 1.0;
	return std::copysign(mag, sign);
};

inline const auto f32_convert_u_i32 = [](wasm_sint32_t value) {
	return static_cast<wasm_float32_t>(to_unsigned(value));
};
inline const auto f32_convert_u_i64 = [](wasm_sint64_t value) {
	return static_cast<wasm_float32_t>(to_unsigned(value));
};
inline const auto f64_convert_u_i32 = [](wasm_sint32_t value) {
	return static_cast<wasm_float64_t>(to_unsigned(value));
};
inline const auto f64_convert_u_i64 = [](wasm_sint64_t value) {
	return static_cast<wasm_float64_t>(to_unsigned(value));
};

inline const auto i32_reinterpret_u_f32 = [](wasm_float32_t value) {
	wasm_sint32_t result;
	assert(sizeof(result) == sizeof(value));
	std::memcpy(&result, &value, sizeof(result));
	return result;
};
inline const auto i64_reinterpret_u_f64 = [](wasm_float64_t value) {
	wasm_sint64_t result;
	assert(sizeof(result) == sizeof(value));
	std::memcpy(&result, &value, sizeof(result));
	return result;
};
inline const auto f32_reinterpret_u_i32 = [](wasm_sint32_t value) {
	wasm_float32_t result;
	assert(sizeof(result) == sizeof(value));
	std::memcpy(&result, &value, sizeof(result));
	return result;
};
inline const auto f64_reinterpret_u_i64 = [](wasm_sint64_t value) {
	wasm_float64_t result;
	assert(sizeof(result) == sizeof(value));
	std::memcpy(&result, &value, sizeof(result));
	return result;
};


} /* namespace wasm::ops::arith */

#define /* VM_ARITH_H */
