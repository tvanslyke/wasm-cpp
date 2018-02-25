#include "interpreter/WasmProgramStack.h"

struct FrameInfo {
	FrameInfo(wasm_value_t* fp):
		frame_pointer_(fp)
	{
		assert(frame_pointer_);
	}

	wasm_value_t* previous_frame() const
	{
		auto offset = frame_pointer_->i64;
		assert(offset >= 0);
		return frame_pointer_ + offset;
	}

	FrameInfo previous_frame_info() const
	{ return FrameInfo(previous_frame()); }
	
	wasm_uint32_t previous_locals_count() const
	{ return previous_frame()[1].u32; }
	
	wasm_value_t* previous_locals_begin() const
	{ return previous_frame() + header_size; }

	wasm_value_t* previous_locals_end() const
	{ return previous_locals_begin() + previous_locals_count(); }

	wasm_value_t* locals_begin() const
	{ return frame_pointer_ + header_size; }
	
	wasm_uint32_t previous_stack_pointer() const
	{
		auto* prev = previous_frame();
		auto offset = prev[2].i64;
		assert(offset >= 0);
		return prev + offset;
	}

	wasm_uint32_t return_count() const
	{ return frame_pointer_[3].u32;	}

	wasm_uint64_t function_index() const
	{ return frame_pointer_[4].u32;	}

	static constexpr const std::ptrdiff_t header_size = 5;
private:	
	wasm_value_t* const frame_pointer_;
};

public:

void WasmProgramStack::push_value(wasm_value_t value)
{
	assert(stack_pointer_ < stack_limit_);
	*stack_pointer_++ = value;
}

wasm_value_t WasmProgramStack::pop_value()
{
	assert(stack_pointer_);
	assert(stack_pointer > locals_end_);
	return *(--stack_pointer_);
}

wasm_value_t WasmProgramStack::get_local(wasm_uint32_t index)
{
	assert(locals_begin_ < locals_end_);
	assert(std::ptrdiff_t(index) < (locals_end_ - locals_begin_));
	wasm_value_t value = locals_[index];
	push_value(value);
	return value;
}

wasm_value_t WasmProgramStack::set_local(wasm_uint32_t index, wasm_value_t value)
{
	assert(locals_begin_ < locals_end_);
	assert(std::ptrdiff_t(index) < (locals_end_ - locals_begin_));
	wasm_value_t value = pop_value();
	locals_[index] = value;
	return value;
}

wasm_value_t WasmProgramStack::tee_local(wasm_uint32_t index, wasm_value_t value)
{
	assert(locals_begin_ < locals_end_);
	assert(std::ptrdiff_t(index) < (locals_end_ - locals_begin_));
	wasm_value_t value = pop_value();
	locals_[index] = value;
	push_value(value);
	return value;
}

void WasmProgramStack::push_frame(const wasm_function& function, std::size_t function_index)
{
	auto param_count = function.param_count();
	auto locals_count = function.locals_count();
	auto total_locals = param_count + locals_count;
	auto return_count = function.return_count();

	// arguments to the function we're pushing on to the stack
	wasm_value_t* args = stack_pointer_ - param_count;

	// the new frame will live at the beginning of these arguments
	auto frame_pos = args;
	
	// check for stack overflow
	auto frame_size = new_frame_size(function);
	auto size_available = stack_limit_ - frame_pos;
	assert(size_available >= frame_size);

	FrameInfo new_frame_info(new_frame_pos);
	// copy the arguments to the new frame's locals vector
	std::memmove(new_frame_info.locals_begin(), args, param_count * sizeof(*args));
	// zero-out the non-argument locals
	std::memset(args + param_count, 0, locals_count * sizeof(*args));
	// initialize the frame header now that we've moved the arguments out of the way
	write_new_frame_header(frame_pos, return_count, function_index);
	
	// finally, set our data members to proper values for the new frame
	auto new_stack_pointer = new_frame_info.locals_begin() + total_locals;
	assert(new_stack_pointer == frame_pos + new_frame_size);
	assert(new_stack_pointer <= stack_limit_);
	frame_pointer_ = frame_pos;
	locals_begin_ = new_frame_info.locals_begin();
	locals_end_ = locals_begin_ + total_locals;
	stack_pointer_ = new_stack_pointer;
}

