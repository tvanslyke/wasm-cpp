#include "wasm_instruction.h"
#include <cstddef>


template <class T>
constexpr std::size_t opcode_effective_size_of()
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	constexpr std::size_t size_err = sizeof(T) % sizeof(opcode_t);
	if constexpr(size_err != 0)
		return sizeof(T) + size_err;
	else	
		return sizeof(T);
}

template <class T>
constexpr std::size_t opcode_width_of()
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	constexpr std::size_t eff_size = opcode_effective_size_of<T>();
	static_assert((eff_size % sizeof(opcode_t)) == 0, "If you see this error message, there's a bug and you should report it!");
	return eff_size / sizeof(opcode_t);
}

template <class T>
constexpr std::size_t opcode_width_of(const T& value)
{
	return opcode_width_of<T>();
}

template <class T>
T opcode_extract_immediate(const wasm_opcode::wasm_opcode_t* code)
{
	static_assert(std::is_trivially_copyable_v<T>);
	T value;
	std::memcpy(&value, code, sizeof(T));
	return value;
}

template <class T>
const wasm_opcode::wasm_opcode_t* opcode_skip_immediates(const wasm_opcode::wasm_opcode_t* code, std::size_t count = 1)
{
	return code + (opcode_width_of<T>() * count);
}


template <class T>
[[nodiscard]]
const wasm_opcode::wasm_opcode_t* 
read_raw_immediate(const wasm_opcode::wasm_opcode_t* code, T* dest)
{
	std::memcpy(dest, code, sizeof(T));
	return code + sizeof(T);
}




