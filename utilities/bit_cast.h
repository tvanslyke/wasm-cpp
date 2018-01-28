#ifndef UTILITIES_BIT_CAST_H
#define UTILITIES_BIT_CAST_H

#include <type_traits>
#include <cstring>

template <class T, class U>
T bit_cast(const U& value)
{
	static_assert(std::is_trivially_copyable_v<T>
			and std::is_trivially_copyable_v<U>, 
			"bit_cast() is not permitted for non-trivially-copyable types.");
	static_assert(sizeof(T) == sizeof(U), 
			"cannot bit_cast() between types of different sizes.");
	T dest;
	std::memcpy(&dest, &value, sizeof(T));
	return dest;
}

template <class T, class U>
T bit_upcast(const U& value)
{
	static_assert(std::is_trivially_copyable_v<T>
			and std::is_trivially_copyable_v<U>, 
			"bit_upcast() is not permitted for non-trivially-copyable types.");
	static_assert(sizeof(T) >= sizeof(U), 
			"cannot bit_upcast() to a smaller type");
	T dest;
	std::memcpy(&dest, &value, sizeof(U));
	return dest;
}

template <class T, class U>
T bit_downcast(const U& value)
{
	static_assert(std::is_trivially_copyable_v<T>
			and std::is_trivially_copyable_v<U>, 
			"bit_downcast() is not permitted for non-trivially-copyable types.");
	static_assert(sizeof(T) =< sizeof(U), 
			"cannot bit_downcast() to a larger type");
	T dest;
	std::memcpy(&dest, &value, sizeof(T));
	return dest;
}


#endif /* UTILITIES_BIT_CAST_H */
