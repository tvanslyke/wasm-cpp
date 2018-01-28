#ifndef WASM_INTERPRETER_H
#define WASM_INTERPRETER_H

#include "wasm_value.h"
#include "wasm_instructions.h"

struct wasm_frame
{
	wasm_frame* ret;
	wasm_instruction* pc;
	wasm_value_t* sp;
	wasm_instruction* lp;
	std::size_t nlocals;
	wasm_value_t[1] locals;
};

struct wasm_runtime
{
	
	wasm_frame* fp;
	wasm_frame*& ret(){ return fp->ret; }
	wasm_instruction*& lp(){ return fp->lp; }
	wasm_value_t*& sp(){ return fp->sp; }
	wasm_instruction*& pc(){ return fp->pc; }
	wasm_value_t* locals() { return fp->locals; }
	
	template <class ImmediateType>
	ImmediateType get_immediate()
	{
		using instr_t = std::underlying_type_t<wasm_instruction>;
		ImmediateType dest;
		instr_t* instrs = pc;
		std::memcpy(&dest, pc, sizeof(ImmediateType));
		pc += sizeof(ImmediateType) / sizeof(instr_t) + (sizeof(ImmediateType) % sizeof(instr_t));
		return dest;
	}

	bool eval();
	void trap();
	void trapif(bool b){ if(b) trap(); }
	void push() { ++sp(); }
	void push(wasm_value_t v) { *sp()++ = v; }
	
	wasm_value_t pop() { return *sp(); }
	wasm_value_t top(std::size_t i = 0) { return *(sp() - std::ptrdiff_t(1 + i)); }

	/* INSTRUCTIONS */
	// control flow
	void block();
	// parametric instructions
	void select();
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
	void select()
	{
		wasm_uint32_t pred = pop().u32;
		wasm_value_t other = pop();
		if(pred)
			*sp() = other;
	}
	// TODO: specialize for traps with integer division
	template <class T, class U>
	void binop(T op, U wasm_value_t::* member)
	{
		auto tmp = pop();
		*sp().*member = op(top().*member, tmp);
	}
	template <class T>
	void unop(T op, U wasm_value_t::* member)
	{
		*sp().*member = op(top().*member);
	}
	
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
	void eq_op(T wasm_value_t::* m){ unop(std::equal_to<>{}, m); }
	template <class T>
	void ne_op(T wasm_value_t::* m){ unop(std::not_equal_to<>{}, m); }
	template <class T>
	void lt_op(T wasm_value_t::* m){ unop(std::less<>{}, m); }
	template <class T>
	void gt_op(T wasm_value_t::* m){ unop(std::greater<>{}, m); }
	template <class T>
	void le_op(T wasm_value_t::* m){ unop(std::less_equal<>{}, m); }
	template <class T>
	void ge_op(T wasm_value_t::* m){ unop(std::greater_equal<>{}, m); }
	
	template <class T>
	void min_op(T wasm_value_t::* m){ binop(std::min, m); }
	template <class T>
	void max_op(T wasm_value_t::* m){ binop(std::max, m); }
	template <class T>
	void nearest_op(T wasm_value_t::* m){ binop(std::copysign, m); }
	template <class T>
	void sqrt_op(T wasm_value_t::* m){ unop(std::sqrt, m); }
	template <class T>
	void ceil_op(T wasm_value_t::* m){ unop(std::ceil, m); }
	template <class T>
	void nearest_op(T wasm_value_t::* m){ unop(std::abs, m); }
	template <class T>
	void neg_op(T wasm_value_t::* m){ unop(std::negate<>, m); }
	
	template <class T, class U>
	void convert_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{ sp().*m_to = top().*m_from; }
};