void WasmProgramStack::pop_frame()
{
	FrameInfo info(frame_pointer_);
	auto return_count = std::ptrdiff_t(info.return_count());
	wasm_value_t* returns_begin = stack_pointer_ - wasm_int64_t(return_count);
	restore_previous_frame();
	assert(remaining_count() >= return_count);
	std::memmove(stack_pointer_, returns_begin, return_count * sizeof(*stack_pointer_));
	stack_pointer_ += return_count;
}

std::ptrdiff_t WasmProgramStack::current_stack_size() const
{
	return stack_pointer_ - stack_bottom_;
}

void WasmProgramStack::jump(std::ptrdiff_t WasmProgramStack::stack_length) 
{
	assert(stack_length < capacity());
	auto pos = stack_bottom_ + stack_length;
	assert(pos <= stack_pointer_);
	assert(pos >= locals_end_);
	stack_pointer_ = stack_length; 
}

std::size_t WasmProgramStack::function_index() const
{
	return current_frame_info().function_index();
}

private:
std::size_t WasmProgramStack::new_frame_size(const wasm_function& function)
{
	return FrameInfo::header_size + function.locals_count() + function.param_count();
}

void WasmProgramStack::restore_previous_frame()
{
	FrameInfo info(current_frame_info());
	auto* prev_frame_pointer = info.previous_frame();
	locals_begin_ = info.previous_locals_begin();
	locals_end_ = info.previous_locals_end();
	stack_pointer_ = info.previous_stack_pointer();
	frame_pointer_ = info.previous_frame();
	assert(frame_pointer_ < locals_begin_);
	assert(locals_begin_ <= locals_end_);
	assert(locals_end_ <= stack_pointer_);
}

void WasmProgramStack::write_new_frame_header(wasm_value_t* frame_pos, wasm_uint32_t return_count, std::size_t function_index)
{
	assert(frame_pos > frame_pointer_);
	assert(stack_pointer_ > frame_pointer_);
	frame_pos[0].i64 = frame_pos - frame_pointer_;
	frame_pos[1].u32 = locals_end_ - locals_begin_;
	frame_pos[2].i64 = stack_pointer_ - frame_pointer_;
	frame_pos[3].u32 = return_count;
	frame_pos[4].u64 = function_index;
}

wasm_value_t* WasmProgramStack::frame_stackbottom() 
{ return locals_end_; }

wasm_value_t* WasmProgramStack::frame_stacktop() 
{ return stack_pointer_; }

void WasmProgramStack::assert_invariants() const
{
	assert(frame_pointer_ < locals_begin_);		
	assert(locals_begin_ <= locals_end_);		
	assert(locals_end_ <= stack_pointer_);
	FrameInfo info(current_frame_info());
	assert(info.previous_frame() == nullptr
		or (info.previous_frame() < frame_pointer_));
}

FrameInfo WasmProgramStack::current_frame_info() const
{
	return FrameInfo(frame_pointer_);
}

FrameInfo WasmProgramStack::previous_frame_info() const
{
	return current_frame_info().previous_frame_info();
}

std::ptrdiff_t WasmProgramStack::remaining_count() const
{
	return stack_limit_ - stack_pointer_;
}

std::ptrdiff_t WasmProgramStack::capacity() const
{
	return stack_limit_ - stack_bottom_;
}

