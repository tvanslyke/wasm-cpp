#ifndef VM_CALL_STACK_H
#define VM_CALL_STACK_H

#include "function/WasmFunction.h"
#include "utilities/SimpleVector.h"
#include "utilities/ListStack.h"
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

namespace detail {
auto make_stack_size_guard(auto& stack)
{
	auto initial_stack_size = stack.size();
	return make_scope_guard([&stack, initial_stack_size]() {
		assert(stack.size() >= initial_stack_size);
		if(stack.size() > initial_stack_size)
			stack.pop_n(stack.size() - initial_stack_size);
	});
}
} /* namespace detail */ 

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

	bool is_bottom() const
	{
		if(not label)
		{
			assert(not arity);
			return true;
		}
		return false;
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
		block_type, pmr::polymorphic_allocator<block_type>
	>;
	using locals_vector_type = gsl::span<value_type>;
	using stack_type = SimpleStack<value_type>;
	using block_iterator = block_list_type::iterator;
	using const_block_iterator = block_list_type::const_iterator;
	
	WasmStackFrame(const WasmFunction& func, StackResource& r):
		code_(func),
		locals_(locals_count(func) + param_count(signature(func))),
		blocks_(r)
	{
		blocks_.emplace_front(nullptr, 0u, get_resource());
	}

	WasmStackFrame(CodeView code, gsl::span<T> locals, StackResource& r):
		return_address_(),
		locals_(locals),
		blocks_(r)
	{
		blocks_.push_front(nullptr, 0u, get_resource());
	}

	~WasmStackFrame()
	{
		while(not blocks_.empty())
			blocks_.pop_front();
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
	
	const char* branch_depth(wasm_uint32_t depth)
	{ return branch(block_at(depth)); }

	const char* try_branch_top() const
	{
		assert(blocks_.begin() != blocks.end())
		if(blocks_.front().is_bottom())
			return nullptr;
		else
			return branch_top();
	}

	[[nodiscard]]
	const char* branch(const_block_iterator pos)
	{
		assert(pos != blocks_.end());
		// get the label
		auto code_pos = pos->label;
		auto arity = pos->arity;
		auto& dest_stack = pos->stack;
		auto& src_stack = current_stack(*this);
		assert(&src_stack != &dest_stack);
		assert(arity <= dest_stack.size());
		assert(arity <= src_stack.size());
		std::copy(src_stack.begin(), src_stack.begin() + arity, dest_stack.begin());
		// TODO: assert(src_stack.size() == arity) for 'IF ... END' blocks
		while(blocks_.begin() != pos)
			blocks_.pop_front();
		return code_pos;
	}

	void push_block(const char* label, gsl::span<const LanguageType> signature)
	{
		assert(
			(label[-1] == static_cast<char>(OpCode::END))
			or (label[-(1 + (std::ptrdiff_t)sizeof(wasm_uint32_t))] == static_cast<char>(OpCode::ELSE))
		);
		auto guard_ = make_stack_size_guard(stack());
		for(const auto& type: signature)
		{
			// push return values onto the stack
			tp::visit_value_type(
				[&](auto WasmValue::* p) { stack_emplace_top(p, 0); }, type
			);
		}
		blocks_.emplace_front(label, signature.size(), get_resource());
	}

	[[nodiscard]]
	const char* branch_top()
	{
		assert(not blocks_.empty());
		assert(not blocks_.front().is_bottom());
		return branch(blocks_.begin());
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
	const std::decay_t<U>& stack_at(wasm_uint32_t index, U WasmValue::* p) const
	{ return stack().at().get(p); }

	template <class U>
	const U& stack_at(wasm_uint32_t index, const U WasmValue::* p) const
	{ return stack().at(index).get(p); }

	template <class U>
	U& stack_at(wasm_uint32_t index, U WasmValue::* p)
	{ return stack().at(index).get(p); }

	T& stack_top()
	{ return stack().at(0u); }

	const T& stack_top() const
	{ return stack().at(0u); }

	template <class U>
	const std::decay_t<U>& stack_top(U WasmValue::* p) const
	{ return stack_top().get(p); }

	template <class U>
	const U& stack_top(const U WasmValue::* p) const
	{ return stack_top().get(p); }

	template <class U>
	U& stack_top(U WasmValue::* p)
	{ return stack_top().get(p); }

	std::pair<const T&, const T&> stack_top_2() const
	{ return std::pair<const T&, const T&>(stack_at(1u), stack_at(0u)); }

	std::pair<T&, T&> stack_top_2() const
	{ return std::pair<T&, T&>(stack_at(1u), stack_at(0u)); }

	template <class L, class R>
	std::pair<const std::decay_t<L>&, const std::decay_t<R>&> stack_top_2(L WasmValue::* l, R WasmValue::* r) const
	{
		return std::pair<const std::decay_t<L>&, const std::decay_t<R>&>(
			stack_at(1u).get(l), stack_at(0u).get(r)
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

	template <class U>
	U stack_pop(U WasmValue::* mem)
	{
		auto v = stack_top(mem);
		stack_pop();
		return v;
	}

	template <class ... Args>
	T stack_emplace_top(Args&& ... args)
	{ return stack().emplace(std::forward<Args>(args)...); }

	void stack_pop_n(std::size_t n)
	{ stack().pop_n(n); }

	auto stack_size() const
	{ return stack().size(); }

	const WasmFunction* function()
	{ return self.code_.function(); }


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

	bool is_function_call() const
	{ return static_cast<bool>(function()); }

	std::optional<WasmInstruction> next_instruction() const
	{ return code_.next_instruction(); }

	void advance(const WasmInstruction& instr)
	{
		assert(next_instruction());
		if(not is_next_instruction(instr))
			throw std::logic_error("Instruction does not match program counter on call stack.");
		code_.advance(instr);
	}
	
	bool is_next_instruction(const WasmInstruction& instr) const
	{ return instr.source().data() == code_.data(); }

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

	CodeView code_;
	const gsl::span<T> locals_;
	block_list_type blocks_;
};

template <class T>
struct WasmCallStack 
{
	using frame_stack_type = ListStack<
		WasmStackFrame<T>, 
		pmr::polymorphic_allocator<WasmStackFrame<T>>
	>;
	using frame_iterator = list_type::const_iterator;

	WasmCallStack(StackResource& 

	const WasmStackFrame<T>& current_frame() const
	{
		assert(not frames_.empty());
		return frames_.top();
	}

	CodeView call_function(const Function& func, const WasmInstruction& call_instr)
	{
		assert(call_instr.opcode() == OpCode::CALL);
		if(func.is_wasm_function())
		{
			return call_wasm_function(func.get_wasm_function(), call_instr);
		}
		else
		{
			call_c_function(func.get_c_function());
			return call_instr.after();
		}
	}

	CodeView call_wasm_function(const WasmFunction& func, const WasmInstruction& call_instr)
	{
		assert(call_instr.opcode() == OpCode::CALL);
		auto& stack = frames_.empty() ? frames_.top().stack() : base_stack_;
		auto func_sig = signature(func);
		auto arg_types = param_types(func_sig);
		auto locals_types = locals(func);
		auto ret_count = return_count(func);
		if(stack.size() < arg_types.size())
			assert(false); // TODO: throw an exception

		// push zero-initialized locals onto the stack.
		auto guard_ = make_stack_size_guard(stack);
		for(LanguageType type: locals_types)
		{
			tp::visit_value_type(
				[&](auto WasmValue::* p) { stack.emplace(p, 0); }, type
			);
		}
		auto return_address = call_instr.after().pos();
		auto locals_vector_size = locals_types.size() + arg_types.size();
		auto locals_pos = stack.data() + (stack.size() - locals_vector_size);
		gsl::span<T> locals_vector(locals_pos, locals_vector_size);
		frames_.emplace(func, return_address, locals_vector);
		return CodeView(func);
	}

	void return_from_expression()
	{
		assert(frames_.size() == 1u);
		_recurse_return_from_frame_unchecked();
	}

	[[nodiscard]]
	CodeView end_block()
	{
		if(const char* p = top_frame().try_branch_top(); p)
		{
			
		}
	}

	[[nodiscard]]
	CodeView return_from_function()
	{
		auto& frame = top_frame();
		assert(frame.is_function_call());
		auto ret_types = return_types(signature(*frame.funtion()));
		assert(frame.stack_size() >= ret_types.size());
		CodeView ret_addr = top_frame().return_address();
		_recurse_return_from_frame(return_types);
		return ret_addr;
	}

	[[nodiscard]]
	CodeView call_table_function(const WasmTable& table, const WasmFunctionSignature& sig, const WasmInstruction& instr)
	{
		assert(instr.opcode() == OpCode::CALL_INDIRECT);
		assert(frames_.empty() or top_frame().can_exectute_instruction(instr));
		auto& stack = top_stack();
		wasm_uint32_t offset = reinterpret_cast<const wasm_uint32_t&>(stack.top().get(tp::i32_c));
		const TableFunction& func = table.at(offset);
		if(func.is_null())
			throw NullTableFunctionError();

		if(func.is_wasm_function())
		{
			auto& f = func.get_wasm_function();
			if(signature(f) != sig)
				throw BadTableFunctionSignature();
			stack.pop();
			return call_wasm_function(func.get_wasm_function());
		}
		assert(func.is_c_function());
		auto& f = func.get_c_function();
		if(f.signature() != sig)
			throw BadTableFunctionSignature();

		{ /* scope */
			auto stack_size = stack.size();
			auto guard_ = make_scope_guard(
				[&, stack_size]{
					// poor man's check to make sure the c function call  
					// doesn't modify 'this'.
					assert(stack.size() == stack_size);
				}
			call_c_function(f, offset);
		} /* /scope */
		return instr.after();
	}

	void call_c_function(const CFunction& cfunc) const
	{
		const auto& sig = cfunc.signature();
		auto param_types = param_types(sig);
		auto return_types = return_types(sig);
		auto& stack = top_stack();
		assert(stack.size() >= param_types.size());
		auto args = gsl::span<const T>(stack.data(), stack.size()).last(param_types.size());
		{
			auto guard_ = make_stack_size_guard(stack());
			for(LanguageType value_tp: return_types)
				stack.emplace(value_tp);
			auto results = gsl::span<T>(stack.data(), stack.size()).last(return_types.size());
			cfunc(results, args);
		}
		assert(stack.size() >= return_types.size() + param_types.size());
		std::rotate(
			stack.begin(),
			stack.begin() + return_types.size(),
			stack.begin() + return_types.size() + param_types.size()
		);
		stack.pop_n(param_types.size());
	}

	std::optional<WasmInstruction> next_instruction() const
	{ return top_frame().next_instruction(); }

	void execute(const WasmInstruction& instr, const WasmModule& module) const
	{
		if(not top_frame().is_next_instruction())
			throw std::logic_error("Instruction does not match program counter on call stack.");
		CodeView next = instr.execute(*this, module);
		top_frame().code_.advance(next);
	}

private:
	WasmStackFrame<T>& top_frame()
	{
		if(frames_.empty())
			throw std::out_of_range("Attempt to access out-of-bounds call stack frame.");
		return frames_.top();
	}

	SimpleStack<T>& top_stack()
	{
		if(frames_.empty())
			return base_stack_;
		return frames_.top().stack();
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

	void _recurse_return_from_frame_unchecked()
	{
		if(auto& stack = top_frame().stack(); stack.empty())
		{
			frames_.pop();
		}
		else
		{
			T ret_v = stack.pop();
			_recurse_return_from_frame_unchecked();
			top_stack().emplace(ret_v);
		}
	}
	void _recurse_return_from_frame(const gsl::span<const LanguageType>& return_types, std::size_t index = 0u)
	{
		assert(not frames_.empty());
		if(index < return_types.size())
		{
			// one of the return values from the callee's frame
			tp::visit_value_type(
				[&](auto WasmValue::* p) {
					auto ret_v = top_frame().stack_at(index, p);
					// keep recursing until we exhaust the return
					// values.  Once we've done that, pop the callee's 
					// frame, and then pop the locals vector and arguments 
					// off the caller's stack.
					_recurse_return_from_frame(return_types, index);
					// now that we've popped the callee's frame from the call stack, 
					// and the locals vector off of the caller's frame's stack, push 
					// the return values onto the callee's frame's stack.
					//
					// note that there may be no frame below the one we popped, in which
					// case, the return values are pushed onto 'this->base_stack_'
					top_stack().stack_emplace_top(p, ret_v);
				},
				return_types[return_types.size() - index]
			);
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

	frame_stack_type frames_;
	SimpleStack<T> base_stack_;
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
