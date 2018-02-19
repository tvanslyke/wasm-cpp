#ifndef INTERPRETER_WASM_INTERPRETER_H
#define INTERPRETER_WASM_INTERPRETER_H

#include "wasm_value.h"
#include "wasm_instruction.h"
#include "interpreter/wasm_call_stack.h"
#include "interpreter/wasm_control_flow_stack.h"
#include "interpreter/functional_ex.h"
#include "module/wasm_program_state.h"
#include "wasm_instruction.h"
#include "utilities/bit_cast.h"
#include "utilities/immediates.h"
#include <cmath>
#include <sstream>

struct wasm_runtime
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	struct trap_error: public std::runtime_error {
		template <class String>
		trap_error(const String& string):
			std::runtime_error(string)
		{
			
		}
		trap_error(opcode_t op):
			std::runtime_error(opcode_message(op))
		{
			
		}
		
		static std::string opcode_message(opcode_t op)
		{
			std::stringstream ss;
			ss << "Trap occurred while evaluating instruction " << std::hex << op << ".";
			return ss.str();
		}
	};
	wasm_runtime(wasm_program_state& state, wasm_call_stack& calls, wasm_control_flow_stack& control_flow, wasm_value_t* stack):
		program_state(state),
		call_stack(calls),
		control_flow_stack(control_flow),
		stack_pointer(stack)
	{
		assert(program_state.start_function.return_count() == 0);
		call_stack.push_frame(program_state.start_function);
		control_flow_stack.push_function(sp(), 0);
	}

	wasm_program_state& program_state;
	wasm_call_stack& call_stack;
	wasm_control_flow_stack& control_flow_stack;
	wasm_value_t* stack_pointer;

	// Frame stuff
	wasm_value_t*& sp(){ return stack_pointer; }
	const opcode_t* pc(){ return call_stack.code(); }
	wasm_value_t* locals() { return call_stack.locals(); }
	
	// module stuff
	template <class ImmediateType>
	ImmediateType get_immediate()
	{
		assert(pc());
		ImmediateType dest;
		const auto* instrs = pc();
		std::memcpy(&dest, instrs, sizeof(dest));
		auto jump_pos = instrs + opcode_width_of<ImmediateType>();
		call_stack.code_jump(jump_pos);
		return dest;
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
	{
		throw trap_error(message);
	}
	void trap()
	{
		throw trap_error(call_stack.code()[-1]);
	}
	void trapif(bool b){ if(b) trap(); }
	
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

	void push() { ++sp(); }

	void push(wasm_value_t v) { *sp()++ = v; }
	
	wasm_value_t pop() { return *(--sp()); }

	wasm_value_t top(std::size_t i = 0) { return *(sp() - std::ptrdiff_t(1 + i)); }

	void push_cf_frame(const opcode_t* lbl, std::size_t arity)
	{
		control_flow_stack.push_frame(sp(), lbl, arity);
	}

	void push_cf_frame_immediate_sig(const opcode_t* lbl)
	{
		auto sig = get_immediate<wasm_sint8_t>();
		push_cf_frame(lbl, sig != decltype(sig)(wasm_language_type::block));
	}
	
	void push_cf_frame_lookahead(std::size_t arity)
	{
		push_cf_frame(pc() + get_immediate<wasm_uint32_t>(), arity);
	}
	void push_cf_frame_lookahead_immediate_sig()
	{
		auto sig = get_immediate<wasm_sint8_t>();
		auto label = pc() + get_immediate<wasm_uint32_t>();
		push_cf_frame(label, sig != decltype(sig)(wasm_language_type::block));
	}

	auto pop_cf_frame()
	{
		return control_flow_stack.pop_frame();
	}

	/* INSTRUCTIONS */

	// control flow

	void block_op()
	{
		push_cf_frame_lookahead_immediate_sig();
	}

	void loop_op()
	{
		push_cf_frame_immediate_sig(pc());
	}
	
	void return_with_arity(wasm_value_t* return_pos, std::size_t return_arity)
	{
		sp() = std::copy(sp() - return_arity, sp(), return_pos);
	}
	void branch(std::size_t depth)
	{
		auto [stack_ptr, label, arity] = control_flow_stack.jump_index(depth);
		return_with_arity(stack_ptr, arity);
		call_stack.code_jump(label);
	}
	
	void branch_top()
	{
		auto [stack_ptr, label, arity] = control_flow_stack.jump_top();
		return_with_arity(stack_ptr, arity);
		call_stack.code_jump(label);
	}

	void br_op()
	{
		branch(get_immediate<wasm_uint32_t>());
	}
	
	void br_if_op()
	{
		if(pop().u32)
			br_op();
	}
	
	void br_table_op()
	{
		auto sz = get_immediate<wasm_uint32_t>();
		std::size_t idx = pop().u32;
		if(idx < sz) // default branch
			idx = sz;
		// go to the selected branch and branch to the stored depth
		skip_immediates<wasm_uint32_t>(idx);
		idx = get_immediate<wasm_uint32_t>();
		branch(idx);
	}
	
	void if_op()
	{
		push_cf_frame_lookahead_immediate_sig();
	}
	
	void else_op()
	{
		[[maybe_unused]]
		auto [stack_ptr, label, arity] = pop_cf_frame();
		(void)stack_ptr;
		(void)label;
		push_cf_frame_lookahead(arity);
		branch_top();
	}
	
	bool end_op()
	{
		auto [stack_ptr, label, arity] = pop_cf_frame();
		if(not label) // end of a function
		{
			return_with_arity(stack_ptr, arity);
			assert(arity == 0);
			call_stack.fast_pop_frame();
			return call_stack.code() == nullptr;
		}
		return false;
	}
	
	void return_op()
	{
		call_stack.fast_pop_frame();
		// sentinal stack frame at the bottom has NULL program counter to 
		// indicate program completion.
		assert(pc());
		auto [stack_ptr, ret_label_sentinal, arity] = control_flow_stack.pop_function();
		assert(ret_label_sentinal == nullptr);
		return_with_arity(stack_ptr, arity);
	}

	// parametric instructions
	void get_local()
	{
		auto idx = get_immediate<wasm_uint32_t>();
		*sp()++ = locals()[idx];
	}
	void tee_local()
	{
		auto idx = get_immediate<wasm_uint32_t>();
		locals()[idx] = *sp();
	}
	void set_local()
	{
		tee_local();
		pop();
	}

	void get_global()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		push(program_state.const_global_at(idx));
	}
	
	void set_global()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		program_state.global_at(idx) = pop();
	}

	void push_frame(const wasm_function& func)
	{
		control_flow_stack.push_function(sp(), func.return_count());
		call_stack.push_frame(func);
	}

	void call()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		const auto& func = program_state.function_at(idx);
		push_frame(func);
	}
	
	void call_indirect()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		const auto& func = program_state.table_function_at(idx);
		push_frame(func);
	}

	void select()
	{
		wasm_uint32_t pred = pop().u32;
		wasm_value_t other = pop();
		if(pred)
			*sp() = other;
	}

	template <class T>
	void load(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		if(not (program_state.const_memory_at(0).load(address, offset, *sp(), member)))
			trap_bad_memory_access(address, offset);
		push();
	}

	template <std::size_t Sz, class T>
	void narrow_load(T wasm_value_t::* member)
	{
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		if(not (program_state.const_memory_at(0).narrow_load<Sz>(address, offset, *sp(), member)))
			trap_bad_memory_access(address, offset);
		push();
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
		sp()->s32 = program_state.memory_at(0).grow_memory(v);
		push();
	}

	void current_memory() 
	{
		sp()->u32 = program_state.memory_at(0).current_memory();
		push();
	}

	template <class T>
	void push_immediate(T wasm_value_t::* member)
	{
		(sp()++)->*member = get_immediate<T>();
	}


	// TODO: specialize for traps with integer division
	template <class T, class U>
	void binop(T op, U wasm_value_t::* member)
	{
		auto tmp = pop();
		*sp().*member = op(top().*member, tmp.*member);
	}
	template <class T, class U>
	void unop(T op, U wasm_value_t::* member)
	{
		*sp().*member = op(top().*member);
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
	void div_op(T wasm_value_t::* m){ binop(std::divides<>{}, m); }
	template <class T>
	void rem_op(T wasm_value_t::* m){ binop(std::modulus<>{}, m); }
	
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
	void convert_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{ sp()->*m_to = top().*m_from; }
	
	template <class T, class U>
	void reinterpret_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{ sp()->*m_to = bit_cast<U>(top().*m_from); }

	opcode_t fetch_opcode_incr() 
	{
		assert(call_stack.code());
		auto oc = *(call_stack.code());
		call_stack.code_next();
		return oc;
	}
};




