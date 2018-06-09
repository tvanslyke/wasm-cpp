#ifndef VM_CODE_CODE_VIEW_H
#define VM_CODE_CODE_VIEW_H

#include <cstddef>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include <array>
#include <bitset>
#include <optional>
#include <variant>
#include <algorithm>
#include <iosfwd>
#include <ostream>
#include <iomanip>
#include <gsl/span>
#include <gsl/gsl>
#include "wasm_base.h"


namespace wasm::opc {

namespace detail {

template <class It>
[[gnu::pure]]
std::tuple<wasm_uint32_t, wasm_uint32_t, It> read_memory_immediate(It first, It last)
{
	alignas(wasm_uint32_t) char buff1[sizeof(wasm_uint32_t)];
	alignas(wasm_uint32_t) char buff2[sizeof(wasm_uint32_t)];
	wasm_uint32_t flags;
	wasm_uint32_t offset;
	for(auto& chr: buff1)
	{
		assert(first != last);
		chr = *first++;
	}
	for(auto& chr: buff2)
	{
		assert(first != last);
		chr = *first++;
	}
	std::memcpy(&flags, buff1, sizeof(flags));
	std::memcpy(&offset, buff1, sizeof(offset));
	return std::make_tuple(flags, offset, first);
}

template <class T, class It>
[[gnu::pure]]
std::pair<T, It> read_serialized_immediate(It first, It last)
{
	static_assert(std::is_trivially_copyable_v<T>);
	T value;
	alignas(T) char buff[sizeof(T)];
	for(auto& chr: buff)
	{
		assert(first != last);
		chr = *first++;
	}
	std::memcpy(&value, buff, sizeof(value));
	return std::make_pair(value, first);
}

} /* namespace detail */

struct BadOpcodeError:
	public std::logic_error
{
	BadOpcodeError(OpCode op, const char* msg):
		std::logic_error(msg),
		opcode(op)
	{

	}
	
	const OpCode opcode;
};

template <class It, class Visitor>
decltype(auto) visit_opcode(Visitor visitor, It first, It last)
{
	assert(first != last);
	OpCode op;
	auto pos = first;
	using value_type = typename std::iterator_traits<It>::value_type;
	using underlying_type = std::underlying_type_t<OpCode>;
	if constexpr(not std::is_same_v<value_type, OpCode>)
	{
		value_type opcode = *pos++;
		op = static_cast<OpCode>(opcode);
		assert(static_cast<value_type>(op) == opcode);
	}
	else
	{
		op = *pos++;
	}

	// Handling the invalid opcode case is optional.
	if constexpr(std::is_invocable_v<Visitor, OpCode, std::nullopt_t>)
	{
		if(not opcode_exists(static_cast<underlying_type>(op)))
		{
			return visitor(
				first, 
				last,
				pos,
				op,
				BadOpcodeError(op, "Given op is not a valid WASM opcode.")
			);
		}
	}
	else
	{
		if(not opcode_exists(static_cast<underlying_type>(op)))
			assert(false);
	}
	
	if(op >= OpCode::I32_LOAD and op <= OpCode::I64_STORE32)
	{
		wasm_uint32_t flags, offset;
		std::tie(flags, offset, pos) = detail::read_memory_immediate(pos, last);
		return visitor(first, last, pos, op, flags, offset);
	}
	else if(
		(op >= OpCode::GET_LOCAL and op <= OpCode::SET_GLOBAL)
		or (op == OpCode::CALL or op == OpCode::CALL_INDIRECT)
		or (op == OpCode::BR or op == OpCode::BR_IF)
		or (op == OpCode::ELSE)
	)
	{
		wasm_uint32_t value;
		std::tie(value, pos) = detail::read_serialized_immediate<wasm_uint32_t>(pos, last);
		return visitor(first, last, pos, op, value);
	}
	else if(op == OpCode::BLOCK or op == OpCode::IF)
	{
		assert(first != last);
		LanguageType tp = static_cast<LanguageType>(*first++);
		wasm_uint32_t label;
		std::tie(label, pos) = detail::read_serialized_immediate<wasm_uint32_t>(pos, last);
		return visitor(first, last, pos, op, tp, label);
	}
	else if(op == OpCode::LOOP)
	{
		assert(first != last);
		LanguageType tp = static_cast<LanguageType>(*first++);
		return visitor(first, last, pos, op, tp);
	}
	else if(op == OpCode::BR_TABLE)
	{
		wasm_uint32_t len;
		std::tie(len, pos) = detail::read_serialized_immediate<wasm_uint32_t>(pos, last);
		auto base = pos;
		std::advance(pos, (1 + len) * sizeof(wasm_uint32_t));
		return visitor(first, last, pos, op, base, len);
	}
	
	switch(op)
	{
	case OpCode::I32_CONST: {
		wasm_sint32_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_sint32_t>(pos, last);
		return visitor(first, last, pos, op, v);
		break;
	}
	case OpCode::I64_CONST: {
		wasm_sint64_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_sint64_t>(pos, last);
		return visitor(first, last, pos, op, v);
		break;
	}
	case OpCode::F32_CONST: {
		wasm_float32_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_float32_t>(pos, last);
		return visitor(first, last, pos, op, v);
		break;
	}
	case OpCode::F64_CONST: {	
		wasm_float64_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_float64_t>(pos, last);
		return visitor(first, last, pos, op, v);
		break;
	}
	default:
		return visitor(op, first, pos, last);
	}
	assert(false and "Internal Error: All cases should have been handled by this point.");
}

struct MemoryImmediate:
	public std::pair<const wasm_uint32_t, const wasm_uint32_t>
{
	using std::pair<const wasm_uint32_t, const wasm_uint32_t>::pair;
};

wasm_uint32_t flags(const MemoryImmediate& immed)
{ return immed.first; }

wasm_uint32_t offset(const MemoryImmediate& immed)
{ return immed.second; }

struct BlockImmediate:
	public std::pair<const LanguageType, const wasm_uint32_t>
{
	using std::pair<const LanguageType, const wasm_uint32_t>::pair;
};

gsl::span<const LanguageType> signature(const BlockImmediate& immed)
{ return gsl::span<const LanguageType>(&(immed.first), 1u); }

std::size_t arity(const BlockImmediate& immed)
{ return signature(immed).size(); }

wasm_uint32_t offset(const BlockImmediate& immed)
{ return immed.second; }

struct BranchTableImmediate
{
	template <class ... T>
	BranchTableImmediate(T&& ... args):
		table_(std::forward<T>(args)...)
	{
		
	}

