#ifndef VM_OP_OPS_H
#define VM_OP_OPS_H

#include "wasm_base.h"
#include "wasm_value.h"
#include "WasmInstruction.h"
#include "utilities/bit_cast.h"
#include <functional>

namespace wasm::opc {

struct TrapError:
	public std::logic_error 
{
	using std::logic_error::logic_error;
};

struct UnreachableError:
	public TrapError
{
	using TrapError::TrapError;
};

struct BadStackAccessError:
	public ValidationError
{
	BadStackAccessError():
		ValidationError("Attempt to access out-of-bounds value on the stack.")
	{
		
	}
};


template <class Results, class Params>
struct OpSignature {
	using results = Results;
	using params = Params;
};

template <WasmInstruction Op>
struct op_traits;

template <WasmInstruction Op>
struct FuncOp;

struct NullaryOp { void operator()() const {} };


/// Binary Arithmetic (pure) Operations
namespace detail {

template <class Binop, class Result, class Left, class Right>
inline const auto make_binary_op(
	Binop binop,
	Result WasmValue::* result_p,
	Left WasmValue::* left_p,
	Right WasmValue::* right_p
)
{
	static_assert(std::is_invocable_v<Binop, Left, Right>);
	static_assert(
		std::is_same_v<
			std::decay_t<Result>,
			std::decay_t<std::invoke_result_t<Binop, Left, Right>>
		>
	);
	return [=](auto& call_stack, WasmModule&, auto&, const CodeView& after, auto&) {
		call_stack.top_frame().stack_pop_2_push_1(
			result_p,
			std::apply(binop, frame.stack_top_2(left_p, right_p))
		);
		return after;
	};
};


template <class Unop, class Result, class Input>
auto make_unary_op(
	Unop unop,
	Result WasmValue::* result_p,
	Input WasmValue::* input_p,
)
{
	static_assert(std::is_invocable_v<Binop, Input>);
	static_assert(
		std::is_same_v<
			std::decay_t<Result>,
			std::decay_t<std::invoke_result_t<Binop, Input>>
		>
	);
	return [=](auto& call_stack, WasmModule&, auto&, const CodeView& after, auto&) {
		call_stack.top_frame().stack_pop_1_push_1(
			result_p, std::invoke(unop, frame.stack_top(input_p))
		);
		return after;
	};
};


template <class Function>
struct NoDiscard {

	template <class ... Args>
	NoDiscard(Args&& ... args):
		func_(std::forward<Args>(args))
	{
			
	}

	template <class ... Args>
	[[nodiscard]] decltype(auto) operator()(Args&& ... args) const & {
		return std::invoke(func_, std::forward<Args>(args)...);
	}

	template <class ... Args>
	[[nodiscard]] decltype(auto) operator()(Args&& ... args) const && {
		return std::invoke(std::move(func_), std::forward<Args>(args)...);
	}

	template <class ... Args>
	[[nodiscard]] decltype(auto) operator()(Args&& ... args) & {
		return std::invoke(func_, std::forward<Args>(args)...);
	}