bool wasm_runtime::eval()
{
	switch(*(fp->pc))
	{
	case NOP: break;
	case DROP: pop(); break;
	case GET_LOCAL: break;
	
	case I32_CONST: [[fallthrough]]
	case I64_CONST:	[[fallthrough]]
	case F32_CONST:	[[fallthrough]]
	case F64_CONST: push(); break;		

	// I32 ARITHMETIC
	case I32_ADD: 		add_op(u_32); 		break; 
	case I32_SUB:		sub_op(u_32); 		break;
	case I32_MUL:		mul_op(u_32); 		break;
	case I32_DIV_S:		div_op(i_32); 		break;
	case I32_DIV_U:		div_op(u_32); 		break;
	case I32_REM_S:		rem_op(i_32); 		break;
	case I32_REM_U:		rem_op(u_32); 		break;
	case I32_AND:		and_op(u_32); 		break;
	case I32_OR:		or_op(u_32);  		break;
	case I32_XOR:		xor_op(u_32); 		break; 
	case I32_SHL:		shl_op(u_32); 		break;
	case I32_SHR_S:		shr_op(i_32); 		break;
	case I32_SHR_U:		shr_op(u_32); 		break;
	case I32_ROTL:		rotl_op(u_32); 		break;
	case I32_ROTR:		rotr_op(u_32); 		break;
	case I32_CLZ:		clz_op(u_32); 		break;
	case I32_CTZ:		ctz_op(u_32); 		break;
	case I32_POPCNT:	popcnt_op(u_32); 	break;
	case I32_EQZ:		eqz_op(u_32); 		break;
	
	// I64 ARITHMETIC
	case I64_ADD: 		add_op(u_64); 		break; 
	case I64_SUB:		sub_op(u_64); 		break;
	case I64_MUL:		mul_op(u_64); 		break;
	case I64_DIV_S:		div_op(i_64); 		break;
	case I64_DIV_U:		div_op(u_64); 		break;
	case I64_REM_S:		rem_op(i_64); 		break;
	case I64_REM_U:		rem_op(u_64); 		break;
	case I64_AND:		and_op(u_64); 		break;
	case I64_OR:		or_op(u_64);  		break;
	case I64_XOR:		xor_op(u_64); 		break; 
	case I64_SHL:		shl_op(u_64); 		break;
	case I64_SHR_S:		shr_op(i_64); 		break;
	case I64_SHR_U:		shr_op(u_64); 		break;
	case I64_ROTL:		rotl_op(u_64); 		break;
	case I64_ROTR:		rotr_op(u_64); 		break;
	case I64_CLZ:		clz_op(u_64); 		break;
	case I64_CTZ:		ctz_op(u_64); 		break;
	case I64_POPCNT:	popcnt_op(u_64); 	break;
	case I64_EQZ:		eqz_op(u_64); 		break;

	// F32 ARITHMETIC 
	case F32_ADD:		add_op(f_32); break;
	case F32_SUB:		sub_op(f_32); break;
	case F32_MUL:		mul_op(f_32); break;
	case F32_DIV:		div_op(f_32); break;
	case F32_SQRT:		sqrt_op(f_32); break;
	case F32_MIN:		min_op(f_32); break;
	case F32_MAX:		max_op(f_32); break;
	case F32_CEIL:		ceil_op(f_32); break;
	case F32_FLOOR:		floor_op(f_32); break;
	case F32_TRUNC:		trunc_op(f_32); break;
	case F32_NEAREST:	nearest_op(f_32); break;
	case F32_ABS:		abs_op(f_32); break;
	case F32_NEG:		neg_op(f_32); break;
	case F32_COPYSIGN:	copysign_op(f_32); break;
	
	// F64 ARITHMETIC 
	case F64_ADD:		add_op(f_64); break;
	case F64_SUB:		sub_op(f_64); break;
	case F64_MUL:		mul_op(f_64); break;
	case F64_DIV:		div_op(f_64); break;
	case F64_SQRT:		sqrt_op(f_64); break;
	case F64_MIN:		min_op(f_64); break;
	case F64_MAX:		max_op(f_64); break;
	case F64_CEIL:		ceil_op(f_64); break;
	case F64_FLOOR:		floor_op(f_64); break;
	case F64_TRUNC:		trunc_op(f_64); break;
	case F64_NEAREST:	nearest_op(f_64); break;
	case F64_ABS:		abs_op(f_64); break;
	case F64_NEG:		neg_op(f_64); break;
	case F64_COPYSIGN:	copysign_op(f_64); break;
	
	
	


	default:
		trap();
	}
	++(fp->pc);
	return true;
}


#endif /* WASM_INTERPRETER_H */
