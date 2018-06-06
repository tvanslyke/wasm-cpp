#ifndef VM_CALL_STACK_H
#define VM_CALL_STACK_H

#include "function/WasmFunction.h"
#include "utilites/SimpleVector.h"
#include "utilites/ListStack.h"
#include "vm/alloc/StackResource.h"
#include <gsl/span>
#include <sstream>

namespace wasm {

namespace ex {

template <class Container, class Index>
decltype(auto) at(Container&& c, const Index& index)
{
	using std::size;
	if(index < size(std::forward<Container>(c)))
		return std::forward<Container>(c)[index];
	throw std::out_of_range("Out-of-bounds container access in wasm::ex::at().");
}

} /* namespace ex */

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

std::ostream& operator<<(std::ostream& os, const SimpleStack<TaggedWasmValue>& stack)
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

template <class T>
struct Block {
	
	using stack_type = SimpleStack<T>;

	Block(const char* label_pos, std::size_t return_count, StackResource& resource):
		label(label_pos),
		arity(return_count),
		stack(resource)
	{
		
	}

	const char* const label;
	const std::size_t arity;
	stack_type stack;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Block<T>& block)
{
	os << "Block(label = " << block.label;
	os << ", arity = " << block.arity;
	os << ", stack = " << block.stack;
	os << ')';
	return os;
}


template <class T>
struct WasmStackFrame
{
	using value_type = T;
	using block_type = Block<value_type>;
	using block_list_type = std::forward_list<
		block_type, std::pmr::polymorphic_allocator<block_type>
	>;
	using locals_vector_type = gsl::span<value_type>;
	using stack_type = SimpleStack<value_type>;
	using block_iterator = block_list_type::iterator;
	using const_block_iterator = block_list_type::const_iterator;
private:
	
public:
	WasmStackFrame(const WasmFunction& fn, CodeView ret_addr, locals_vector_type locals, StackResource& resource):
		current_function_(fn),
		return_address_(ret_addr),
		locals_(locals),
		blocks_(std::pmr::polymorphic_allocator(&resource))
	{
		assert(locals_.size() == locals_count(function()) + param_count(function()));
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

	const char* branch_depth(wasm_uint32_t depth)
	{ return branch(block_at(depth)); }

	const char* branch(const_block_iterator pos)
	{
		assert(pos != blocks_.end());
		// get the label
		auto code_pos = pos->label;
		auto arity = pos->arity;
		auto& dest_stack = (std::next(pos) == blocks_.end()) ? stack_ : std::next(pos)->stack_;
		auto& src_stack = current_stack(*this);
		assert(&src_stack != &dest_stack);
		assert(arity <= dest_stack.size());
		assert(arity <= src_stack.size());
		std::copy(src_stack.begin(), src_stack.begin() + arity, dest_stack.begin());
		// TODO: is this correct?: assert(src_stack.size() == arity)
		while(blocks_.begin() != pos)
			blocks_.pop_front();
		assert(blocks_.begin() == pos);
		blocks_.pop_front();
		// give the StackResource a nudge to ensure allocations are contiguous.
		current_stack(*this).reseat();
	}

	void push_block(const char* label, gsl::span<const LanguageType> signature)
	{
		assert(
			(label[-1] == static_cast<char>(OpCode::END))
			or (label[-(1 + (std::ptrdiff_t)sizeof(wasm_uint32_t))] == static_cast<char>(OpCode::ELSE))
		);
		for(const auto& type: signature)
		{
			// push return values onto the stack
			visit_value_type(
				[&](auto WasmValue::* p) { stack_emplace_top(p, 0); }, type
			);
		}
		blocks_.emplace_front(label, signature.size(), get_resource());
	}

	void pop_block()
	{
		assert(not blocks_.empty());
		branch(blocks_.begin());
	}

	T& local_at(wasm_uint32_t index)
	{ return ex::at(locals_, index); }

	const T& local_at(wasm_uint32_t index) const
	{ return ex::at(locals_, index); }

	T& stack_at(wasm_uint32_t index)
	{ return ex::at(stack(), index); }

	const T& stack_at(wasm_uint32_t index)
	{ return ex::at(stack(), index); }

	template <class U>
	T& stack_at(wasm_uint32_t index, U WasmValue::* p)
	{ return get(ex::at(stack(), index), p); }

	const T& stack_at(wasm_uint32_t index)
	{ return ex::at(stack(), index); }

	T& stack_top()
	{ return stack().top(); }

	const T& stack_top() const
	{ return stack().top(); }

	template <class U>
	const T& stack_top(U WasmValue::* p) const
	{ return get(stack_top(), p); }

	template <class U>
	T& stack_top(U WasmValue::* p)
	{ return get(stack_top(), p); }

	std::pair<const T&, const T&> stack_top_2() const
	{ return std::pair<const T&, const T&>(stack_at(1u), stack_at(0u)); }

	std::pair<T&, T&> stack_top_2() const
	{ return std::pair<T&, T&>(stack_at(1u), stack_at(0u)); }

	template <class L, class R>
	std::pair<const L&, const R&> stack_top_2(L WasmValue::* l, R WasmValue::* r) const
	{
		return std::pair<const L&, const R&>(
			get(stack_at(1u), l), get(stack_at(0u), r)
		); 
	}

	template <class U>
	void stack_pop_2_push_1(U WasmValue::* mem, U value)
	{
		stack_pop();
		stack_pop_1_push_1(mem, value);
	}

	template <class U>
	void stack_pop_1_push_1(U WasmValue::* mem, U value)
	{ stack().replace_top(mem, value); }

	T stack_pop()
	{ return stack().pop(); }

	template <class ... Args>
	T stack_emplace_top(Args&& ... args)
	{ return stack().emplace(std::forward<Args>(args)...); }

	void stack_pop_n(std::size_t n)
	{ stack().pop_n(n); }

	auto stack_size() const
	{ return stack().size(); }

	const WasmFunction* function()
	{ return self.current_function_; }

	CodeView code() const
	{ return self.code_; }

	WasmStackFrame(const WasmFunction& func):
		current_function_(&func),
		code_(func),
		locals_(locals_count(func) + param_count(signature(func))),
		blocks_(),
		stack_()
	{
		blocks_.emplace_front(nullptr, 0u, get_resource());
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

	const auto& stack() const
	{
		assert(not blocks_.empty());
		return blocks_.front().stack;
	}

	auto& stack()
	{
		assert(not blocks_.empty());
		return blocks_.front().stack;
	}

	
	StackResource& get_resource()
	{
		std::memory_resource* rsc = blocks_.get_allocator().resource();
		assert(rsc);
		assert(static_cast<bool>(dynamic_cast<StackResource*>(rsc)));
		return *static_cast<StackResource*>(rsc);
	}


	WasmStackFrame(
		const WasmFunction& func,
		const char* ret_addr,
		gsl::span<T> locals
	):
		current_function_(&func),
		return_address_(ret_addr),
		locals_(locals),
		blocks_()
	{
		if(func)
		{
			// stack frame for a function
			assert(locals_.size() == param_count(*func) + locals_count(*func));
		}
		else
		{
			// stack frame for code not bound to a function
			assert(not return_address_); 
		}
	}

	// pointer to the current function, if this stack frame originates from a 
	// function call
	const WasmFunction* const current_function_;
	const char* const return_address_;
	const gsl::span<T> locals_;
	block_list_type blocks_;
};


template <class T>
const typename WasmStackFrame<T>::stack_type& current_stack(const WasmStackFrame<T>& self)
{
	auto range = blocks(self);
	if(range.first == range.second)
		return stack(self);
	else
		return stack(*range.first);
}

template <class T>
typename WasmStackFrame<T>::stack_type& current_stack(WasmStackFrame<T>& self)
{
	return const_cast<Block::stack_type&>(
		current_stack(static_cast<const WasmStackFrame&>(self))
	);
}

template <class T>
std::string_view remaining_code(const WasmStackFrame<T>& self)
{
	auto code_pos = code(self);
	auto full_code = code(function(self));
	auto first = full_code.data();
	auto last = first + full_code.size();
	assert(first <= code_pos);
	assert(last > code_pos);
	return std::string_view(code_pos, last - code_pos);
}

template <class T>
struct WasmCallStack 
{
	using stack_type = ListStack<
		WasmStackFrame<T>, 
		std::pmr::polymorphic_allocator<WasmStackFrame<T>>
	>;
	using frame_iterator = list_type::const_iterator;

	const WasmStackFrame<T>& current_frame() const
	{
		assert(not frames_.empty());
		return frames_.top();
	}

	CodeView push_frame(const WasmFunction& func, const char* return_address)
	{
		auto& stack = frames_.empty() ? frames_.top().stack() : base_stack_;
		auto func_sig = signature(func);
		auto arg_types = param_types(func_sig);
		auto locals_types = locals(func);
		auto ret_count = return_count(func);
		if(stack.size() < arg_types.size())
			assert(false); // TODO: throw an exception.
		// push zero-initialized locals onto the stack
		for(LanguageType type: locals_types)
		{
			visit_value_type(
				[&](auto WasmValue::* p) { stack.emplace(p, 0); }, type
			);
		}
		auto locals_vector_size = locals_types.size() + arg_types.size();
		auto locals_pos = stack.data() + (stack.size() - locals_vector_size);
		gsl::span<T> locals_vector(locals_pos, locals_vector_size);
		frames_.emplace(func, return_address, locals_vector);
		return CodeView(func);
	}

	[[nodiscard]]
	CodeView pop_frame()
	{
		assert(not frames_.empty());
		auto ret_types = return_types(signature(src_func));
		assert(src_frame.stack_size() >= ret_types.size());
		const char* ret_addr = top_frame().return_address();
		auto code_view = code(src_func);
	}

private:
	WasmStackFrame<T>& top_frame()
	{
		if(frames_.empty())
			throw std::out_of_range("Attempt to access out-of-bounds frame.");
		return frames_.top();
	}

	std::pair<WasmStackFrame<T>&, WasmStackFrame<T>&> top_two_frames()
	{
		if(frames_.empty())
			throw std::out_of_range("Attempt to access out-of-bounds frame.");
		auto pos = frames_.begin();
		const auto& hi = *pos++;
		if(pos == frames_.end())
			throw std::out_of_range("Attempt to access out-of-bounds frame.");
		const auto& lo = *pos++;
		return std::pair<WasmStackFrame&, WasmStackFrame&>(lo, hi);
	}

	void _recurse_return_from_frame(std::size_t return_count)
	{
		assert(not frames_.empty());
		assert(std::next(frames_.begin()) != frames_.end());
		auto& top_frame = frames_.front();
		if(return_count > 0u)
		{
			// one of the return values from the callee's frame 
			auto ret_v = top_frame.stack_pop();
			// keep recursing into this member function until we exhaust the return
			// types.  Once we've done that, pop the callee's frame, and then pop the locals
			// vector and arguments off the caller's stack.
			_recurse_return_from_frame(return_count - 1u);
			// now that we've popped the callee's frame and then the locals vector off of 
			// the caller's frame's stack, push the return values onto the callee's 
			// frame's stack.
			frames_.front().stack_emplace_top(ret_v);
		}
		else
		{
			const auto* func_p = top_frame.function();
			assert(func_p);
			const auto& func = *func_p;
			std::size_t total_locals = param_count(func) + locals_count(func);
			// pop the top frame
			frames_.pop_front();
			assert(not frames_.empty());
			auto& new_top_frame = frames_.front();
			assert(new_top_frame.stack_size() >= total_locals);
			// pop the locals (args + locals) vector of the frame we just popped
			// off of the stack.  note that the locals vector of the current frame
			// lives on the stack of the previous frame.
			new_top_frame.stack_pop_n(total_locals);
			// push the return values of the frame we just popped... 
			// ... by simply returning from _recurse_return_from_frame().  each
			// recursive call which took the if() branch above will push the return
			// values onto the new frame. 
		}
	}

	const StackResource& stack_resource() const
	{ return base_stack_.get_resource(); }

	StackResource& stack_resource()
	{ return base_stack_.get_resource(); }

	stack_type frames_;
	SimpleStack base_stack_;
};

template <class T>
const T& local_at(const WasmCallStack<T>& self, std::size_t idx)
{
	auto locals = locals(current_frame(self));
	if(idx >= locals.size())
		throw ValidationError<std::out_of_range>("Attempt to access out-of-bounds local.");
	return locals[idx];
}

template <class T>
T& local_at(WasmCallStack<T>& self, std::size_t idx)
{
	auto locals = locals(current_frame(self));
	if(idx >= locals.size())
		throw ValidationError<std::out_of_range>("Attempt to access out-of-bounds local.");
	return locals[idx];
}

template <class T>
const auto& current_stack(const WasmCallStack<T>& self, std::size_t idx)
{ return current_stack(current_frame(self)); }

template <class T>
auto& current_stack(WasmCallStack<T>& self, std::size_t idx)
{ return current_stack(current_frame(self)); }


} /* namespace wasm */

#endif /* VM_CALL_STACK_H */