	template <class ... Args>
	[[nodiscard]] decltype(auto) operator()(Args&& ... args) && {
		return std::invoke(std::move(func_), std::forward<Args>(args)...);
	}

private:
	Function func_;
};

template <class Func>
NoDiscard<std::decay_t<Func>> make_nodiscard(Func&& func) {
	return NoDiscard<std::decay_t<Func>>(std::forward<Func>(func));
}

} /* namespace detail */


template <OpCode Op>
inline const auto op_func
	= [](auto& stack, auto& module, auto& )
	{ throw BadOpcodeError(); };

/// Control Flow Operations
template <>
inline const auto op_func<OpCode::NOP> = [](){ return;	};

template <>
inline const auto op_func<OpCode::UNREACHABLE>
	= []() { throw UnreachableError(); };

template <>
inline const auto op_func<OpCode::BLOCK>
	= [](auto& call_stack, WasmModule&, const CodeView& pos, const CodeView& after, const BlockImmediate& immed) -> CodeView
{
	auto sig = signature(immed);
	auto offset = offset(immed);
	assert(offset > 0u);
	call_stack.top_frame().push_block(pos.jump(offset), sig);
	return after;
};

template <>
inline const auto op_func<OpCode::LOOP>
	= [](auto& call_stack, WasmModule&, const CodeView& pos, const CodeView& after, LanguageType immed) -> CodeView
{
	call_stack.top_frame().push_block(after, sig);
	return after;
};

template <>
inline const auto op_func<OpCode::IF>
	= [](auto& call_stack, WasmModule&, const CodeView& pos, const CodeView& after, const IfImmediate& immed) -> CodeView
		-> CodeView
{
	auto& frame = call_stack.top_frame();
	auto condition = reinterpret_cast<const wasm_uint32_t&>(frame.stack_top(tp::i32_c));
	assert(instr.opcode() == static_cast<char>(OpCode::IF));
	// CodeView starting at the END opcode for this if block
	auto end_pos = pos.jump(end_offset(immed));
	auto sig = signature(immed);
	if(static_cast<bool>(condition))
	{
		// push the IF block
		frame.push_block(end_pos, sig);
		return after;
	}
	else
	{
		assert(else_offset(immed) != 0u);
		frame.push_block(end_pos, sig);
		// CodeView starting just after the ELSE opcode for this if block
		return pos.jump(else_offset(immed));
	}
};

template <>
inline const auto op_func<OpCode::ELSE>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView&, std::monostate) -> CodeView
{
	return call_stack.top_frame().branch_top();
};

template <>
inline const auto op_func<OpCode::END>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, std::monostate) -> CodeView
{
	if(after.size() == 0u)
		return call_stack.return_from_frame();
	else
		return call_stack.top_frame().branch_top();
};

template <>
inline const auto op_func<OpCode::BR>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView&, wasm_uint32_t depth) -> CodeView
{
	return call_stack.top_frame().branch_depth(depth);
};

template <>
inline const auto op_func<OpCode::BR_IF>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t depth) -> CodeView
{
	auto& frame = call_stack.top_frame();
	wasm_uint32_t cond = reinterpret_cast<const wasm_uint32_t&>(frame_stack_top(tp::i32));
	auto tmp = frame.stack_pop();
	if(static_cast<bool>(cond))
	{
		auto guard_ = make_scope_guard([&](){ frame.stack_emplace_top(tmp); });
		return frame.branch_depth(depth);
	}
	return after;
};

template <>
inline const auto op_func<OpCode::BR_TABLE>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView&, auto table) -> CodeView
{
	auto& frame = call_stack.top_frame();
	wasm_uint32_t index = reinterpret_cast<const wasm_uint32_t&>(frame.stack_top(tp::i32));
	wasm_uint32_t depth = table.at(index);
	auto tmp = frame.stack_pop();
	auto guard_ = make_scope_guard([&](){ frame.stack_emplace_top(tmp); });
	return frame.branch_depth(depth);
};

template <>
inline const auto op_func<OpCode::RETURN>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView&, std::monostate) -> CodeView
{
	return call_stack.return_from_frame();
}

/// Function Calls
template <>
inline const auto op_func<OpCode::CALL>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t index) -> CodeView
{
	return call_stack.call_function(module.function_at(index), after);
};

template <>
inline const auto op_func<OpCode::CALL_INDIRECT>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t type_index) -> CodeView
{
	call_stack.call_table_function(module.table_at(0), module.type_at(type_index));
	return after;
};

/// Variable Access
template <>
inline const auto op_func<OpCode::GET_LOCAL>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t local_index) -> CodeView
{
	auto& frame = call_stack.top_frame();
	frame.stack_emplace_top(frame.local_at(local_index));
	return after;
};

template <>
inline const auto op_func<OpCode::TEE_LOCAL>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t local_index) -> CodeView
{
	auto& frame = call_stack.top_frame();
	set(frame.local_at(local_index), frame.stack_top());
	return after;
};

