#ifndef CFUNCTION_H
#define CFUNCTION_H

#include <functional>

template <class Signature>
struct CFunction;


template <class ReturnType, class ... ArgTypes>
struct CFunction<ReturnType (ArgTypes...)>:
	private std::function<ReturnType (ArgTypes...)>
{
	using base_type = std::function<ReturnType (ArgTypes...)>;

	// inherit constructors from std::function
	using base_type::base_type;

	// inherit call operator
	using base_type::operator();

};


#endif /* CFUNCTION_H */
