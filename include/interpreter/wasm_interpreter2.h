#ifndef INTERPRETER_WASM_INTERPRETER2_H
#define INTERPRETER_WASM_INTERPRETER2_H

#include "wasm_value.h"
#include "wasm_instruction.h"
#include "interpreter/functional_ex.h"
#include "interpreter/WasmProgramStack.h"
#include "module/wasm_program_state.h"
#include "wasm_instruction.h"
#include "utilities/bit_cast.h"
#include "utilities/immediates.h"
#include <cmath>
#include <sstream>



struct wasm_runtime
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	
	wasm_runtime(wasm_program_state& state, WasmProgramStack& stack):
		program_state(state),
		call_stack(stack)
	{
		if(program_state.start_function.return_count() != 0)
		{
			std::cerr << "Warning: start function return count =/= 0 (" 
				<< program_state.start_function.return_count() << ")\n";
		}
		call_stack.call_op(program_state.start_function);
		assert(call_stack.frame_count() == 1);
	}

	wasm_program_state& program_state;
	WasmProgramStack& call_stack;

	// Frame stuff
	const opcode_t* pc(){ return call_stack.program_counter(); }
	
	// module stuff
	template <class ImmediateType>
	ImmediateType get_immediate()
	{
		return call_stack.read_immediate<ImmediateType>();
	}

	template <class ImmediateType>
	void skip_immediates(std::size_t count)
	{
		assert(pc());
		auto jump_pos = opcode_skip_immediates<ImmediateType>(pc(), count);
		call_stack.code_jump(jump_pos);
	}

	bool eval();

	void trap(const std::string& message)
	{ throw trap_error(message); }

	
	void trap_bad_memory_access(std::size_t address, std::size_t offset) 
	{
		std::stringstream message;
		message << "Attempt to access out-of-bounds linear memory address ";
		message << std::hex << address << ' ';
		message << "with offset ";
		message << offset;
		message << ".";
		trap(message.str());
	}

	void trap_bad_instruction(opcode_t op) 
	{
		std::stringstream message;
		message << "Unknown instruction ";
		message << std::hex << op << ' ';
		message << "encountered in program. (this is a validation error - compiler or interpreter has a bug)";
		trap(message.str());
	}


	void push(wasm_value_t v) { call_stack.push_value(v); }

	template <class T>	
	void push(T wasm_value_t::* member, wasm_value_t v) 
	{
		wasm_value_t value;
		value.*member = v.*member;
		call_stack.push_value(value);
	}
	
	wasm_value_t pop() { return call_stack.pop_value(); }

	/* INSTRUCTIONS */

	// control flow
	void block_op()
	{
		call_stack.block_op(get_immediate<signed char>(), call_stack.read_label_immediate());
	}

	void loop_op()
	{
		call_stack.loop_op();
	}
	

	void br_op()
	{
		call_stack.br_op(get_immediate<wasm_uint32_t>());
	}
	
	void br_if_op()
	{
		auto depth = get_immediate<wasm_uint32_t>();
		call_stack.br_if_op(depth);
	}
	
	void br_table_op()
	{
		auto sz = get_immediate<wasm_uint32_t>();
		std::size_t idx = pop().u32;
		if(idx < sz) // default branch
			idx = sz;
		auto branch_depth = call_stack.immediate_array_at<wasm_uint32_t>(idx);
		call_stack.br_op(branch_depth);
	}
	
	void if_op()
	{
		auto cond = pop().u32;
		block_op();
		if(not cond)
			call_stack.br_op(0);
	}
	
	void else_op()
	{
		call_stack.else_op(call_stack.read_label_immediate());
	}
	
	bool end_op()
	{
		return call_stack.end_op();
	}
	
	void return_op()
	{
		call_stack.return_op();
	}

	// parametric instructions
	void get_local_op()
	{ call_stack.get_local_op(call_stack.read_immediate<wasm_uint32_t>()); }

	void set_local_op()
	{ call_stack.set_local_op(call_stack.read_immediate<wasm_uint32_t>()); }

	void tee_local_op()
	{ call_stack.tee_local_op(call_stack.read_immediate<wasm_uint32_t>()); }


	void get_global_op()
	{
		// TODO: store member-pointer with globals 
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		call_stack.push_value(program_state.const_global_at(idx));
	}
	
	void set_global_op()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		program_state.global_at(idx) = pop();
	}


	void call_op()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		const auto& func = program_state.function_at(idx);
		call_stack.call_op(func);
	}
	
	void call_indirect_op()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		std::size_t func_idx = program_state.table_function_index(idx);
		const auto& func = program_state.function_at(func_idx);
		call_stack.call_op(func);
	}

	void select_op()
	{
		wasm_uint32_t pred = pop().u32;
		wasm_value_t other = pop();
		if(not pred)
		{
			pop();
			push(other);
		}
	}

	template <class T>
	void load(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		wasm_value_t value;
		if(not (program_state.const_memory_at(0).load(address, offset, value, member)))
			trap_bad_memory_access(address, offset);
		push(value);
	}

	template <std::size_t Sz, class T>
	void narrow_load(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		wasm_value_t value;
		if(not (program_state.const_memory_at(0).narrow_load<Sz>(address, offset, value, member)))
			trap_bad_memory_access(address, offset);
		push(value);
	}

	template <class T>
	void store(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto value = pop();
		auto address = pop().u32;
		if(not (program_state.memory_at(0).store(address, offset, value, member)))
			trap_bad_memory_access(address, offset);
	}

	template <std::size_t Sz, class T>
	void wrap_store(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto value = pop();
		auto address = pop().u32;
		if(not (program_state.memory_at(0).wrap_store<Sz>(address, offset, value, member)))
			trap_bad_memory_access(address, offset);
	}

	void grow_memory()
	{
		auto v = pop().u32;
		wasm_value_t value;
		value.s32 = program_state.memory_at(0).grow_memory(v);
		push(s_32, value);
	}

	void current_memory() 
	{
		wasm_value_t value;
		value.u32 = program_state.memory_at(0).current_memory();
		push(u_32, value);
	}

	template <class T>
	void push_immediate(T wasm_value_t::* member)
	{
		wasm_value_t value;
		value.*member = get_immediate<T>();
		push(value);
	}


	// TODO: specialize for traps with integer division
	template <class T, class U>
	void binop(T op, U wasm_value_t::* member)
	{
		auto second = pop().*member;
		auto first = pop().*member;
		wasm_value_t result;
		result.*member = op(first, second);
		push(result);
	}

	template <class T, class U>
	void unop(T op, U wasm_value_t::* member)
	{
		auto value = pop();
		value.*member = op(value.*member);
		push(value);
	}

	template <class T>
	using unop_overload_type = T (*)(T);
	template <class T>
	using binop_overload_type = T (*)(T, T);

	template <class T>
	void add_op(T wasm_value_t::* m){ binop(std::plus<>{}, m); }
	template <class T>
	void sub_op(T wasm_value_t::* m){ binop(std::minus<>{}, m); }
	template <class T>
	void mul_op(T wasm_value_t::* m){ binop(std::multiplies<>{}, m); }
	template <class T>
	void div_op(T wasm_value_t::* m){ binop(wasm_divide<>{}, m); }
	template <class T>
	void rem_op(T wasm_value_t::* m){ binop(wasm_modulus<>{}, m); }
	
	template <class T>
	void and_op(T wasm_value_t::* m){ binop(std::bit_and<>{}, m); }
	template <class T>
	void or_op(T wasm_value_t::* m){ binop(std::bit_or<>{}, m); }
	template <class T>
	void xor_op(T wasm_value_t::* m){ binop(std::bit_xor<>{}, m); }
	template <class T>
	void shl_op(T wasm_value_t::* m){ binop(bit_lshift<>{}, m); }
	template <class T>
	void shr_op(T wasm_value_t::* m){ binop(bit_rshift<>{}, m); }
	template <class T>
	void rotl_op(T wasm_value_t::* m){ binop(bit_lrotate<>{}, m); }
	template <class T>
	void rotr_op(T wasm_value_t::* m){ binop(bit_rrotate<>{}, m); }
	
	template <class T>
	void clz_op(T wasm_value_t::* m){ unop(bit_clz<>{}, m); }
	template <class T>
	void ctz_op(T wasm_value_t::* m){ unop(bit_ctz<>{}, m); }
	template <class T>
	void popcnt_op(T wasm_value_t::* m){ unop(bit_popcnt<>{}, m); }
	template <class T>
	void eqz_op(T wasm_value_t::* m){ unop(std::logical_not<>{}, m); }
	template <class T>
	void eq_op(T wasm_value_t::* m){ binop(std::equal_to<>{}, m); }
	template <class T>
	void ne_op(T wasm_value_t::* m){ binop(std::not_equal_to<>{}, m); }
	template <class T>
	void lt_op(T wasm_value_t::* m){ binop(std::less<>{}, m); }
	template <class T>
	void gt_op(T wasm_value_t::* m){ binop(std::greater<>{}, m); }
	template <class T>
	void le_op(T wasm_value_t::* m){ binop(std::less_equal<>{}, m); }
	template <class T>
	void ge_op(T wasm_value_t::* m){ binop(std::greater_equal<>{}, m); }
	
	template <class T>
	void min_op(T wasm_value_t::* m){ binop((const T& (*)(const T&, const T&))std::min<T>, m); }
	template <class T>
	void max_op(T wasm_value_t::* m){ binop((const T& (*)(const T&, const T&))std::max<T>, m); }
	template <class T>
	void copysign_op(T wasm_value_t::* m){ binop((binop_overload_type<T>)std::copysign, m); }

	template <class T>
	void nearest_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::nearbyint, m); }
	template <class T>
	void sqrt_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::sqrt, m); }
	template <class T>
	void ceil_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::ceil, m); }
	template <class T>
	void abs_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::abs, m); }
	template <class T>
	void floor_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::floor, m); }
	template <class T>
	void trunc_op(T wasm_value_t::* m){ unop((unop_overload_type<T>)std::trunc, m); }
	template <class T>
	void neg_op(T wasm_value_t::* m){ unop(std::negate<>{}, m); }
	
	template <class T, class U>
	void trunc_int_op(T wasm_value_t::* m_int, U wasm_value_t::* m_float){ 
		static_assert(std::is_floating_point_v<U>);
		static_assert(std::is_integral_v<T>);
		wasm_trunc<T, U> trunc;
		wasm_value_t result;
		result.*m_int = trunc(pop().*m_float);
		push(result);
	}
	
	template <class T, class U>
	void convert_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{
		auto value = pop();
		wasm_value_t result;
		result.*m_to = value.*m_from;
		push(result);
	}
	
	template <class T, class U>
	void reinterpret_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{
		auto value = pop();
		wasm_value_t result;
		result.*m_to = bit_cast<U>(value.*m_from);
		push(result);
	}

	opcode_t fetch_opcode_incr() 
	{
		return call_stack.next_opcode();
	}
};

