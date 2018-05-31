#ifndef WASM_PROGRAM_STACK_H
#define WASM_PROGRAM_STACK_H
#include "wasm_base.h"
#include "wasm_value.h"
#include "wasm_instruction.h"
#include "function/WasmFunction.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>

struct WasmProgramStackBase
{
	using opcode_t = wasm_opcode::wasm_opcode_t;

	const WasmFunction& current_function() const
	{
		return *function_;
	}
	// total number of 'wasm_value_t' instances occupied on the stack	
	std::ptrdiff_t size() const
	{
		assert(stack_pointer_ >= stack_bottom_);
		return stack_pointer_ - stack_bottom_; 
	}
	
	// total number of 'wasm_value_t' instances allocated on the stack
	std::ptrdiff_t capacity() const
	{ 
		assert(stack_limit_ > stack_bottom_);
		return stack_limit_ - stack_bottom_;
	}

	// total number of 'wasm_value_t' instances allocated in the locals vector (including arguments)
	std::ptrdiff_t locals_size() const
	{ 
		assert(locals_end_ >= locals_begin_);
		return locals_end_  - locals_begin_;
	}

	// total number of 'wasm_value_t' instances allocated on the stack after the locals vector
	std::ptrdiff_t nonlocals_size() const
	{
		assert(stack_pointer_ >= locals_end_);
		return stack_pointer_  - locals_end_;
	}

	std::ptrdiff_t return_count() const
	{ return function_->return_count(); }

	std::ptrdiff_t frame_count() const
	{ return frame_count_; }
	
	std::ptrdiff_t param_count() const
	{ return function_->param_count(); }
	
	std::ptrdiff_t local_variables_count() const
	{ return function_->locals_count(); }
	
	std::ptrdiff_t locals_vector_size() const
	{ return locals_end_ - locals_begin_; }
	
	const wasm_value_t* args_begin() const
	{ return locals_begin_; }

	const wasm_value_t* args_end() const
	{ return locals_begin_ + param_count(); }
	
	const wasm_value_t* locals_vector_begin() const
	{ return locals_begin_; }

	const wasm_value_t* locals_vector_end() const
	{ return locals_end_; }
	
	const opcode_t* program_counter() const
	{ return program_counter_; }


	bool is_in_block() const
	{
		return label_;
	}

	std::string debug_frame() const
	{
		std::stringstream str;
		auto escape = [](void* p) {
			asm volatile("" : : "g"(p) : "memory");
		};
		auto wasm_value_str = [&](wasm_value_t value) -> std::string {
			std::stringstream repr;
			escape(&value);
			repr << std::hex << std::right << std::setw(20) << value.u32;
			escape(&value);
			repr << std::hex << std::right << std::setw(20) << value.u64;
			escape(&value);
			repr << std::right << std::setw(20) << value.f32;
			escape(&value);
			repr << std::right << std::setw(20) << value.f64;
			return repr.str();
		};
		str << "Call frame for " << function_->name() << ':' << '\n';
		str << "While executing instruction 0x" << std::hex << int(*(program_counter())) << '\n';
		if(program_counter() > function_->code())
			str << "(previous instruction: 0x" << std::hex << int(program_counter()[-1]) << ")\n";
		str << '\t';
		str << std::right << std::setw(20) << "i32";
		str << std::right << std::setw(20) << "i64";
		str << std::right << std::setw(20) << "f32";
		str << std::right << std::setw(20) << "f64";
		str << '\n';
		str << '\t' << "Stack:" << '\n';
		auto stack_ptr = std::make_reverse_iterator(stack_pointer_);
		std::ptrdiff_t block_depth = 0;
		for(auto label = label_; label; label = static_cast<wasm_value_t*>(label[0]._ptr))
			++block_depth;
		for(auto label = label_; label; label = static_cast<wasm_value_t*>(label[0]._ptr))
		{
			for(auto stop = std::make_reverse_iterator(label + 3); stack_ptr < stop; ++stack_ptr)
			{
				str << '\t';
				for(std::ptrdiff_t i = 0; i < block_depth; ++i)
					str << ' ';
				str << wasm_value_str(*stack_ptr) << '\n';
			}
			stack_ptr += 3;
		}
		for(auto stop = std::make_reverse_iterator(locals_end_); stack_ptr < stop; ++stack_ptr)
				str << '\t' << wasm_value_str(*stack_ptr) << '\n';
		str << '\t' << "Local Variables:" << '\n';
		for(auto sp = std::make_reverse_iterator(locals_end_); sp < std::make_reverse_iterator(args_end()); ++sp)
			str << '\t' << wasm_value_str(*sp) << '\n';
		str << '\t' << "Local Variables (function arguments):" << '\n';
		for(auto sp = std::make_reverse_iterator(args_end()); sp < std::make_reverse_iterator(args_begin()); ++sp)
			str << '\t' << wasm_value_str(*sp) << '\n';
		return str.str();
	}
	const wasm_value_t* const_stack_pointer() const
	{
		return stack_pointer_;
	}
protected:
	WasmProgramStackBase(wasm_value_t* stack_begin, wasm_value_t* stack_end):
		stack_bottom_(stack_begin),
		stack_limit_(stack_end),
		frame_count_(0),
		function_(nullptr),
		frame_pointer_(nullptr),
		locals_begin_(stack_begin),
		locals_end_(stack_begin),
		label_(nullptr),
		stack_pointer_(stack_begin),
		program_counter_(nullptr)
	{

	}