void WasmProgramStack::save_current_frame(wasm_uint32_t param_count, wasm_uint32_t return_count) const
{
	wasm_value_t* save_location = stack_pointer_ - param_count;
	wasm_value_t* args_begin = 
	std::ptrdiff_t header_size = /* TODO */;
	std::memmove(save_location, 
}

struct WasmControlFlowStack
{
	WasmControlFlowStack(wasm_value_t* first, wasm_value_t* last):
		first_(first), last_(last), pos_(first)
	{
		assert(last_ >= first_);
		assert(((last_ - first_) % 2) == 0);
	}

	const opcode_t* push_block(const opcode_t* program_counter, wasm_value_t* stack_pointer)
	{
		auto [arity, label, code] = read_block(program_counter);
		enter_block(label, stack_pointer, arity)
		return code;
	}

	const opcode_t* replace_block(const opcode_t* program_counter, wasm_value_t* stack_pointer)
	{
		// used in the 'else' op
		auto [arity, _label, code] = read_block(program_counter, false);
		std::tie(std::ignore, std::ignore, arity) = pop_block();
		enter_block(label, stack_pointer, arity);
		return code;
	}

	std::tuple<const wasm_opcode_t*, wasm_value_t*, wasm_uint32_t> pop_block()
	{
		wasm_value_t* entry = exit_block();
		return {
			static_cast<const wasm_opcode_t*>(entry[0]._const_ptr), // label
			static_cast<wasm_value_t*>(entry[1]._ptr), // stack pointer
			entry[2].u32 // arity
		};
	}

	wasm_value_t* save_to_frame_header(wasm_value_t* dest)
	{
		(*dest++)._ptr = first_;
		(*dest++)._ptr = last_;
		(*dest++)._ptr = pos_;
		return dest;
	}

	wasm_value_t* restore_to_frame_header(wasm_value_t* dest)
	{
		first_ = static_cast<wasm_value_t*>((*dest++)._ptr);
		last_ = static_cast<wasm_value_t*>((*dest++)._ptr);
		pos_ = static_cast<wasm_value_t*>((*dest++)._ptr);
		return dest;
	}

	std::ptrdiff_t size() const
	{
		auto sz = (pos_ - first_) / 2;
		assert(sz >= 0);
		return sz;
	}

	std::ptrdiff_t capacity() const
	{
		auto cap = (pos_ - first_) / 2;
		assert(cap >= 0);
		return cap;
	}

	private:
	static std::tuple<wasm_uint32_t, const wasm_opcode_t*, const wasm_opcode_t*> 
	read_block(const opcode_t* code, bool read_block_type = true)
	{
		signed char block_type;
		wasm_sint32_t offset;
		if(read_block_type)
			program_counter = read_raw_immediate(program_counter, &block_type);
		else 
			block_type = -0x40;
		auto program_counter_next = read_raw_immediate(program_counter, &offset);
		// return the arity and destination label of this block, after inspecting the immediates
		// additionally, return the pointer to opcode following these immediates
		return {block_arity(block_type), program_counter + offset, program_counter_next};
	}

	void enter_block(const opcode_t* label, wasm_value_t* stack_pointer, wasm_uint32_t arity)
	{
		assert(pos_ < last_);
		auto entry = pos_;
		pos_ += entry_size;
		entry[0]._const_ptr = label;
		entry[1]._ptr = stack_pointer;
		entry[2].u32 = arity;
		assert(pos_ <= last_);
		return entry;
	}

	wasm_value_t* exit_block()
	{
		assert(pos_ > first_);
		auto entry = pos_;
		pos_ -= entry_size;
		assert(pos_ >= first_);
		return entry;
	}

	static wasm_uint32_t block_arity(signed char block_type) 
	{
		switch(block_type)
		{
		case -0x01: /* i32 */ [[fallthrough]]
		case -0x02: /* i64 */ [[fallthrough]]
		case -0x03: /* f32 */ [[fallthrough]]
		case -0x04: /* f64 */
			return 1;
			break;
		case -0x40: /* void */
			return 0;
			break;
		default:
			throw std::runtime_error("Bad 'block_type' encountered.");
			break;
		}
	}

	wasm_value_t* first_;
	wasm_value_t* last_;
	wasm_value_t* pos_;
	static constexpr const std::ptrdiff_t entry_size = 3;
};


#endif /* WASM_PROGRAM_STACK_H */
