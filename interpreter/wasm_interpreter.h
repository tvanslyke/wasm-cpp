#ifndef WASM_INTERPRETER_H
#define WASM_INTERPRETER_H

#include "wasm_value.h"
#include "wasm_instructions.h"
#include "utilities/bit_cast.h"

struct wasm_control_flow_frame;
struct wasm_frame
{
	const wasm_frame* ret;
	const wasm_instruction* program_counter;
	wasm_control_flow_frame* lp;
	const std::size_t locals_count;
	wasm_value_t[1] locals;
	
	wasm_frame(wasm_frame* prev, const wasm_function& func):
		ret{prev}, program_counter{wasm_function.code.c_str()},
		lp{prev->lp}, locals_count{func.locals_count} 
	{
		
	}
	std::ptrdiff_t get_next_offset() 
	{
		auto total_bytes = offsetof(wasm_frame, locals);
		total_bytes += (nlocals + (nlocals == 0)) * sizeof(wasm_value_t);
		auto align_err = (total_bytes % alignof(wasm_frame)) > 0;
		// correct alignment
		if(align_err)
			total_bytes += (alignof(wasm_frame) - align_err);
		return total_bytes;
	}
};

struct wasm_control_flow_frame
{
	wasm_control_flow_frame(const wasm_value_t *sp, wasm_uint32_t sig, const wasm_instruction* lbl = nullptr):
		label{lbl}, signature{sig}, stack_pointer{sp}
	{
		
	}
	const wasm_value_t* stack_pointer;
	wasm_uint32_t signature;
	const wasm_instruction* label;
};

struct wasm_runtime
{
	wasm_frame* frame_pointer;
	wasm_value_t* stack_pointer;
	wasm_module* main_module;
	

	// Frame stuff
	wasm_frame*& ret(){ return fp->ret; }
	wasm_frame*& fp(){ return frame_pointerp; }
	wasm_value_t*& sp(){ return stack_pointer; }
	wasm_instruction*& lp(){ return fp->lp; }
	wasm_instruction*& pc(){ return fp->program_counter; }
	wasm_value_t* locals() { return fp->locals; }
	
	// module stuff
	
	
	template <class ImmediateType>
	ImmediateType get_immediate()
	{
		using instr_t = std::underlying_type_t<wasm_instruction>;
		ImmediateType dest;
		instr_t* instrs = pc();
		std::memcpy(&dest, instrs, sizeof(ImmediateType));
		pc() += sizeof(ImmediateType) / sizeof(instr_t) + (sizeof(ImmediateType) % sizeof(instr_t));
		return dest;
	}

	template <class ImmediatesType>
	void skip_immediates(std::size_t count)
	{
		pc() += sizeof(ImmediateType);
	}


	bool eval();
	void trap();
	void trapif(bool b){ if(b) trap(); }
	void push() { ++sp(); }
	wasm_value_t& push_ref() { return *sp()++; }
	void push(wasm_value_t v) { *sp()++ = v; }
	
	wasm_value_t pop() { return *(--sp()); }
	wasm_value_t top(std::size_t i = 0) { return *(sp() - std::ptrdiff_t(1 + i)); }

	void push_cf_frame(const wasm_instruction* lbl, wasm_sint32_t sig)
	{
		*lp()++ = wasm_control_flow_frame(sp(), sig, lbl);
	}

	void push_cf_frame_immediate_sig(const wasm_instruction* lbl)
	{
		push_cf_frame(lbl, get_immediate<wasm_uint8_t>());
	}
	
	void push_cf_frame_lookahead(wasm_sint32_t sig)
	{
		push_cf_frame(pc() + get_immediate<wasm_uint32_t>(), sig);
	}
	void push_cf_frame_lookahead_immediate_sig()
	{
		auto label = pc() + get_immediate<wasm_uint32_t>();
		auto sig = get_immediate<wasm_uint8_t>();
		push_cf_frame(label, sig);
	}

