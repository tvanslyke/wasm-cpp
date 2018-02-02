#ifndef MODULE_WASM_RESIZABLE_LIMITS_H
#define MODULE_WASM_RESIZABLE_LIMITS_H

#include <cstddef>
#include <optional>

struct wasm_resizable_limits
{
	wasm_resizable_limits() = delete;
	wasm_resizable_limits(wasm_uint32_t init):
		initial(init)
	{
		
	}

	wasm_resizable_limits(wasm_uint32_t init, wasm_uint32_t maxm):
		initial(init), maximum(maxm)
	{
		
	}
	
	
	const wasm_uint32_t initial;
	const std::optional<wasm_uint32_t> maximum;
};

bool operator==(const wasm_resizable_limits& left, 
		const wasm_resizable_limits& right) const 
{
	return (left.initial == right.initial) and
		(left.maximum == right.maximum);
}

bool operator!=(const wasm_resizable_limits& left, 
		const wasm_resizable_limits& right) const 
{
	return not (left == right);
}


#endif /* MODULE_WASM_RESIZABLE_LIMITS_H */
