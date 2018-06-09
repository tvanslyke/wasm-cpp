#ifndef UTILITIES_SCOPE_GUARD_H
#define UTILITIES_SCOPE_GUARD_H
#include <exception>
#include <functional>
#include <type_traits>

template <class OnError>
struct ScopeGuard 
{
	template <class ... Args>
	ScopeGuard(Args&& ... args) noexcept(std::is_nothrow_constructible_v<OnError, Args...>):
		on_error_(std::forward<Args>(args) ...),
		active_exceptions_at_construction_(std::uncaught_exceptions())
	{
	    
	}

	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard(ScopeGuard&&) = delete;

	~ScopeGuard()
	{
		if(exceptions_thrown())
			call_on_error();
	}

private:
	void call_on_error() noexcept(std::is_nothrow_invocable_v<OnError>)
	{ std::invoke(on_error_); }

	bool exceptions_thrown() const noexcept
	{ return std::uncaught_exceptions() > active_exceptions_at_construction_; }

	// Invoked if this ScopeGuard is destroyed due to the stack being unwound 
	// from a thrown exception.
	OnError on_error_;
	// Number of uncaught exceptions at the time of construction.
	const int active_exceptions_at_construction_;
};

template <class OnError>
ScopeGuard<std::decay_t<OnError>> make_scope_guard(OnError&& on_error)
	noexcept(std::is_nothrow_constructible_v<ScopeGuard<std::decay_t<OnError>>, OnError>)
{
	return ScopeGuard<std::decay_t<OnError>>(std::forward<OnError>(on_error));
}


#endif /* UTILITIES_SCOPE_GUARD_H */