	void call_function(const WasmFunction& func)
	{
		auto [args_begin, args_end] = arg_vector(func.param_count());
		// the header of the frame we're currently pushing on to the stack 
		// is going to live in the spot where the arguments start.  first we'll 
		// have to move the arguments out of the way
		wasm_value_t* frame_begin = args_begin;
		std::ptrdiff_t total_locals = func.param_count() + func.locals_count();
		auto locals_offset = frame_header_size; 
		std::ptrdiff_t total_size = frame_header_size + total_locals;
		assert(capacity() - size() >= total_size);
		// Copy the arguments in the new frame's locals vector.
		{
			wasm_value_t* args_dest = frame_begin + locals_offset;
			std::copy_backward(args_begin, args_end, args_dest + func.param_count());
		}
		// Now that the argument vector is no longer in the way, save the current frame state 
		// in the space we just carved out on the stack.
		// 'pos' points to the first element not in the frame header
		stack_pointer_ = frame_begin;
		wasm_value_t* pos = save_frame_state(frame_begin, stack_limit_);
		
		// finally, initialize the new frame 
		assert(pos - frame_begin == locals_offset);
		function_ = &func;
		frame_pointer_ = frame_begin;
		locals_begin_ = frame_begin + locals_offset;
		locals_end_ = frame_begin + total_size;
		label_ = nullptr;
		stack_pointer_ = locals_end_;
		program_counter_ = func.code();
		assert_invariants();
		assert(/* TODO: sanity check */ "sanity check failed after pushing new frame on to call stack");
		// initialize non-argument locals to zero
		std::fill(locals_begin_ + func.param_count(), locals_end_, zero_wasm_value());
		++frame_count_;
	}
	
	void return_from_function()
	{
		// get a pointer to the values to return 
		auto top = stack_pointer_;
		auto returns_begin = top - function_->return_count();
		assert(function_->return_count() < 2);
		restore_frame_state(frame_pointer_, stack_limit_);
		// move the return values to the top-of-stack of the previous frame
		stack_pointer_ = std::copy(returns_begin, top, stack_pointer_);
		assert(frame_count_ > 0);
		--frame_count_;
	}

	// pop a value off of the value stack
	wasm_value_t pop_value() 
	{
		assert(stack_pointer_ > locals_end_);
		return *(--stack_pointer_);
	}
	
	// push a value on to the value stack
	void push_value(wasm_value_t value) 
	{
		assert(stack_pointer_ < stack_limit_);
		*stack_pointer_++ = value;
	}

	// set the program counter to the given position
	void code_jump(const opcode_t* pos)
	{
		assert(pos >= code_begin());
		assert(pos < code_end());
		program_counter_ = pos;
	}

	// return the pointer to the first opcode in the current function
	const opcode_t* code_begin() const
	{ return function_->code(); }

	// return the the past-the-end pointer of the current function's code
	const opcode_t* code_end() const
	{ return function_->code() + function_->code_size(); }

	wasm_value_t* stack_pointer() 
	{ return stack_pointer_; }
	
	void branch(std::ptrdiff_t index)
	{
		auto lbl = get_label(index);
		branch_label(lbl);
	}

	// push a new block with a label on to the control-flow stack	
	void push_label(const opcode_t* code_pos, wasm_uint32_t arity)
	{
		assert(capacity() - size() >= 3);
		assert_valid_jump(code_pos);
		auto new_label = stack_pointer_;
		// push pointer to preceding label
		(*stack_pointer_++)._ptr = label_;
		// push the destination of the label
		(*stack_pointer_++)._const_ptr = code_pos;
		// push the arity of the block
		(*stack_pointer_++).u32 = arity;
		label_ = new_label;
	}

	void pop_label()
	{
		assert(label_);
		auto prev_label = static_cast<wasm_value_t*>(label_[0]._ptr);
		assert(prev_label < label_);
		label_ = prev_label;
	}

