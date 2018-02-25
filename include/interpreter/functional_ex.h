#ifndef FUNCTIONAL_EX_H
#define FUNCTIONAL_EX_H


#include <functional>
#include <type_traits>
#include <sstream>
#include <cmath>
#include "bitutils.h"
#include "utilities/bit_cast.h"

struct trap_error: public std::runtime_error {
	template <class String>
	trap_error(const String& string):
		std::runtime_error(string)
	{
		
	}
	trap_error(opcode_t op):
		std::runtime_error(opcode_message(op))
	{
		
	}
	
	static std::string opcode_message(opcode_t op)
	{
		std::stringstream ss;
		ss << "Trap occurred while evaluating instruction " << std::hex << op << ".";
		return ss.str();
	}
};

template <class T = void>
struct bit_lshift
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		static_assert(std::is_unsigned_v<T>, "Don't pass signed values to bit_lshift::operator() plz.");
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		return lhs << (rhs % bits); 
	}
};
template <>
struct bit_lshift<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_lshift<T>{}(lhs, rhs); }
};

template <class T = void>
struct bit_rshift
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		using UnsignedT = std::make_unsigned_t<T>;
		constexpr std::size_t bits = std::numeric_limits<UnsignedT>::digits;
		const auto shift = rhs % bits;
		if constexpr(std::is_unsigned_v<T>)
			return lhs >> shift; 
		else
		{
			UnsignedT lhs_u = bit_cast<UnsignedT>(lhs);
			bool msb = (UnsignedT(1) << (bits - 1)) & lhs_u;
			lhs_u >>= shift;
			if(msb)
			{
				// sign extension
				UnsignedT mask = ~UnsignedT(0);
				mask <<= (bits - shift);
				lhs_u |= mask;
			}
			return bit_cast<T>(lhs_u);
		}
	}
};

template <>
struct bit_rshift<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_rshift<T>{}(lhs, rhs); }
};

template <class T = void>
struct bit_rrotate
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		static_assert(std::is_unsigned_v<T>);
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		const auto shift = rhs % bits;
		return (lhs >> shift) | (lhs << (bits - shift));
	}
};

template <>
struct bit_rrotate<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_rrotate<T>{}(lhs, rhs); }
};

template <class T = void>
struct bit_lrotate
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		static_assert(std::is_unsigned_v<T>);
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		const auto shift = rhs % bits;
		return (lhs << shift) | (lhs >> (bits - shift));
	}
};

template <>
struct bit_lrotate<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_lrotate<T>{}(lhs, rhs); }
};

template <class T = void>
struct bit_clz
{
	decltype(auto) operator()(const T& val) const
	{
		return count_leading_zeros(val);
	}
};

template <>
struct bit_clz<void>
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_clz<T>{}(val); }
};

template <class T = void>
struct bit_ctz
{
	decltype(auto) operator()(const T& val) const
	{ return count_trailing_zeros(val); }
};

template <>
struct bit_ctz<void>
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_ctz<T>{}(val); }
};

template <class T = void>
struct bit_popcnt
{
	decltype(auto) operator()(const T& val) const
	{ return population_count(val); }
};

template <>
struct bit_popcnt<void>
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_popcnt<T>{}(val); }
};

namespace detail {
	template <class T>
	inline constexpr const char* wasm_type_name_str_v;

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_float32_t> = "f32";

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_float64_t> = "f32";

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_sint32_t> = "i32";

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_sint64_t> = "i32";

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_uint32_t> = "u32";

	template <>
	inline constexpr const char* wasm_type_name_str_v<wasm_uint64_t> = "u32";

} /* namespace detail */

template <class T = void>
struct wasm_divide
{
	decltype(auto) operator()(const T& left, const T& right) const
	{
		if constexpr (std::is_integral_v<T> and std::is_signed_v<T>)
		{
			constexpr T maxm = std::numeric_limits<T>::max();
			if((left == maxm) and (right == -1))
			{
				std::stringstream message;
				message << "Trap after attempt to divide "
					<< detail::wasm_type_name_str_v<T>
					<< " maximum "
					<< maxm
					<< " by -1.";
				throw trap_error(message.str());
			}
		}
		if(right == 0)
		{
			std::stringstream message;
			message << "Trap after attempt to divide "
				<< detail::wasm_type_name_str_v<T>
				<< " ("
				<< left
				<< ") by 0.";
			throw trap_error(message.str());
		}
		
		return std::divides<>{}(left, right);
	}
};

template <>
struct wasm_divide<void>
{
	template <class T>
	decltype(auto) operator()(const T& left, const T& right) const
	{ return wasm_divide<T>{}(left, right); }
};


template <class T = void>
struct wasm_modulus
{
	decltype(auto) operator()(const T& left, const T& right) const
	{
		if(right == 0)
		{
			std::stringstream message;
			message << "Trap after attempt to compute the modulus of "
				<< detail::wasm_type_name_str_v<T>
				<< " ("
				<< left
				<< ") with base 0.";
			throw trap_error(message.str());
		}
		if constexpr(std::is_unsigned_v<T>)
			return std::modulus<>{}(left, right);
		else if((right != -1) or (left != std::numeric_limits<T>::min()))
			return std::modulus<>{}(left, right);
		else 
			return static_cast<T>(0);
	}
};

template <>
struct wasm_modulus<void>
{
	template <class T>
	decltype(auto) operator()(const T& left, const T& right) const
	{ return wasm_modulus<T>{}(left, right); }
};

template <class Integer, class Float = void>
struct wasm_trunc
{
	decltype(auto) operator()(const Float& value) const
	{
		constexpr Float int_maxm = std::numeric_limits<Integer>::max();
		constexpr Float int_minm = std::numeric_limits<Integer>::min();

		if(value > int_maxm or value < int_minm)
		{
			std::stringstream message;
			message << "Trap after attempt to truncate "
				<< detail::wasm_type_name_str_v<Float>
				<< " with value "
				<< value
				<< " to type "
				<< detail::wasm_type_name_str_v<Integer>
				<< ". (domain error)";
			throw trap_error(message.str());
		}
		
		return std::trunc(value);
	}
};

template <class Integer>
struct wasm_trunc<Integer, void>
{
	template <class Float>
	decltype(auto) operator()(const Float& value) const
	{ return wasm_trunc<Integer, Float>{}(value); }
};

#endif /* FUNCTIONAL_EX_H */
