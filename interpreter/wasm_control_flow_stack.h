#ifndef INTERPRETER_WASM_CONTROL_FLOW_STACK_H
#define INTERPRETER_WASM_CONTROL_FLOW_STACK_H

#include "wasm_base.h"

class wasm_control_flow_stack 
{
	struct frame_t;
	using label_t = const wasm_opcode::wasm_opcode_t*;
	using index_t = std::vector<frame_t>::difference_type;
	struct frame_t 
	{
		frame_t(wasm_value_t* sp):
			stack_pointer(sp)
		{
			
		}

		frame_t(wasm_value_t* sp, label_t lbl):
			stack_pointer(sp), label(lbl)
		{
			
		}

		frame_t(wasm_value_t* sp, label_t lbl, std::size_t ret_count = 0):
			stack_pointer(sp), label(lbl), arity(ret_count)
		{
			
		}

		frame_t() = delete;

		bool is_ret_frame() const
		{
			return (label == nullptr);
		}

		wasm_value_t* stack_pointer;
		std::size_t arity;
		label_t label = nullptr;
		static const frame_t sentinel;
	};
public:
	wasm_control_flow_stack(std::size_t max_size):
		frames(max_size, frame_t::sentinel), pos(0)
	{
		
	}

	void push_frame(wasm_value_t* sp, label_t lbl, std::size_t arity)
	{ 
		frames.at(pos++) = frame_t(sp, lbl, arity);
	}
	
	std::tuple<wasm_value_t*, label_t, std::size_t> pop_frame()
	{
		auto frame = pop_top();
		assert(pos >= 0);
		return {frame.stack_pointer, frame.label, frame.arity};
	}

	void push_function(wasm_value_t* sp, std::size_t arity)
	{
		push_frame(sp, nullptr, arity); 
	}

	std::tuple<wasm_value_t*, label_t, std::size_t> pop_function()
	{
		while(not pop_top().is_ret_frame())
		{
			/* LOOP */
		}
		return pop_frame();
	}
	
	std::tuple<wasm_value_t*, label_t, std::size_t> jump_index(index_t index)
	{
		frame_t frame = top(index);
		std::fill(frames.begin() + stack_index(index), frames.begin() + stack_index(pos), frame_t::sentinel);
		pos = stack_index(index);
		return {frame.stack_pointer, frame.label, frame.arity};
	}
	
	std::tuple<wasm_value_t*, label_t, std::size_t> jump_top()
	{
		frame_t frame = pop_top();
		return {frame.stack_pointer, frame.label, frame.arity};
	}
	
private:

	
	index_t stack_index(index_t idx) const
	{
		assert(idx < pos);
		return (pos - idx) - 1;
	}

	const frame_t& top(index_t idx = 0) const
	{
		return frames.at(stack_index(idx));
	}
	
	frame_t& top(index_t idx = 0)
	{
		return frames.at(stack_index(idx));
	}
	
	frame_t pop_top()
	{
		frame_t top_frame = top();
		--pos;
		return top_frame;
	}

	std::vector<frame_t> frames;
	index_t pos;
};

const wasm_control_flow_stack::frame_t 
wasm_control_flow_stack::frame_t::sentinel(nullptr, nullptr, 0);
#endif /* INTERPRETER_WASM_CONTROL_FLOW_STACK_H */