	// replace the top entry in the control-flow stack with a new label.
	// used for the 'else' op
	void replace_label(const opcode_t* code_pos)
	{
		assert_valid_jump(code_pos);
		auto old_code_pos = static_cast<const opcode_t*>(label_[1]._const_ptr);
		assert(old_code_pos < code_pos);
		label_[1]._const_ptr = code_pos;
	}

	template <class T>
	T read_immediate()
	{
		T value;
		assert(code_end() - program_counter_ > std::ptrdiff_t(sizeof(value)));
		std::memcpy(&value, program_counter_, std::ptrdiff_t(sizeof(value)));
		program_counter_ += sizeof(value);
		return value;
	}
	

	static wasm_uint32_t block_arity(signed char tp)
	{
		switch(tp)
		{
		case -0x01: [[fallthrough]]
		case -0x02: [[fallthrough]]
		case -0x03: [[fallthrough]]
		case -0x04:
			return 1;
			break;
		case -0x40:
			return 0;
			break;
		default:
			throw std::runtime_error("Bad 'block_type' encountered.");
		}
	}
	
	wasm_value_t get_local(wasm_uint32_t idx) const
	{
		assert(idx < locals_vector_size());
		return locals_begin_[idx]; 
	}
	
	void set_local(wasm_uint32_t idx, wasm_value_t value) const
	{
		assert(idx < locals_vector_size());
		locals_begin_[idx] = value; 
	}
	
	opcode_t current_instruction() const
	{
		assert(program_counter_ < code_end());
		return *program_counter_;
	}
	
	void advance_instruction() 
	{
		assert(program_counter_ < function_->code() + function_->code_size());
		++program_counter_;
	}
private:
	void assert_invariants()
	{}
	wasm_value_t* save_frame_state(wasm_value_t* dest_begin, const wasm_value_t* dest_end)
	{
		assert(dest_end - dest_begin >= frame_header_size);
		auto pos = dest_begin;
		(*pos++)._const_ptr = function_;
		(*pos++)._ptr = frame_pointer_;
		(*pos++)._ptr = locals_begin_;
		(*pos++)._ptr = label_;
		(*pos++)._ptr = stack_pointer_;
		(*pos++)._const_ptr = program_counter_;
		assert(pos <= dest_end);
		assert(pos - dest_begin == frame_header_size);
		return pos;
	}
	
	void restore_frame_state(wasm_value_t* begin, const wasm_value_t* end)
	{
		assert(end - begin >= frame_header_size);
		auto pos = begin;
		function_ = static_cast<const WasmFunction*>((*pos++)._const_ptr);
		frame_pointer_ = static_cast<wasm_value_t*>((*pos++)._ptr);
		locals_begin_ = static_cast<wasm_value_t*>((*pos++)._ptr);
		if(function_)
			locals_end_ = locals_begin_ + function_->locals_count() + function_->param_count();
		else
			locals_end_ = locals_begin_;
		label_ = static_cast<wasm_value_t*>((*pos++)._ptr);
		stack_pointer_ = static_cast<wasm_value_t*>((*pos++)._ptr);
		program_counter_ = static_cast<const opcode_t*>((*pos++)._const_ptr);
		assert(pos <= end);
		assert(pos - begin == frame_header_size);
	}

	std::array<wasm_value_t*, 2> arg_vector(wasm_uint32_t param_count)
	{
		assert(nonlocals_size() >= param_count);
		return std::array<wasm_value_t*, 2>{stack_pointer_ - param_count, stack_pointer_};
	}


	
	wasm_value_t* get_label(std::ptrdiff_t count) const
	{
		wasm_value_t* label = label_;
		while(count-- > 0)
		{
			assert(label);
			label = static_cast<wasm_value_t*>(label[0]._ptr);
		}
		return label;
	}
	
	void branch_label(wasm_value_t* label)
	{
		assert(label);
		assert(stack_pointer_ > label);
		label_ = static_cast<wasm_value_t*>(label[0]._ptr);
		auto code_ptr = static_cast<const opcode_t*>(label[1]._const_ptr);
		assert_valid_jump(code_ptr);
		auto arity = label[2].u32;
		assert((stack_pointer_ - (label + 3)) == arity);
		// reseat the program counter
		program_counter_ = code_ptr;
		auto ret_begin = stack_pointer_ - arity;
		auto ret_end = stack_pointer_;
		// return the values from block 
		stack_pointer_ = label;
		stack_pointer_ = std::copy(ret_begin, ret_end, stack_pointer_);
	}

	void assert_valid_jump(const opcode_t* pos) const
	{
		assert(code_begin() <= pos);
		assert(pos < code_end());
	}