	wasm_control_flow_frame pop_cf_frame()
	{
		*(--lp());
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
	
	void branch(std::size_t depth)
	{
		branch((lp() - depth) - 1);
	}
	void branch(const wasm_control_flow_frame* cf_frame)
	{
		sp() = cf_frame->stack_pointer;
		pc() = cf_frame->label;
		lp() = cf_frame();
		--lp();
	}
	
	void br_op()
	{
		branch(get_immediate<wasm_uint32_t>());
	}
	
	void br_if_op()
	{
		if(pop().u32)
			branch(get_immediate<wasm_uint32_t>());
	}
	
	void br_table_op()
	{
		auto sz = get_immediate<wasm_uint32_t>();
		const auto* pc_save = pc();
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
		lp()[-1].label = pc();
		push_cf_frame_lookahead(pop_cf_frame().signature);
		branch(0);
	}
	
	void end_op()
	{
		pop_cf_frame();
	}
	
	void return_op()
	{
		auto* frame = fp();
		frame->wasm_frame();
		fp() = ret();
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
		assert(main_module->globals.size() > idx);
		push(*(main_module->globals[idx]));
	}
	
	void set_global()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		assert(main_module->globals.size() > idx);
		assert(main_module->global_mutabilities.size() > idx);
		main_module->globals[idx] = pop();
	}

	void push_frame(const wasm_function& func)
	{
		auto* frame = fp();
		frame
	}

	void call()
	{
		wasm_uint32_t idx = get_immediate<wasm_uint32_t>();
		assert(main_module->functions.size() > idx);
		const auto& func = *(main_module->functions[idx]);
		// TODO: check for stack overflow 
		auto* new_frame = fp() + fp()->get_next_offset();
		new(new_frame) wasm_frame(fp(), func);
		fp() = new_frame;
	}
	
	void call_indirect()
	{
		// TODO: implement
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
		assert(main_module->memories.size());
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		if(not main_module->memories[0].load(address, offset, *sp(), member))
			trap();
		push();
	}

	template <std::size_t Sz, class T>
	void narrow_load(T wasm_value_t::* member)
	{
		assert(main_module->memories.size());
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto address = pop().u32;
		if(not main_module->memories[0].narrow_load<Sz>(address, offset, *sp(), member))
			trap();
		push();
	}

	template <class T>
	void store(T wasm_value_t::* member)
	{
		assert(main_module->memories.size());
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto value = pop();
		auto address = pop().u32;
		if(not main_module->memories[0].store(address, offset, value, member))
			trap();
	}

	template <std::size_t Sz, class T>
	void wrap_store(T wasm_value_t::* member)
	{
		assert(main_module->memories.size());
		[[maybe_unused]] auto alignment_hint = get_immediate<wasm_ptr_t>();
		auto offset = get_immediate<wasm_uint32_t>();
		auto value = pop();
		auto address = pop().u32;
		if(not main_module->memories[0].wrap_store<Sz>(address, offset, value, member))
			trap();
	}

	void grow_memory()
	{
		assert(main_module->memories.size());
		auto v = pop().u32;
		sp()->i32 = (main_module->memories[0].grow_memory(v));
		push();
	}

	void current_memory() 
	{
		assert(main_module->memories.size());
		sp()->u32 = main_module->memories[0].current_memory();
		push();
	}