template <>
inline const auto op_func<OpCode::SET_LOCAL>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, wasm_uint32_t local_index) -> CodeView
{
	auto& frame = call_stack.top_frame();
	op_func<OpCode::TEE_LOCAL>(frame, local_index);
	frame.stack_pop();
	return after;
};

template <>
inline const auto op_func<OpCode::GET_GLOBAL>
	= [](auto& call_stack, WasmModule& module, const auto&, const CodeView& after, wasm_uint32_t global_index) -> CodeView
{
	auto& frame = call_stack.top_frame();
	const auto& g = module.global_at(global_index);
	frame.stack_emplace_top(static_cast<TaggedWasmValue>(g));
	return after;
};

template <>
inline const auto op_func<OpCode::SET_GLOBAL>
	= [](auto& call_stack, WasmModule& module, const auto&, const CodeView& after, wasm_uint32_t global_index) -> CodeView
{
	auto& frame = call_stack.top_frame();
	auto& g = module.global_at(global_index);
	set(g, frame.stack_top());
	frame.stack_pop();
	return after;
};

/// Parametric Operations
template <>
inline const auto op_func<OpCode::SELECT>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, std::monostate) -> CodeView 
{
	auto& frame = call_stack.top_frame();
	// throw early so that the erroneous state of the stack can be inspected
	// for debugging purposes.
	if(frame.stack_size() < 3u)
		BadStackAccess();
	auto cond = frame.stack_pop(tp::i32);
	auto alt = frame.stack_pop();
	if(not cond)
		frame.stack_pop_1_push_1(alt);
	return after;
};

template <>
inline const auto op_func<OpCode::DROP>
	= [](auto& call_stack, WasmModule&, const auto&, const CodeView& after, std::monostate) -> CodeView
{
	call_stack.top_frame().stack_pop();
	return after;
};


/// Memory Operations

namespace detail {
enum class LoadStoreOp: bool {
	LOAD, STORE
};

inline const auto identity = [](auto&& value) -> decltype(auto) { return value; };

template <class Dest, class Src>
inline const auto sign_ext = [](auto src) -> Dest {
	static_assert(std::conjunction_v<std::is_integral<Dest> and std::is_signed<Dest>>);
	static_assert(std::conjunction_v<std::is_integral<Src> and std::is_signed<Src>>);
	static_assert(sizeof(Dest) > sizeof(Src));
	static_assert(std::is_same_v<decltype(src), Src>);
	using USrc = std::make_unsigned_v<Src>;
	using UDest = std::make_unsigned_v<Dest>;
	constexpr std::size_t src_bits = CHAR_BIT * sizeof(Src);
	constexpr UDest signbit = UDest(1u) << (src_bits - 1u);
	USrc usrc = reinterpret_cast<const USrc&>(src);
	return reinterpret_cast<const Dest&>(UDest((usrc ^ signbit) - signbit));
};

template <class Dest, class Src>
inline const auto widen = [](auto src) -> Dest {
	static_assert(std::conjunction_v<std::is_integral<Dest> and std::is_signed<Dest>>);
	static_assert(std::conjunction_v<std::is_integral<Src> and std::is_signed<Src>>);
	static_assert(sizeof(Dest) > sizeof(Src));
	static_assert(std::is_same_v<decltype(src), Src>);
	using USrc = std::make_unsigned_v<Src>;
	using UDest = std::make_unsigned_v<Dest>;
	USrc usrc = reinterpret_cast<const USrc&>(src);
	return reinterpret_cast<const Dest&>(static_cast<UDest>(usrc));
};

template <class Dest, class Src>
inline const auto wrap_value = [](auto src) -> Dest {
	static_assert(std::conjunction_v<std::is_integral<Dest> and std::is_signed<Dest>>);
	static_assert(std::conjunction_v<std::is_integral<Src> and std::is_signed<Src>>);
	static_assert(sizeof(Dest) < sizeof(Src));
	static_assert(std::is_same_v<decltype(src), Src>);
	using USrc = std::make_unsigned_v<Src>;
	using UDest = std::make_unsigned_v<Dest>;
	return static_cast<UDest>(reinterpret_cast<const USrc&>(src));
};

template <LoadStoreOp LoadStore, class StoredType, class Member, class Convert>
auto memory_op(Member member, Convert convert = idendity) {
	return [=](
		auto& call_stack,
		WasmModule& module,
		[[maybe_unused]] auto pos,
		const CodeView& after,
		const MemoryImmediate& immed
	)
	{
		auto& frame = call_stack.top_frame();
		wasm_uint32_t offset = offset(immed);
		static_assert(std::is_trivially_copyable_v<StoredType>);
		auto& mem = module.memory_at(0);
		if constexpr(LoadStore == LoadStoreOp::LOAD)
		{
			wasm_uint32_t base = reinterpret_cast<const wasm_uint32_t&>(frame.stack_top(tp::i32_c));
			StoredType value = load_little_endian<StoredType>(mem, base, offset);
			frame.stack_pop_1_push_1(member, convert(value));
		}
		else
		{
			static_assert(LoadStore == LoadStoreOp::STORE);
			const auto& [base, value] = frame.stack_top_2(tp::i32_c, member);
			StoredType converted_value = convert(value);
			wasm_uint32_t base = reinterpret_cast<const wasm_uint32_t&>(base);
			store_little_endian<StoredType>(mem, base, offset, converted_value);
		}
	};
}

}

