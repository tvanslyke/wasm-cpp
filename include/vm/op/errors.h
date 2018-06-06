#ifndef VM_OP_ERRORS_H
#define VM_OP_ERRORS_H

#include <stdexcept>

namespace wasm::ops {

struct TrapError:
	public std::logic_error 
{
	using std::logic_error::logic_error;
};

struct DivisionByZeroError:
	public TrapError
{
	DivisionByZeroError():
		TrapError("Trap: attemped division by zero.")
	{
		
	}

};

struct OverflowError:
	public TrapError
{
	using TrapError::TrapError;
};

struct UnreachableError:
	public TrapError
{
	using TrapError::TrapError;
};

struct BadTruncation:
	public TrapError
{
	DivisionByZeroError():
		TrapError("Trap: out-of-range value in float-to-integer truncation")
	{
		
	}
};

} /* namespace wasm::ops */

#endif /* VM_OP_ERRORS_H */