bool wasm_runtime::eval()
{
	using namespace wasm_opcode;
	switch(fetch_opcode_incr())
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
	case END:
		if(end_op())
			return false;
		break;
	case RETURN:			return_op();			break;
	case UNREACHABLE:		trap("Unreachable.");		break;
	
	// BASIC INSTRUCTIONS
	case NOP:			/* no-op */			break;
	case DROP: 			pop();				break;
	case I32_CONST: 		push_immediate(u_32);		break;
	case I64_CONST: 		push_immediate(u_64);		break;
	case F32_CONST: 		push_immediate(f_32);		break;
	case F64_CONST: 		push_immediate(f_64);		break;
	case GET_LOCAL: 		get_local();			break;
	case SET_LOCAL: 		set_local();			break;
	case TEE_LOCAL: 		tee_local();			break;
	case GET_GLOBAL: 		get_global();			break;
	case SET_GLOBAL: 		set_global();			break;
	case SELECT: 			select();			break;
	case CALL: 			call();				break;
	case CALL_INDIRECT: 		call_indirect();		break;


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
	case I32_TRUNC_F32_S:		convert_op(f_32, s_32);		break;
	case I32_TRUNC_F32_U:		convert_op(f_32, u_32);		break;
	case I32_TRUNC_F64_S:		convert_op(f_64, s_32);		break;
	case I32_TRUNC_F64_U:		convert_op(f_64, u_32);		break;
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
	case I64_TRUNC_F32_S:		convert_op(f_32, s_64);		break;
	case I64_TRUNC_F32_U:		convert_op(f_32, u_64);		break;
	case I64_TRUNC_F64_S:		convert_op(f_64, s_64);		break;
	case I64_TRUNC_F64_U:		convert_op(f_64, u_64);		break;
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

	return true;
}

#endif /* INTERPRETER_WASM_INTERPRETER_H */