// load
template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD>
	= detail::memory_op<wasm_sint32_t>(tp::i32);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD>
	= detail::memory_op<wasm_sint64_t>(tp::i64);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::F32_LOAD>
	= detail::memory_op<wasm_float32_t>(tp::f32);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::F64_LOAD>
	= detail::memory_op<wasm_float64_t>(tp::f64);

// store
template <>
inline const auto op_func<detail::LoadStoreOp::STORE, OpCode::I32_STORE>
	= detail::memory_op<wasm_sint32_t>(tp::i32);

template <>
inline const auto op_func<detail::LoadStoreOp::STORE, OpCode::I64_STORE>
	= detail::memory_op<wasm_sint64_t>(tp::i64);

template <>
inline const auto op_func<detail::LoadStoreOp::STORE, OpCode::F32_STORE>
	= detail::memory_op<wasm_float32_t>(tp::f32);

template <>
inline const auto op_func<detail::LoadStoreOp::STORE, OpCode::F64_STORE>
	= detail::memory_op<wasm_float64_t>(tp::f64);


// sign-extending load
template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD8_S>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::sign_ext<wasm_sint32_t, wasm_sint8_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD16_S>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::sign_ext<wasm_sint32_t, wasm_sint16_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD8_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::sign_ext<wasm_sint64_t, wasm_sint8_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD16_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::sign_ext<wasm_sint64_t, wasm_sint16_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD32_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::sign_ext<wasm_sint64_t, wasm_sint32_t>
	);

// non-sign-extending load
template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD8_U>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::widen<wasm_sint32_t, wasm_sint8_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD16_U>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::widen<wasm_sint32_t, wasm_sint16_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD8_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::widen<wasm_sint64_t, wasm_sint8_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD16_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::widen<wasm_sint64_t, wasm_sint16_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD32_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::widen<wasm_sint64_t, wasm_sint32_t>
	);

// truncating stores
template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD8_U>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::wrap_value<wasm_sint8_t, wasm_sint32_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I32_LOAD16_U>
	= detail::memory_op<wasm_sint32_t>(
		tp::i32, detail::wrap_value<wasm_sint16_t, wasm_sint32_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD8_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::wrap_value<wasm_sint8_t, wasm_sint64_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD16_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::wrap_value<wasm_sint16_t, wasm_sint64_t>
	);

template <>
inline const auto op_func<detail::LoadStoreOp::LOAD, OpCode::I64_LOAD32_S>
	= detail::memory_op<wasm_sint64_t>(
		tp::i64, detail::wrap_value<wasm_sint32_t, wasm_sint64_t>
	);

