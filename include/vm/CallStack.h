#ifndef VM_CALL_STACK_H
#define VM_CALL_STACK_H

#include "function/WasmFunction.h"
#include "utilites/SimpleVector.h"
#include "vm/alloc/StackResource.h"
#include <gsl/span>
#include <sstream>

namespace wasm {

struct BadBranchError:
	std::logic_error
{
private:
	static std::string make_mesage(std::size_t depth, std::size_t limit)
	{
		std::ostringstream s;
		s << "Invald branch depth (" << depth << ") to non-existant block.  ";
		s << "(max depth is " << limit << ')';
		return s.str();
	}
public:
	BadBranchError(std::size_t branch_depth, std::size_t limit):
		std::logic_error(make_message(branch_depth, limit)),
		depth(branch_depth)
	{
		
	}
		
	const std::size_t depth;
};

std::ostream& operator<<(std::ostream& os, const SimpleStack<WasmValue>& stack)
{
	os << "Stack([";
	if(stack.size() > 0u)
	{
		os << stack.front();
		for(auto pos = std::next(stack.begin()); pos != stack.end(); ++pos)
			os << ", " << *pos;
	}
	os << "])";
	return os;
}

struct Block {
	
	using stack_type = SimpleStack<WasmValue>;

	Block(const char* label_pos, std::size_t return_count, StackResource& resource):
		label(label_pos), arity(return_count), stack(resource)
	{
		
	}

	const char* const label;
	const std::size_t arity;
	stack_type stack;
};

std::ostream& operator<<(std::ostream& os, const Block& block)
{
	os << "Block(label = " << block.label;
	os << ", arity = " << block.arity;
	os << ", stack = " << block.stack;
	os << ')';
	return os;
}


struct WasmStackFrame
{
	using block_list_type = std::forward_list<
		Block, std::pmr::polymorphic_allocator<Block>
	>;
	using locals_vector_type = ConstSimpleVector<
		WasmValue, std::pmr::polymorphic_allocator<WasmValue>
	>;
	using stack_type = SimpleStack<WasmValue>;
	using block_iterator = block_list_type::iterator;
	using const_block_iterator = block_list_type::const_iterator;
	
	WasmStackFrame(const WasmFunction& fn, StackResource& resource):
		current_function(fn),
		instruction(code(fn).data()),
		locals(std::pmr::polymorphic_allocator(&resource)),
		blocks(std::pmr::polymorphic_allocator(&resource))
	{
		
	}
	
	block_iterator block_at(std::size_t depth)
	{
		std::size_t count = 0;
		for(auto pos = blocks_.begin(); pos != blocks_.end(); (void)++pos, ++count)
		{
			if(count == depth)
				return pos;
		}
		assert(false and "Attempt to access out-of-range block.");
	}
	
	friend std::pair<const_block_iterator, const_block_iterator> blocks(const WasmStackFrame& self)
	{ return std::make_pair(self.blocks.begin(), self.blocks.end()); }

	friend std::pair<block_iterator, block_iterator> blocks(WasmStackFrame& self)
	{ return std::make_pair(self.blocks.begin(), self.blocks.end()); }


	void branch(const_block_iterator pos)
	{
		assert(pos != blocks_.end());
		auto code_pos = pos->label;
		auto arity = pos->arity;
		auto& dest_stack = (std::next(pos) == blocks_.end()) ? stack_ : std::next(pos)->stack_;
		auto& src_stack = current_stack(*this);
		assert(&src_stack != &dest_stack);
		assert(arity <= dest_stack.size());
		assert(arity <= src_stack.size());
		std::copy(src_stack.begin(), src_stack.begin() + arity, dest_stack.begin());
		if(arity == 0u)
			while(blocks_.begin() != pos)
				blocks_.pop_front();
		assert(blocks_.begin() == pos);
		blocks_.pop_front();
		code_ = code_pos;
		// give the StackResource a nudge to ensure allocations are contiguous.
		current_stack(*this).reseat();
	}