enum class StateChangeType {
	no_change,
	unary_op,
	binary_op,
	memory_write,
	memory_read,
	jump,
	block,
	call,
	indirect_call,
};

struct StateChange {
	StateChangeType change_type;
	struct FunctionCall {
		std::size_t function_index;
	};

	struct IndirectFunctionCall {
		std::size_t table;
		std::size_t table_index;
	};

	struct UnaryOp {
		WasmValueType argument_type;
		WasmValueType result_type;
		wasm_value_t (*operation)(wasm_value_t);
	};

	struct BinaryOp {
		WasmValueType first_argument_type;
		WasmValueType second_argument_type;
		WasmValueType result_type;
		wasm_value_t (*operation)(wasm_value_t, wasm_value_t);
	};


};



StateChange opcode_dispatch()
{
	// TODO: implement compile-time switch for computed goto with GCC
	using namespace wasm_opcode;
	bool done = false;
	auto opcode = fetch_opcode_incr();
	switch(opcode)
	{

	// BLOCK INSTRUCTIONS
	case BLOCK:			block_op(); 			break;
	case LOOP:			loop_op(); 			break;
	case BR:			br_op(); 			break;
	case BR_IF:			br_if_op(); 			break;
	case BR_TABLE:			br_table_op(); 			break;
	case IF:			if_op(); 			break;
	case ELSE:			else_op(); 			break;
	
	// special case:
	// end_op() indicates if the program is complete
	case END:			done = not end_op(); 		break;
	case RETURN:			return_op();			break;
	case UNREACHABLE:		trap("Unreachable.");		break;
	
	// BASIC INSTRUCTIONS
	case NOP:			/* no-op */			break;
	case DROP: 			pop();				break;
	case I32_CONST: 		push_immediate(u_32);		break;
	case I64_CONST: 		push_immediate(u_64);		break;
	case F32_CONST: 		push_immediate(f_32);		break;
	case F64_CONST: 		push_immediate(f_64);		break;
	case GET_LOCAL: 		get_local_op();			break;
	case SET_LOCAL: 		set_local_op();			break;
	case TEE_LOCAL: 		tee_local_op();			break;
	case GET_GLOBAL: 		get_global_op();		break;
	case SET_GLOBAL: 		set_global_op();		break;
	case SELECT: 			select_op();			break;
	case CALL: 			call_op();			break;
	case CALL_INDIRECT: 		call_indirect_op();		break;


	// I32 ARITHMETIC
	case I32_ADD: 			add_op(u_32); 			break;
	case I32_SUB:			sub_op(u_32); 			break;
	case I32_MUL:			mul_op(u_32); 			break;
	case I32_DIV_S:			div_op(s_32); 			break;
	case I32_DIV_U:			div_op(u_32); 			break;
	case I32_REM_S:			rem_op(s_32); 			break;
	case I32_REM_U:			rem_op(u_32); 			break;
	case I32_AND:			and_op(u_32); 			break;
	case I32_OR:			or_op(u_32);  			break;
	case I32_XOR:			xor_op(u_32); 			break;
	case I32_SHL:			shl_op(u_32); 			break;
	case I32_SHR_S:			shr_op(s_32); 			break;
	case I32_SHR_U:			shr_op(u_32); 			break;
	case I32_ROTL:			rotl_op(u_32); 			break;
	case I32_ROTR:			rotr_op(u_32); 			break;
	case I32_CLZ:			clz_op(u_32); 			break;
	case I32_CTZ:			ctz_op(u_32); 			break;
	case I32_POPCNT:		popcnt_op(u_32); 		break;
	case I32_EQZ:			eqz_op(u_32); 			break;

	// I32 COMPARISONS
	case I32_EQ:			eq_op(s_32);			break;
	case I32_NE: 			ne_op(s_32);			break;
	case I32_LT_S:			lt_op(s_32);			break;
	case I32_LT_U:			lt_op(u_32);			break;
	case I32_GT_S:			gt_op(s_32);			break;
	case I32_GT_U:			gt_op(u_32);			break;
	case I32_LE_S:			le_op(s_32);			break;
	case I32_LE_U:			le_op(u_32);			break;
	case I32_GE_S:			ge_op(s_32);			break;
	case I32_GE_U:			ge_op(u_32);			break;
	
	// I32 COMPARISONS
	case I32_WRAP:			convert_op(u_64, u_32);		break;
	case I32_TRUNC_F32_S:		trunc_int_op(s_32, f_32);	break;
	case I32_TRUNC_F32_U:		trunc_int_op(u_32, f_32);	break;
	case I32_TRUNC_F64_S:		trunc_int_op(s_32, f_64);	break;
	case I32_TRUNC_F64_U:		trunc_int_op(u_32, f_64);	break;
	case I32_REINTERPRET_F32:	reinterpret_op(f_32, u_32);	break;

	// I64 ARITHMETIC
	case I64_ADD: 			add_op(u_64); 			break;
	case I64_SUB:			sub_op(u_64); 			break;
	case I64_MUL:			mul_op(u_64); 			break;
	case I64_DIV_S:			div_op(s_64); 			break;
	case I64_DIV_U:			div_op(u_64); 			break;
	case I64_REM_S:			rem_op(s_64); 			break;
	case I64_REM_U:			rem_op(u_64); 			break;
	case I64_AND:			and_op(u_64); 			break;
	case I64_OR:			or_op(u_64);  			break;
	case I64_XOR:			xor_op(u_64); 			break;
	case I64_SHL:			shl_op(u_64); 			break;
	case I64_SHR_S:			shr_op(s_64); 			break;
	case I64_SHR_U:			shr_op(u_64); 			break;
	case I64_ROTL:			rotl_op(u_64); 			break;
	case I64_ROTR:			rotr_op(u_64); 			break;
	case I64_CLZ:			clz_op(u_64); 			break;
	case I64_CTZ:			ctz_op(u_64); 			break;
	case I64_POPCNT:		popcnt_op(u_64); 		break;
	case I64_EQZ:			eqz_op(u_64); 			break;
	
	// I64 COMPARISONS
	case I64_EQ:			eq_op(s_64);			break;
	case I64_NE: 			ne_op(s_64);			break;
	case I64_LT_S:			lt_op(s_64);			break;
	case I64_LT_U:			lt_op(u_64);			break;
	case I64_GT_S:			gt_op(s_64);			break;
	case I64_GT_U:			gt_op(u_64);			break;
	case I64_LE_S:			le_op(s_64);			break;
	case I64_LE_U:			le_op(u_64);			break;
	case I64_GE_S:			ge_op(s_64);			break;
	case I64_GE_U:			ge_op(u_64);			break;
	
	// I64 COMPARISONS
	case I64_EXTEND_S:		convert_op(s_32, s_64);		break;
	case I64_EXTEND_U:		convert_op(u_32, u_64);		break;
	case I64_TRUNC_F32_S:		trunc_int_op(s_64, f_32);	break;
	case I64_TRUNC_F32_U:		trunc_int_op(u_64, f_32);	break;
	case I64_TRUNC_F64_S:		trunc_int_op(s_64, f_64);	break;
	case I64_TRUNC_F64_U:		trunc_int_op(u_64, f_64);	break;
	case I64_REINTERPRET_F64:	reinterpret_op(f_32, u_32);	break;

	// F32 ARITHMETIC 
	case F32_ADD:			add_op(f_32);   		break;
	case F32_SUB:			sub_op(f_32);   		break;
	case F32_MUL:			mul_op(f_32);   		break;
	case F32_DIV:			div_op(f_32);   		break;
	case F32_SQRT:			sqrt_op(f_32);  		break;
	case F32_MIN:			min_op(f_32);   		break;
	case F32_MAX:			max_op(f_32);   		break;
	case F32_CEIL:			ceil_op(f_32);  		break;
	case F32_FLOOR:			floor_op(f_32); 		break;
	case F32_TRUNC:			trunc_op(f_32); 		break;
	case F32_NEAREST:		nearest_op(f_32); 		break;
	case F32_ABS:			abs_op(f_32); 			break;
	case F32_NEG:			neg_op(f_32); 			break;
	case F32_COPYSIGN:		copysign_op(f_32); 		break;
	
	// F32 COMPARISONS
	case F32_EQ:			eq_op(f_32);			break;
	case F32_NE: 			ne_op(f_32);			break;
	case F32_LT:			lt_op(f_32);			break;
	case F32_GT:			gt_op(f_32);			break;
	case F32_LE:			le_op(f_32);			break;
	case F32_GE:			ge_op(f_32);			break;
	
	// F32 CONVERSIONS
	case F32_DEMOTE:         	convert_op(f_64, f_32);		break;
	case F32_CONVERT_I32_S:       	convert_op(s_32, f_32);		break;
	case F32_CONVERT_I32_U:       	convert_op(u_32, f_32);		break;
	case F32_CONVERT_I64_S:       	convert_op(s_64, f_32);		break;
	case F32_CONVERT_I64_U:       	convert_op(u_64, f_32);		break;
	case F32_REINTERPRET_I32:     	reinterpret_op(u_32, f_32);	break;
	
	// F64 ARITHMETIC 
	case F64_ADD:			add_op(f_64); 			break;
	case F64_SUB:			sub_op(f_64); 			break;
	case F64_MUL:			mul_op(f_64); 			break;
	case F64_DIV:			div_op(f_64); 			break;
	case F64_SQRT:			sqrt_op(f_64); 			break;
	case F64_MIN:			min_op(f_64); 			break;
	case F64_MAX:			max_op(f_64); 			break;
	case F64_CEIL:			ceil_op(f_64); 			break;
	case F64_FLOOR:			floor_op(f_64); 		break;
	case F64_TRUNC:			trunc_op(f_64); 		break;
	case F64_NEAREST:		nearest_op(f_64); 		break;
	case F64_ABS:			abs_op(f_64); 			break;
	case F64_NEG:			neg_op(f_64); 			break;
	case F64_COPYSIGN:		copysign_op(f_64); 		break;
	
	// F64 COMPARISONS
	case F64_EQ:			eq_op(f_64);			break;
	case F64_NE: 			ne_op(f_64);			break;
	case F64_LT:			lt_op(f_64);			break;
	case F64_GT:			gt_op(f_64);			break;
	case F64_LE:			le_op(f_64);			break;
	case F64_GE:			ge_op(f_64);			break;
	
	// F64 CONVERSIONS
	case F64_PROMOTE:         	convert_op(f_32, f_64);		break;
	case F64_CONVERT_I32_S:       	convert_op(s_32, f_64);		break;
	case F64_CONVERT_I32_U:       	convert_op(u_32, f_64);		break;
	case F64_CONVERT_I64_S:       	convert_op(s_64, f_64);		break;
	case F64_CONVERT_I64_U:       	convert_op(u_64, f_64);		break;
	case F64_REINTERPRET_I64:     	reinterpret_op(u_64, f_64);	break;
	
	// LOADS AND STORES
	case I32_LOAD:			load(u_32); 			break;
	case I64_LOAD:			load(u_64); 			break;
	case F32_LOAD:			load(f_32); 			break;
	case F64_LOAD:			load(f_64); 			break;
	case I32_LOAD8_S:		narrow_load<1>(s_32);		break;
	case I32_LOAD8_U:		narrow_load<1>(u_32);		break;
	case I32_LOAD16_S:		narrow_load<2>(s_32);		break;
	case I32_LOAD16_U:		narrow_load<2>(u_32);		break;
	case I64_LOAD8_S:		narrow_load<1>(s_64);		break;
	case I64_LOAD8_U:		narrow_load<1>(u_64);		break;
	case I64_LOAD16_S:		narrow_load<2>(s_64);		break;
	case I64_LOAD16_U:		narrow_load<2>(u_64);		break;
	case I64_LOAD32_S:		narrow_load<4>(s_64);		break;
	case I64_LOAD32_U:		narrow_load<4>(u_64);		break;
	case I32_STORE:			store(u_32); 			break;
	case I64_STORE:			store(u_64); 			break;
	case F32_STORE:			store(f_32); 			break;
	case F64_STORE:			store(f_64); 			break;
	case I32_STORE8:		wrap_store<1>(u_32);		break;
	case I32_STORE16:		wrap_store<2>(u_32);		break;		
	case I64_STORE8:		wrap_store<1>(u_32);		break;		
	case I64_STORE16:		wrap_store<2>(u_32);		break;		
	case I64_STORE32:		wrap_store<4>(u_32);		break;		
	
	// LINEAR MEMORY MANAGEMENT
	case GROW_MEMORY:		grow_memory();			break;
	case CURRENT_MEMORY:		current_memory();		break;

	default:
		trap("Unkown instruction reached");
	}

	return not done;
}

#endif /* INTERPRETER_WASM_INTERPRETER2_H */