// misc memory ops
template <>
inline const auto op_func<OpCode::GROW_MEMORY> 
	= [](auto& call_stack, WasmModule& module, [[maybe_unused]] auto pos, CodeView next, const std::monostate&) {
		auto& mem = module.memory_at(0);
		auto top_v = call_stack.top_frame().stack_top(tp::i32_c);
		wasm_uint32_t delta = reinterpret_cast<const wasm_uint32_t&>(top_v);
		auto result = grow_memory(mem, delta);
		static_assert(std::is_same_v<decltype(result), wasm_sint32_t>);
		call_stack.top_frame().stack_pop_1_push_1(tp::i32, result);
		return next;
	};

template <>
inline const auto op_func<OpCode::CURRENT_MEMORY> 
	= [](auto& call_stack, WasmModule& module, [[maybe_unused]] auto pos, CodeView next, const std::monostate&) {
		auto& mem = module.memory_at(0);
		auto sz = current_memory(mem);
		wasm_uint32_t u32_sz = sz;
		assert(sz == u32_sz);
		wasm_sint32_t i32_sz = reinterpret_cast<const wasm_sint32_t&>(u32_sz);
		call_stack.top_frame().stack_emplace_top(tp::i32, i32_sz);
		return next;
	};

/// i32 ops
template <>
inline const auto op_func<OpCode::I32_ADD>
	= detail::make_binary_op(arith::i32_add, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_SUB>
	= detail::make_binary_op(arith::i32_sub, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_MUL>
	= detail::make_binary_op(arith::i32_mul, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_DIV_S>
	= detail::make_binary_op(arith::i32_div_s, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_DIV_U>
	= detail::make_binary_op(arith::i32_div_u, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_REM_S>
	= detail::make_binary_op(arith::i32_rem_s, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_REM_U>
	= detail::make_binary_op(arith::i32_rem_u, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_AND>
	= detail::make_binary_op(arith::i32_and, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_OR>
	= detail::make_binary_op(arith::i32_or, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_XOR>
	= detail::make_binary_op(arith::i32_xor, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_SHL>
	= detail::make_binary_op(arith::i32_shl, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_SHR_U>
	= detail::make_binary_op(arith::i32_shr_u, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_SHR_S>
	= detail::make_binary_op(arith::i32_shr_s, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_ROTL>
	= detail::make_binary_op(arith::i32_rotl, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_ROTR>
	= detail::make_binary_op(arith::i32_rotr, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_ROTR>
	= detail::make_binary_op(arith::i32_rotr, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_CLZ>
	= detail::make_unary_op(arith::i32_clz, tp::i32, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_CTZ>
	= detail::make_unary_op(arith::i32_ctz, tp::i32, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_POPCNT>
	= detail::make_unary_op(arith::i32_popcnt, tp::i32, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_EQ>
	= detail::make_binary_op(arith::i32_eq, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_NE>
	= detail::make_binary_op(arith::i32_ne, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_LT>
	= detail::make_binary_op(arith::i32_lt, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_LE>
	= detail::make_binary_op(arith::i32_le, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_GT>
	= detail::make_binary_op(arith::i32_gt, tp::i32, tp::i32_c, tp::i32_c);

template <>
inline const auto op_func<OpCode::I32_GE>
	= detail::make_binary_op(arith::i32_ge, tp::i32, tp::i32_c, tp::i32_c);


/// i64 ops

template <>
inline const auto op_func<OpCode::I64_ADD>
	= detail::make_binary_op(arith::i64_add, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_SUB>
	= detail::make_binary_op(arith::i64_sub, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_MUL>
	= detail::make_binary_op(arith::i64_mul, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_DIV_S>
	= detail::make_binary_op(arith::i64_div_s, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_DIV_U>
	= detail::make_binary_op(arith::i64_div_u, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_REM_S>
	= detail::make_binary_op(arith::i64_rem_s, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_REM_U>
	= detail::make_binary_op(arith::i64_rem_u, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_AND>
	= detail::make_binary_op(arith::i64_and, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_OR>
	= detail::make_binary_op(arith::i64_or, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_XOR>
	= detail::make_binary_op(arith::i64_xor, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_SHL>
	= detail::make_binary_op(arith::i64_shl, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_SHR_U>
	= detail::make_binary_op(arith::i64_shr_u, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_SHR_S>
	= detail::make_binary_op(arith::i64_shr_s, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_ROTL>
	= detail::make_binary_op(arith::i64_rotl, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_ROTR>
	= detail::make_binary_op(arith::i64_rotr, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_ROTR>
	= detail::make_binary_op(arith::i64_rotr, tp::i64, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_CLZ>
	= detail::make_unary_op(arith::i64_clz, tp::i64, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_CTZ>
	= detail::make_unary_op(arith::i64_ctz, tp::i64, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_POPCNT>
	= detail::make_unary_op(arith::i64_popcnt, tp::i64, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_EQ>
	= detail::make_binary_op(arith::i64_eq, tp::i32, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_NE>
	= detail::make_binary_op(arith::i64_ne, tp::i32, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_LT>
	= detail::make_binary_op(arith::i64_lt, tp::i32, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_LE>
	= detail::make_binary_op(arith::i64_le, tp::i32, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_GT>
	= detail::make_binary_op(arith::i64_gt, tp::i32, tp::i64_c, tp::i64_c);

template <>
inline const auto op_func<OpCode::I64_GE>
	= detail::make_binary_op(arith::i64_ge, tp::i32, tp::i64_c, tp::i64_c);


// f32 ops

template <>
inline const auto op_func<OpCode::F32_ADD>
	= detail::make_binary_op(arith::f32_add, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_SUB>
	= detail::make_binary_op(arith::f32_sub, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_MUL>
	= detail::make_binary_op(arith::f32_mul, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_DIV>
	= detail::make_binary_op(arith::f32_div, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_SQRT>
	= detail::make_unary_op(arith::f32_sqrt, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_MIN>
	= detail::make_binary_op(arith::f32_min, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_MAX>
	= detail::make_binary_op(arith::f32_max, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_CEIL>
	= detail::make_unary_op(arith::f32_ceil, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_FLOOR>
	= detail::make_unary_op(arith::f32_floor, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_NEAREST>
	= detail::make_unary_op(arith::f32_nearest, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_NEG>
	= detail::make_unary_op(arith::f32_neg, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_COPYSIGN>
	= detail::make_binary_op(arith::f32_copysign, tp::f32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_EQ>
	= detail::make_binary_op(arith::f32_eq, tp::i32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_NE>
	= detail::make_binary_op(arith::f32_ne, tp::i32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_LT>
	= detail::make_binary_op(arith::f32_lt, tp::i32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_LE>
	= detail::make_binary_op(arith::f32_le, tp::i32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_GT>
	= detail::make_binary_op(arith::f32_gt, tp::i32, tp::f32_c, tp::f32_c);

template <>
inline const auto op_func<OpCode::F32_GE>
	= detail::make_binary_op(arith::f32_ge, tp::i32, tp::f32_c, tp::f32_c);


// f64 ops

template <>
inline const auto op_func<OpCode::F64_ADD>
	= detail::make_binary_op(arith::f64_add, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_SUB>
	= detail::make_binary_op(arith::f64_sub, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_MUL>
	= detail::make_binary_op(arith::f64_mul, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_DIV>
	= detail::make_binary_op(arith::f64_div, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_SQRT>
	= detail::make_unary_op(arith::f64_sqrt, tp::f64, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_MIN>
	= detail::make_binary_op(arith::f64_min, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_MAX>
	= detail::make_binary_op(arith::f64_max, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_CEIL>
	= detail::make_unary_op(arith::f64_ceil, tp::f64, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_FLOOR>
	= detail::make_unary_op(arith::f64_floor, tp::f64, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_NEAREST>
	= detail::make_unary_op(arith::f64_nearest, tp::f64, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_NEG>
	= detail::make_unary_op(arith::f64_neg, tp::f64, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_COPYSIGN>
	= detail::make_binary_op(arith::f64_copysign, tp::f64, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_EQ>
	= detail::make_binary_op(arith::f64_eq, tp::i32, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_NE>
	= detail::make_binary_op(arith::f64_ne, tp::i32, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_LT>
	= detail::make_binary_op(arith::f64_lt, tp::i32, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_LE>
	= detail::make_binary_op(arith::f64_le, tp::i32, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_GT>
	= detail::make_binary_op(arith::f64_gt, tp::i32, tp::f64_c, tp::f64_c);

template <>
inline const auto op_func<OpCode::F64_GE>
	= detail::make_binary_op(arith::f64_ge, tp::i32, tp::f64_c, tp::f64_c);


// conversion ops

// truncate signed
template <>
inline const auto op_func<OpCode::I32_TRUNC_S_F32>
	= detail::make_unary_op(arith::i32_trunc_s_f32, tp::i32, tp::f32_c);

template <>
inline const auto op_func<OpCode::I32_TRUNC_S_F64>
	= detail::make_unary_op(arith::i32_trunc_s_f64, tp::i32, tp::f64_c);

template <>
inline const auto op_func<OpCode::I64_TRUNC_S_F32>
	= detail::make_unary_op(arith::i64_trunc_s_f32, tp::i64, tp::f32_c);

template <>
inline const auto op_func<OpCode::I64_TRUNC_S_F64>
	= detail::make_unary_op(arith::i64_trunc_s_f64, tp::i64, tp::f64_c);

// truncate unsigned
template <>
inline const auto op_func<OpCode::I32_TRUNC_U_F32>
	= detail::make_unary_op(arith::i32_trunc_u_f32, tp::i32, tp::f32_c);

template <>
inline const auto op_func<OpCode::I32_TRUNC_U_F64>
	= detail::make_unary_op(arith::i32_trunc_u_f64, tp::i32, tp::f64_c);

template <>
inline const auto op_func<OpCode::I64_TRUNC_U_F32>
	= detail::make_unary_op(arith::i64_trunc_u_f32, tp::i64, tp::f32_c);

template <>
inline const auto op_func<OpCode::I64_TRUNC_U_F64>
	= detail::make_unary_op(arith::i64_trunc_u_f64, tp::i64, tp::f64_c);

// promote/demote
template <>
inline const auto op_func<OpCode::F32_DEMOTE_F64>
	= detail::make_unary_op(arith::f32_demote_f64, tp::f32, tp::f32_c);

template <>
inline const auto op_func<OpCode::F64_PROMOTE_F32>
	= detail::make_unary_op(arith::f64_promote_f32, tp::f64, tp::f32_c);

// signed int-to-float conversions
template <>
inline const auto op_func<OpCode::F32_CONVERT_S_I32>
	= detail::make_unary_op(arith::f32_convert_s_i32, tp::f32, tp::i32_c);

template <>
inline const auto op_func<OpCode::F32_CONVERT_S_I64>
	= detail::make_unary_op(arith::f32_convert_s_i64, tp::f32, tp::i64_c);

template <>
inline const auto op_func<OpCode::F64_CONVERT_S_I32>
	= detail::make_unary_op(arith::f64_convert_s_i32, tp::f64, tp::i32_c);

template <>
inline const auto op_func<OpCode::F64_CONVERT_S_I64>
	= detail::make_unary_op(arith::f64_convert_s_i64, tp::f64, tp::i64_c);

// unsigned int-to-float conversions
template <>
inline const auto op_func<OpCode::F32_CONVERT_U_I32>
	= detail::make_unary_op(arith::f32_convert_u_i32, tp::f32, tp::i32_c);

template <>
inline const auto op_func<OpCode::F32_CONVERT_U_I64>
	= detail::make_unary_op(arith::f32_convert_u_i64, tp::f32, tp::i64_c);

template <>
inline const auto op_func<OpCode::F64_CONVERT_U_I32>
	= detail::make_unary_op(arith::f64_convert_u_i32, tp::f64, tp::i32_c);

template <>
inline const auto op_func<OpCode::F64_CONVERT_U_I64>
	= detail::make_unary_op(arith::f64_convert_u_i64, tp::f64, tp::i64_c);


} /* namespace wasm::opc */

#define /* VM_OP_OPS_H */