	template <class T>
	void push_immediate(T wasm_value::* member)
	{
		(sp()++)->*member = get_immediate<T>();
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
	
	template <class T, class U>
	void reinterpret_op(T wasm_value_t::* m_from, U wasm_value_t::* m_to)
	{ sp().*m_to = bit_cast<U>(top().*m_from); }
};




bool wasm_runtime::eval()
{
	switch(*pc()++)
	{

	// BLOCK INSTRUCTIONS
	case BLOCK:			block_op(); 			break;
	case LOOP			loop_op(); 			break;
	case BR				br_op(); 			break;
	case BR_IF			br_if_op(); 			break;
	case BR_TABLE			br_table_op(); 			break;
	case IF				if_op(); 			break;
	case ELSE			else_op(); 			break;
	case END			end_op(); 			break;
	case RETURN			return_op(); 			break;
	case UNREACHABLE		trap(); 			break;
	
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
	case I32_DIV_S:			div_op(i_32); 			break;
	case I32_DIV_U:			div_op(u_32); 			break;
	case I32_REM_S:			rem_op(i_32); 			break;
	case I32_REM_U:			rem_op(u_32); 			break;
	case I32_AND:			and_op(u_32); 			break;
	case I32_OR:			or_op(u_32);  			break;
	case I32_XOR:			xor_op(u_32); 			break;
	case I32_SHL:			shl_op(u_32); 			break;
	case I32_SHR_S:			shr_op(i_32); 			break;
	case I32_SHR_U:			shr_op(u_32); 			break;
	case I32_ROTL:			rotl_op(u_32); 			break;
	case I32_ROTR:			rotr_op(u_32); 			break;
	case I32_CLZ:			clz_op(u_32); 			break;
	case I32_CTZ:			ctz_op(u_32); 			break;
	case I32_POPCNT:		popcnt_op(u_32); 		break;
	case I32_EQZ:			eqz_op(u_32); 			break;

	// I32 COMPARISONS
	case I32_EQ:			eq_op(i_32);			break;
	case I32_NE: 			ne_op(i_32);			break;
	case I32_LT_S:			lt_op(i_32);			break;
	case I32_LT_U:			lt_op(u_32);			break;
	case I32_GT_S:			gt_op(i_32);			break;
	case I32_GT_U:			gt_op(u_32);			break;
	case I32_LE_S:			le_op(i_32);			break;
	case I32_LE_U:			le_op(u_32);			break;
	case I32_GE_S:			ge_op(i_32);			break;
	case I32_GE_U:			ge_op(u_32);			break;
	
	// I32 COMPARISONS
	case I32_WRAP			convert_op(u_64, u_32);		break;
	case I32_TRUNC_F32_S		convert_op(f_32, i_32);		break;
	case I32_TRUNC_F32_U		convert_op(f_32, u_32);		break;
	case I32_TRUNC_F64_S		convert_op(f_64, i_32);		break;
	case I32_TRUNC_F64_U		convert_op(f_64, u_32);		break;
	case I32_REINTERPRET_F32	reinterpret_op(f_32, u_32);	break;

	// I64 ARITHMETIC
	case I64_ADD: 			add_op(u_64); 			break;
	case I64_SUB:			sub_op(u_64); 			break;
	case I64_MUL:			mul_op(u_64); 			break;
	case I64_DIV_S:			div_op(i_64); 			break;
	case I64_DIV_U:			div_op(u_64); 			break;
	case I64_REM_S:			rem_op(i_64); 			break;
	case I64_REM_U:			rem_op(u_64); 			break;
	case I64_AND:			and_op(u_64); 			break;
	case I64_OR:			or_op(u_64);  			break;
	case I64_XOR:			xor_op(u_64); 			break;
	case I64_SHL:			shl_op(u_64); 			break;
	case I64_SHR_S:			shr_op(i_64); 			break;
	case I64_SHR_U:			shr_op(u_64); 			break;
	case I64_ROTL:			rotl_op(u_64); 			break;
	case I64_ROTR:			rotr_op(u_64); 			break;
	case I64_CLZ:			clz_op(u_64); 			break;
	case I64_CTZ:			ctz_op(u_64); 			break;
	case I64_POPCNT:		popcnt_op(u_64); 		break;
	case I64_EQZ:			eqz_op(u_64); 			break;
	
	// I64 COMPARISONS
	case I64_EQ:			eq_op(i_64);			break;
	case I64_NE: 			ne_op(i_64);			break;
	case I64_LT_S:			lt_op(i_64);			break;
	case I64_LT_U:			lt_op(u_64);			break;
	case I64_GT_S:			gt_op(i_64);			break;
	case I64_GT_U:			gt_op(u_64);			break;
	case I64_LE_S:			le_op(i_64);			break;
	case I64_LE_U:			le_op(u_64);			break;
	case I64_GE_S:			ge_op(i_64);			break;
	case I64_GE_U:			ge_op(u_64);			break;
	
	// I64 COMPARISONS
	case I64_EXTEND_S		convert_op(i_32, i_64);		break;
	case I64_EXTEND_U		convert_op(u_32, u_64);		break;
	case I64_TRUNC_F32_S		convert_op(f_32, i_64);		break;
	case I64_TRUNC_F32_U		convert_op(f_32, u_64);		break;
	case I64_TRUNC_F64_S		convert_op(f_64, i_64);		break;
	case I64_TRUNC_F64_U		convert_op(f_64, u_64);		break;
	case I64_REINTERPRET_F64	reinterpret_op(f_32, u_32);	break;

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
	case F32_DEMOTE         	convert_op(f_64, f_32);		break;
	case F32_CONVERT_I32_S       	convert_op(i_32, f_32);		break;
	case F32_CONVERT_I32_U       	convert_op(u_32, f_32);		break;
	case F32_CONVERT_I64_S       	convert_op(i_64, f_32);		break;
	case F32_CONVERT_I64_U       	convert_op(u_64, f_32);		break;
	case F32_REINTERPRET_I32     	reinterpret_op(u_32, f_32);	break;
	
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
	case F64_PROMOTE         	convert_op(f_32, f_64);		break;
	case F64_CONVERT_I32_S       	convert_op(i_32, f_64);		break;
	case F64_CONVERT_I32_U       	convert_op(u_32, f_64);		break;
	case F64_CONVERT_I64_S       	convert_op(i_64, f_64);		break;
	case F64_CONVERT_I64_U       	convert_op(u_64, f_64);		break;
	case F64_REINTERPRET_I64     	reinterpret_op(u_64, f_64);	break;
	
	// LOADS AND STORES
	case I32_LOAD:			load(u_32); 			break;
	case I64_LOAD:			load(u_64); 			break;
	case F32_LOAD:			load(f_32); 			break;
	case F64_LOAD:			load(f_64); 			break;
	case I32_LOAD8_S:		narrow_load<1>(i_32);		break;
	case I32_LOAD8_U:		narrow_load<1>(u_32);		break;
	case I32_LOAD16_S:		narrow_load<2>(i_32);		break;
	case I32_LOAD16_U:		narrow_load<2>(u_32);		break;
	case I64_LOAD8_S:		narrow_load<1>(i_64);		break;
	case I64_LOAD8_U:		narrow_load<1>(u_64);		break;
	case I64_LOAD16_S:		narrow_load<2>(i_64);		break;
	case I64_LOAD16_U:		narrow_load<2>(u_64);		break;
	case I64_LOAD32_S:		narrow_load<4>(i_64);		break;
	case I64_LOAD32_U:		narrow_load<4>(u_64);		break;
	case I32_STORE:			store(u_32); 			break;
	case I64_STORE:			store(u_64); 			break;
	case F32_STORE:			store(f_32); 			break;
	case F64_STORE:			store(f_64); 			break;
	case I32_STORE8:		wrap_store<1>(u_32)		break;
	case I32_STORE16:		wrap_store<2>(u_32)		break;		
	case I64_STORE8:		wrap_store<1>(u_32)		break;		
	case I64_STORE16:		wrap_store<2>(u_32)		break;		
	case I64_STORE32:		wrap_store<4>(u_32)		break;		
	
	// LINEAR MEMORY MANAGEMENT
	case GROW_MEMORY:		grow_memory();			break;
	case CURRENT_MEMORY:		current_memory();		break;

	default:
		trap();
	}

	return true;
}

#endif /* WASM_INTERPRETER_H */