	wasm_uint32_t at(wasm_uint32_t idx) const
	{
		idx = std::min(std::size_t(table_.size() - 1u), std::size_t(idx));
		wasm_uint32_t depth;
		std::memcpy(&depth, table_.data() + idx, sizeof(depth));
		return depth;
	}

private:
	const gsl::span<const char[sizeof(wasm_uint32_t)]> table_;
};

struct CodeView
{
	struct Iterator;
	using value_type = WasmInstruction;
	using pointer = WasmInstruction*;
	using const_pointer = const WasmInstruction*;
	using reference = WasmInstruction&;
	using const_reference = WasmInstruction&;
	using size_type = std::string_view::size_type;
	using difference_type = std::string_view::difference_type;
	using iterator = Iterator;
	using const_iterator = iterator;

private:

	struct InstructionVisitor {

		template <class ... T>
		WasmInstruction operator()(
			const char* first,
			const char* last,
			const char* pos,
			OpCode op,
			T&& ... args
		)
		{
			assert(first < pos);
			assert(pos <= last);
			return make_instr(
				std::string_view(first, pos - first),
				op,
				last,
				std::forward<T>(args)...
			);
		}

	private:
		// Overload for instructions with immediate operands
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last)
		{ return WasmInstruction(view, op, last, std::monostate()); }

		template <
			class T,
			/* enable overloads for the simple alternatives. */
			class = std::enable_if_t<
				std::disjunction_v<
					std::is_same_v<std::decay_t<T>, wasm_uint32_t>,
					std::is_same_v<std::decay_t<T>, wasm_sint32_t>,
					std::is_same_v<std::decay_t<T>, wasm_sint64_t>,
					std::is_same_v<std::decay_t<T>, wasm_float32_t>,
					std::is_same_v<std::decay_t<T>, wasm_float64_t>
				>
			>
		>
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, T&& arg)
		{ return WasmInstruction(view, op, last, std::forward<T>(arg)); }

		/// Loop overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, LanguageType tp)
		{ return WasmInstruction(view, op, last, tp); }

		/// Branch table overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, const char* base, wasm_uint32_t len)
		{
			assert(last > base);
			std::size_t byte_count = base - last;
			std::size_t bytes_needed = sizeof(wasm_uint32_t) * (len + 1u);
			assert(byte_count >= bytes_needed);
			// the table and 'view' should end at the same byte address
			assert(view.data() + view.size() == (base + (len + 1u) * sizeof(wasm_uint32_t)));
			using buffer_type = const char[sizeof(wasm_uint32_t)];
			auto table = gsl::span<buffer_type>(reinterpret_cast<buffer_type*>(base), len + 1u);
			return WasmInstruction(view, op, last, table);
		}

		/// Block overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, LanguageType tp, wasm_uint32_t label)
		{ return WasmInstruction(view, op, last, BlockImmediate(tp, label)); }

		/// Memory overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, wasm_uint32_t flags, wasm_uint32_t offset)
		{ return WasmInstruction(view, op, last, MemoryImmediate(flags, offset)); }

		/// Invalid opcode overload
		[[noreturn]]
		WasmInstruction make_instr(std::string_view, OpCode, const char*, const BadOpCodeError& err)
		{ throw err; }

	};

public:
	
	struct Iterator {
		using value_type = CodeView::value_type;
		using difference_type = CodeView::difference_type;
		using pointer = CodeView::pointer;
		using reference = CodeView::value_type;
		using iterator_category = std::input_iterator_tag;
		
		friend bool operator==(const Iterator& left, const Iterator& right)
		{ return left.code_ == right.code_; }
		
		friend bool operator!=(const Iterator& left, const Iterator& right)
		{ return not (left == right); }
	private:
		gsl::not_null<CodeView*> code_;
	};

	CodeView(const WasmFunction& func):
		function_(&func),
		code_(code(func))
	{
		
	}

	const WasmFunction* function() const
	{ return function_; }

	OpCode current_op() const
	{
		assert(ready());
		return static_cast<WasmInstruction>(code_.front());
	}

	bool done() const
	{ return not code_.empty(); }

	std::optional<WasmInstruction> next_instruction() const
	{
		assert(ready());
		if(code_.size() == 0)
			return std::nullopt;
		return visit_opcode(InstructionVisitor{}, code_.data(), code_.data() + code_.size());
	}

	const char* pos() const
	{ return code_.data(); }

	void advance(const CodeView& other)
	{
		assert(function() == other.function());
		assert(ready());
		assert(code_.data() + code_.size() == other.code_.data() + other.code_.size());
		assert(code_.data() < other.code_.data());
		code_ = other.code_;
	}

private:

	bool ready() const
	{
		if(done())
			return false;
		assert(opcode_exists(code_.front()));
		return true;
	}
	const WasmFunction* function_;
	std::string_view code_;
};


} /* namespace opc */
} /* namespace wasm */





#endif /* VM_CODE_CODE_VIEW_H */