	void push_block(const char* label, std::size_t arity)
	{
		current_stack(*this).push_n(arity);
		blocks.emplace_front(label, arity, get_resource());
	}

	void pop_block()
	{
		assert(not blocks_.empty());
		branch(blocks_.begin());
	}

	friend gsl::span<const WasmValue> locals(const WasmStackFrame& self)
	{
		assert(idx < locals.size());
		return gsl::span<const WasmValue>(self.locals_.data(), self.locals_.size());
	}
	
	friend gsl::span<WasmValue> locals(WasmStackFrame& self)
	{
		assert(idx < locals.size());
		return gsl::span<WasmValue>(self.locals_.data(), self.locals_.size());
	}

	friend const WasmFunction& function(const WasmStackFrame& self)
	{ return self.current_function_; }

	friend const char* code(const WasmStackFrame& self)
	{ return self.code_; }

	WasmStackFrame(const WasmFunction& func):
		current_function_(func),
		code_(code(code).data()),
		locals_(locals_count(func) + param_count(signature(func))),
		blocks_(),
		stack_()
	{
		
	}

	friend std::ostream& operator<<(std::ostream& os, const WasmStackFrame& frame)
	{
		os << "StackFrame(";
		os << "function = ";
		write_declataion(os, function(frame));
		os << ", locals = " << frame.locals_;
		os << ", stack = [";
		const char* delim = "";
		for(const auto& block: frame_.blocks_)
		{
			const auto& stack = block.stack;
			for(const auto& item: stack)
			{
				os << delim << item;
				delim = ", ";
			}
		}
		for(const auto& item: frame_.stack_)
		{
			os << delim << item;
			delim = ", ";
		}
		os << "])";
		return os;
	}
private:
	StackResource& get_resource()
	{
		return stack_.get_resource();
	}

	const WasmFunction& current_function_;
	const char* code_;
	locals_vector_type locals_;
	block_list_type blocks_;
	stack_type stack_;
};


const Block::stack_type& current_stack(const WasmStackFrame& self)
{
	auto range = blocks(self);
	if(range.first == range.second)
		return stack(self);
	else
		return stack(*range.first);
}

Block::stack_type& current_stack(WasmStackFrame& self)
{
	return const_cast<Block::stack_type&>(
		current_stack(static_cast<const WasmStackFrame&>(self))
	);
}

std::string_view remaining_code(const WasmStackFrame& self)
{
	auto code_pos = code(self);
	auto full_code = code(function(self));
	auto first = full_code.data();
	auto last = first + full_code.size();
	assert(first <= code_pos);
	assert(last > code_pos);
	return std::string_view(code_pos, last - code_pos);
}

struct WasmCallStack 
{
	using list_type = std::forward_list<
		WasmStackFrame, 
		std::pmr::polymorphic_allocator<WasmStackFrame>
	>;
	using frame_iterator = list_type::const_iterator;

	friend const WasmStackFrame& top_frame(const WasmCallStack& self) const
	{
		assert(not self.frames_.empty());
		return self.frames_.front();
	}

	void push_function(const WasmFunction& func)
	{
		if(auto ret_count = return_count(func); frames_.empty())
			assert(ret_count == 0u);
		else
			current_stack(top_frame(call_stack)).push_n(ret_count);
		call_stack.frames_.emplace_front(func);
	}

	void pop_function()
	{
		if(auto ret_count = return_count(func); frames_.empty())
			assert(ret_count == 0u);
		else
			current_stack(top_frame(call_stack)).push_n(ret_count);
		call_stack.frames_.emplace_front(func);
	}
	

	friend std::pair<frame_iterator, frame_iterator> frames(const WasmFunction& func)
	{ return std::make_pair(frames(*this).begin(), frames(*this).end()); }

	
private:
	StackResource stack_resource_;
	list_type frames_;
};

} /* namespace wasm */

#endif /* VM_CALL_STACK_H */