	// pointer to the first value in the stack
	const wasm_value_t* const stack_bottom_;
	// pointer to the past-the-end value 
	const wasm_value_t* const stack_limit_;
	// total number of stack frames 
	std::ptrdiff_t frame_count_{0};


	/* all members below this line MUST be saved in the stack space before pushing a new frame */
	// pointer to the beginning of the current frame.
	// 
	// Each frame has a 'header' that is created when it is pushed on to the stack.  
	// It would be nice to be able to access this information as though it were a struct living 
	// directly in the stack space, but C++ doesn't allow variable length structs neither C nor
	// C++ provide any guarantees about struct padding and alignment and yada yada yada.  
	//
	// The point is, we have to do a less-than-ideal ceremony whenever we push or pop a stack frame.
	// It could be prettier in theory, but it isn't.
	//
	// Anyways, each frame header looks like this:
	// 	[0] wasm_value_t._ptr = pointer to the function called to create the previous frame 
	// 	[1] wasm_value_t._ptr = pointer to the header of the previous frame's previous frame
	// 	[2] wasm_value_t._ptr = pointer to the beginning of the locals vector of the previous frame
	// 	[3] wasm_value_t._ptr = label pointer of the previous frame
	// 	[4] wasm_value_t._ptr = stack pointer of the previous frame
	// 	[5] wasm_value_t._ptr = program counter of the previous_frame
	
	// function being executed in the current frame
	const WasmFunction* function_;
	// pointer to the saved-state of the previous frame
	wasm_value_t* frame_pointer_;
	// pointer to the first value in the current locals vector
	wasm_value_t* locals_begin_;
	// pointer to the first value not in the current locals vector
	wasm_value_t* locals_end_;
	// pointer to the label that was most-recently pushed on to the stack
	wasm_value_t* label_ = nullptr;
	// pointer to the current top-of-stack
	wasm_value_t* stack_pointer_;
	// program counter
	const opcode_t* program_counter_;
	static constexpr const std::ptrdiff_t frame_header_size = 6;
};






struct WasmProgramStack:
	public WasmProgramStackBase
{
	using opcode_t = WasmProgramStackBase::opcode_t;
	using WasmProgramStackBase::WasmProgramStackBase;
	WasmProgramStack(wasm_value_t* begin, wasm_value_t* end):
		WasmProgramStackBase(begin, end)
	{
		
	}
	const opcode_t* read_label_immediate()
	{
		auto pos = program_counter();
		pos += read_immediate<wasm_uint32_t>();
		assert(pos < code_end());
		return pos;
	}

	void block_op(signed char block_type, const opcode_t* label_pos)
	{
		push_label(label_pos, block_arity(block_type));
	}
	
	void else_op(const opcode_t* label_pos)
	{
		// replace the top label with a pointer to this block's 'end' position
		replace_label(label_pos);
		// jump to the label we just pushed
		branch(0);
	}


	void if_op(signed char block_type, const opcode_t* label_pos)
	{
		block_op(block_type, label_pos);
	}

	void loop_op()
	{
		push_label(program_counter(), 0); 
	}

	void br_op(wasm_uint32_t depth)
	{
		branch(depth);
	}

	void br_if_op(wasm_uint32_t depth)
	{
		auto top = pop_value().u32;
		if(top)
			br_op(depth);
	}

	bool end_op()
	{
		if(is_in_block())
		{
			pop_label();
			return true;
		}
		else
		{
			return_op();
			return bool(program_counter());
		}
	}

	void return_op()
	{
		return_from_function();
	}

	void call_op(const WasmFunction& func)
	{
		call_function(func);
	}

	void const_op(wasm_value_t value)
	{
		push_value(value);
	}

	void get_local_op(wasm_uint32_t idx)
	{
		push_value(get_local(idx));
	}

	void set_local_op(wasm_uint32_t idx)
	{
		set_local(idx, pop_value());
	}

	void tee_local_op(wasm_uint32_t idx)
	{
		auto value = pop_value();
		push_value(value);
		set_local(idx, value);
	}

	template <class T>
	T immediate_array_at(wasm_uint32_t idx)
	{
		T value;
		auto mem = program_counter();
		mem += idx * sizeof(value);
		std::memcpy(&value, mem, sizeof(value));
		return value;
	}

	opcode_t get_opcode() const
	{
		return current_instruction();
	}
	
	opcode_t next_opcode() 
	{
		auto instr = current_instruction();
		advance_instruction();
		return instr;
	}
	
	using WasmProgramStackBase::read_immediate;
	using WasmProgramStackBase::push_value;
	using WasmProgramStackBase::pop_value;
};


#endif /* WASM_PROGRAM_STACK_H */
