#ifndef WASM_INSTRUCTION_H
#define WASM_INSTRUCTION_H
#include <cstddef>
#include <cstdint>
#include <array>


namespace wasm {
namespace opc {

enum class Opcode: wasm_ubyte_t
{
	
	// BLOCK INSTRUCTIONS
	BLOCK			= 0x02u,
	LOOP			= 0x03u,
	BR			= 0x0cu,
	BR_IF			= 0x0du,
	BR_TABLE		= 0x0eu,
	IF			= 0x04u,
	ELSE			= 0x05u,
	END			= 0x0bu,
	RETURN			= 0x0fu,
	UNREACHABLE		= 0x00u,

	// BASIC INSTRUCTIONS
	NOP 			= 0x01u,
	DROP 			= 0x1au,
	I32_CONST 		= 0x41u,
	I64_CONST 		= 0x42u,
	F32_CONST 		= 0x43u,
	F64_CONST 		= 0x44u,
	GET_LOCAL 		= 0x20u,
	SET_LOCAL 		= 0x21u,
	TEE_LOCAL 		= 0x22u,
	GET_GLOBAL 		= 0x23u,
	SET_GLOBAL 		= 0x24u,
	SELECT 			= 0x1bu,
	CALL 			= 0x10u,
	CALL_INDIRECT 		= 0x11u,
	
	// INTEGER ARITHMETIC INSTRUCTIONS
	// int32
	I32_ADD			= 0x6au,
	I32_SUB			= 0x6bu,
	I32_MUL			= 0x6cu,
	I32_DIV_S		= 0x6du,
	I32_DIV_U		= 0x6eu,
	I32_REM_S		= 0x6fu,
	I32_REM_U		= 0x70u,
	I32_AND			= 0x71u,
	I32_OR			= 0x72u,
	I32_XOR			= 0x73u,
	I32_SHL			= 0x74u,
	I32_SHR_S		= 0x75u,
	I32_SHR_U		= 0x76u,
	I32_ROTL		= 0x77u,
	I32_ROTR		= 0x78u,
	I32_CLZ			= 0x67u,
	I32_CTZ			= 0x68u,
	I32_POPCNT		= 0x69u,
	I32_EQZ			= 0x45u,
	// int64
	I64_ADD			= 0x7cu,
	I64_SUB			= 0x7du,
	I64_MUL			= 0x7eu,
	I64_DIV_S		= 0x7fu,
	I64_DIV_U		= 0x80u,
	I64_REM_S		= 0x81u,
	I64_REM_U		= 0x82u,
	I64_AND			= 0x83u,
	I64_OR			= 0x84u,
	I64_XOR			= 0x85u,
	I64_SHL			= 0x86u,
	I64_SHR_S		= 0x87u,
	I64_SHR_U		= 0x88u,
	I64_ROTL		= 0x89u,
	I64_ROTR		= 0x8au,
	I64_CLZ			= 0x79u,
	I64_CTZ			= 0x7au,
	I64_POPCNT		= 0x7bu,
	I64_EQZ			= 0x50u,

	// FLOATING POINT ARITHMETIC INSTRUCTIONS
	// float32
	F32_ADD			= 0x92u,
	F32_SUB	    		= 0x93u,
	F32_MUL	    		= 0x94u,
	F32_DIV	    		= 0x95u,
	F32_SQRT    		= 0x91u,
	F32_MIN	    		= 0x96u,
	F32_MAX	    		= 0x97u,
	F32_CEIL    		= 0x8du,
	F32_FLOOR   		= 0x8eu,
	F32_TRUNC   		= 0x8fu,
	F32_NEAREST 		= 0x90u,
	F32_ABS	    		= 0x8bu,
	F32_NEG	    		= 0x8cu,
	F32_COPYSIGN		= 0x98u,
	// float64
	F64_ADD	    		= 0xa0u,
	F64_SUB	    		= 0xa1u,
	F64_MUL	    		= 0xa2u,
	F64_DIV	    		= 0xa3u,
	F64_SQRT    		= 0x9fu,
	F64_MIN	    		= 0xa4u,
	F64_MAX	    		= 0xa5u,
	F64_CEIL    		= 0x9bu,
	F64_FLOOR   		= 0x9cu,
	F64_TRUNC   		= 0x9du,
	F64_NEAREST 		= 0x9eu,
	F64_ABS	    		= 0x99u,
	F64_NEG	    		= 0x9au,
	F64_COPYSIGN		= 0xa6u,

	// INTEGER COMPARISON INSTRUCTIONS
	// int32
	I32_EQ  		= 0x46u,
	I32_NE  		= 0x47u,
	I32_LT_S		= 0x48u,
	I32_LT_U		= 0x49u,
	I32_GT_S		= 0x4au,
	I32_GT_U		= 0x4bu,
	I32_LE_S		= 0x4cu,
	I32_LE_U		= 0x4du,
	I32_GE_S		= 0x4eu,
	I32_GE_U		= 0x4fu,
	// int64
	I64_EQ  		= 0x51u,
	I64_NE  		= 0x52u,
	I64_LT_S		= 0x53u,
	I64_LT_U		= 0x54u,
	I64_GT_S		= 0x55u,
	I64_GT_U		= 0x56u,
	I64_LE_S		= 0x57u,
	I64_LE_U		= 0x58u,
	I64_GE_S		= 0x59u,
	I64_GE_U		= 0x5au,
	
	// FLOATING POINT COMPARISON INSTRUCTIONS
	// float32
	F32_EQ  		= 0x5bu,
	F32_NE  		= 0x5cu,
	F32_LT			= 0x5du,
	F32_GT			= 0x5eu,
	F32_LE			= 0x5fu,
	F32_GE			= 0x60u,
	// float64
	F64_EQ  		= 0x61u,
	F64_NE  		= 0x62u,
	F64_LT			= 0x63u,
	F64_GT			= 0x64u,
	F64_LE			= 0x65u,
	F64_GE			= 0x66u,


	// CONVERSION INSTRUCTIONS
	// to int32
	I32_WRAP		= 0xa7u,
	I32_TRUNC_F32_S		= 0xa8u,
	I32_TRUNC_F32_U		= 0xa9u,
	I32_TRUNC_F64_S		= 0xaau,
	I32_TRUNC_F64_U		= 0xabu,
	I32_REINTERPRET_F32	= 0xbcu,
	
	// to int64
	I64_EXTEND_S 		= 0xacu,
	I64_EXTEND_U		= 0xadu,
	I64_TRUNC_F32_S		= 0xaeu,
	I64_TRUNC_F32_U		= 0xafu,
	I64_TRUNC_F64_S		= 0xb0u,
	I64_TRUNC_F64_U		= 0xb1u,
	I64_REINTERPRET_F64	= 0xbdu,

	// to float32
	F32_DEMOTE              = 0xb6u,
	F32_CONVERT_I32_S       = 0xb2u,
	F32_CONVERT_I32_U       = 0xb3u,
	F32_CONVERT_I64_S       = 0xb4u,
	F32_CONVERT_I64_U       = 0xb5u,
	F32_REINTERPRET_I32     = 0xbeu,

	// to float64
	F64_PROMOTE             = 0xbbu,
	F64_CONVERT_I32_S       = 0xb7u,
	F64_CONVERT_I32_U       = 0xb8u,
	F64_CONVERT_I64_S       = 0xb9u,
	F64_CONVERT_I64_U       = 0xbau,
	F64_REINTERPRET_I64     = 0xbfu,


	// LOAD AND STORE INSTRUCTIONSu

	I32_LOAD		= 0x28u,
	I64_LOAD		= 0x29u,
	F32_LOAD		= 0x2au,
	F64_LOAD		= 0x2bu,
	I32_LOAD8_S		= 0x2cu,
	I32_LOAD8_U		= 0x2du,
	I32_LOAD16_S		= 0x2eu,
	I32_LOAD16_U		= 0x2fu,
	I64_LOAD8_S		= 0x30u,
	I64_LOAD8_U		= 0x31u,
	I64_LOAD16_S		= 0x32u,
	I64_LOAD16_U		= 0x33u,
	I64_LOAD32_S		= 0x34u,
	I64_LOAD32_U		= 0x35u,
	I32_STORE		= 0x36u,
	I64_STORE		= 0x37u,
	F32_STORE		= 0x38u,
	F64_STORE		= 0x39u,
	I32_STORE8		= 0x3au,
	I32_STORE16		= 0x3bu,
	I64_STORE8		= 0x3cu,
	I64_STORE16		= 0x3du,
	I64_STORE32		= 0x3eu,

	// MEMORY INSTRUCTIONS
	GROW_MEMORY 		= 0x40u,
	CURRENT_MEMORY 		= 0x3fu,
};


static constexpr const 
std::array<wasm_opcode::wasm_opcode_t, 84> non_instructions{
	  6,   7,   8,   9,  10,  
	 18,  19,  20,  21,  22,  23,  24,  25,  
	 28,  29,  30,  31,  
  	 37,  38,  39, 
	192, 193, 194, 195, 196, 197, 198, 
	199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 
	219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 
	229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 
	239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 
	249, 250, 251, 252, 253, 254, 255
	// don't worry, I didn't do this by hand :)
};

inline bool wasm_instruction_dne(wasm_opcode::wasm_opcode_t opcode)
{
	// most compilers should be able to optimize this very well.
	// very brittle function; sensitive to changes in spec
	return (opcode > 191) 
		or ((opcode > 17) and (opcode < 26))
		or ((opcode >  5) and (opcode < 11))
		or ((opcode > 27) and (opcode < 32))
		or ((opcode > 36) and (opcode < 40));
}



const std::array<const char*, 256>& opcode_names()
{
	static const std::array<const char*, 256> names { [](){ // immediately-invoked lambda
		std::array<const char*, 256> tmp;
		tmp.fill("UNUSED-OPCODE");

		tmp[BLOCK]               = "block";
		tmp[LOOP]                = "loop";
		tmp[BR]                  = "br";
		tmp[BR_IF]               = "br_if";
		tmp[BR_TABLE]            = "br_table";
		tmp[IF]                  = "if";
		tmp[ELSE]                = "else";
		tmp[END]                 = "end";
		tmp[RETURN]              = "return";
		tmp[UNREACHABLE]         = "unreachable";

		tmp[NOP]                 = "nop";
		tmp[DROP]                = "drop";
		tmp[I32_CONST]           = "i32.const";
		tmp[I64_CONST]           = "i64.const";
		tmp[F32_CONST]           = "f32.const";
		tmp[F64_CONST]           = "f64.const";
		tmp[GET_LOCAL]           = "get_local";
		tmp[SET_LOCAL]           = "set_local";
		tmp[TEE_LOCAL]           = "tee_local";
		tmp[GET_GLOBAL]          = "get_global";
		tmp[SET_GLOBAL]          = "set_global";
		tmp[SELECT]              = "select";
		tmp[CALL]                = "call";
		tmp[CALL_INDIRECT]       = "call_indirect";

		tmp[I32_ADD]             = "i32.add";
		tmp[I32_SUB]             = "i32.sub";
		tmp[I32_MUL]             = "i32.mul";
		tmp[I32_DIV_S]           = "i32.div_s";
		tmp[I32_DIV_U]           = "i32.div_u";
		tmp[I32_REM_S]           = "i32.rem_s";
		tmp[I32_REM_U]           = "i32.rem_u";
		tmp[I32_AND]             = "i32.and";
		tmp[I32_OR]              = "i32.or";
		tmp[I32_XOR]             = "i32.xor";
		tmp[I32_SHL]             = "i32.shl";
		tmp[I32_SHR_S]           = "i32.shr_s";
		tmp[I32_SHR_U]           = "i32.shr_u";
		tmp[I32_ROTL]            = "i32.rotl";
		tmp[I32_ROTR]            = "i32.rotr";
		tmp[I32_CLZ]             = "i32.clz";
		tmp[I32_CTZ]             = "i32.ctz";
		tmp[I32_POPCNT]          = "i32.popcnt";
		tmp[I32_EQZ]             = "i32.eqz";

		tmp[I64_ADD]             = "i64.add";
		tmp[I64_SUB]             = "i64.sub";
		tmp[I64_MUL]             = "i64.mul";
		tmp[I64_DIV_S]           = "i64.div_s";
		tmp[I64_DIV_U]           = "i64.div_u";
		tmp[I64_REM_S]           = "i64.rem_s";
		tmp[I64_REM_U]           = "i64.rem_u";
		tmp[I64_AND]             = "i64.and";
		tmp[I64_OR]              = "i64.or";
		tmp[I64_XOR]             = "i64.xor";
		tmp[I64_SHL]             = "i64.shl";
		tmp[I64_SHR_S]           = "i64.shr_s";
		tmp[I64_SHR_U]           = "i64.shr_u";
		tmp[I64_ROTL]            = "i64.rotl";
		tmp[I64_ROTR]            = "i64.rotr";
		tmp[I64_CLZ]             = "i64.clz";
		tmp[I64_CTZ]             = "i64.ctz";
		tmp[I64_POPCNT]          = "i64.popcnt";
		tmp[I64_EQZ]             = "i64.eqz";

		tmp[F32_ADD]             = "f32.add";
		tmp[F32_SUB]             = "f32.sub";
		tmp[F32_MUL]             = "f32.mul";
		tmp[F32_DIV]             = "f32.div";
		tmp[F32_SQRT]            = "f32.sqrt";
		tmp[F32_MIN]             = "f32.min";
		tmp[F32_MAX]             = "f32.max";
		tmp[F32_CEIL]            = "f32.ceil";
		tmp[F32_FLOOR]           = "f32.floor";
		tmp[F32_TRUNC]           = "f32.trunc";
		tmp[F32_NEAREST]         = "f32.nearest";
		tmp[F32_ABS]             = "f32.abs";
		tmp[F32_NEG]             = "f32.neg";
		tmp[F32_COPYSIGN]        = "f32.copysign";

		tmp[F64_ADD]             = "f64.add";
		tmp[F64_SUB]             = "f64.sub";
		tmp[F64_MUL]             = "f64.mul";
		tmp[F64_DIV]             = "f64.div";
		tmp[F64_SQRT]            = "f64.sqrt";
		tmp[F64_MIN]             = "f64.min";
		tmp[F64_MAX]             = "f64.max";
		tmp[F64_CEIL]            = "f64.ceil";
		tmp[F64_FLOOR]           = "f64.floor";
		tmp[F64_TRUNC]           = "f64.trunc";
		tmp[F64_NEAREST]         = "f64.nearest";
		tmp[F64_ABS]             = "f64.abs";
		tmp[F64_NEG]             = "f64.neg";
		tmp[F64_COPYSIGN]        = "f64.copysign";

		tmp[I32_EQ]              = "i32.eq";
		tmp[I32_NE]              = "i32.ne";
		tmp[I32_LT_S]            = "i32.lt_s";
		tmp[I32_LT_U]            = "i32.lt_u";
		tmp[I32_GT_S]            = "i32.gt_s";
		tmp[I32_GT_U]            = "i32.gt_u";
		tmp[I32_LE_S]            = "i32.le_s";
		tmp[I32_LE_U]            = "i32.le_u";
		tmp[I32_GE_S]            = "i32.ge_s";
		tmp[I32_GE_U]            = "i32.ge_u";

		tmp[I64_EQ]              = "i64.eq";
		tmp[I64_NE]              = "i64.ne";
		tmp[I64_LT_S]            = "i64.lt_s";
		tmp[I64_LT_U]            = "i64.lt_u";
		tmp[I64_GT_S]            = "i64.gt_s";
		tmp[I64_GT_U]            = "i64.gt_u";
		tmp[I64_LE_S]            = "i64.le_s";
		tmp[I64_LE_U]            = "i64.le_u";
		tmp[I64_GE_S]            = "i64.ge_s";
		tmp[I64_GE_U]            = "i64.ge_u";

		tmp[F32_EQ]              = "f32.eq";
		tmp[F32_NE]              = "f32.ne";
		tmp[F32_LT]              = "f32.lt";
		tmp[F32_GT]              = "f32.gt";
		tmp[F32_LE]              = "f32.le";
		tmp[F32_GE]              = "f32.ge";

		tmp[F64_EQ]              = "f64.eq";
		tmp[F64_NE]              = "f64.ne";
		tmp[F64_LT]              = "f64.lt";
		tmp[F64_GT]              = "f64.gt";
		tmp[F64_LE]              = "f64.le";
		tmp[F64_GE]              = "f64.ge";

		tmp[I32_WRAP]            = "i32.wrap/i64";
		tmp[I32_TRUNC_F32_S]     = "i32.trunc_s/f32";
		tmp[I32_TRUNC_F32_U]     = "i32.trunc_u/f32";
		tmp[I32_TRUNC_F64_S]     = "i32.trunc_s/f64";
		tmp[I32_TRUNC_F64_U]     = "i32.trunc_u/f64";
		tmp[I32_REINTERPRET_F32] = "i32.reinterpret/f32";

		tmp[I64_EXTEND_S]        = "i64.extend_s/i32";
		tmp[I64_EXTEND_U]        = "i64.extend_u/i32";
		tmp[I64_TRUNC_F32_S]     = "i64.trunc_s/f32";
		tmp[I64_TRUNC_F32_U]     = "i64.trunc_u/f32";
		tmp[I64_TRUNC_F64_S]     = "i64.trunc_s/f64";
		tmp[I64_TRUNC_F64_U]     = "i64.trunc_u/f64";
		tmp[I64_REINTERPRET_F64] = "i64.reinterpret/f64";

		tmp[F32_DEMOTE]          = "f32.demote/f64";
		tmp[F32_CONVERT_I32_S]   = "f32.convert_s/i32";
		tmp[F32_CONVERT_I32_U]   = "f32.convert_u/i32";
		tmp[F32_CONVERT_I64_S]   = "f32.convert_s/i64";
		tmp[F32_CONVERT_I64_U]   = "f32.convert_u/i64";
		tmp[F32_REINTERPRET_I32] = "f32.reinterpret/i32";

		tmp[F64_PROMOTE]         = "f64.promote";
		tmp[F64_CONVERT_I32_S]   = "f64.convert_s/i32";
		tmp[F64_CONVERT_I32_U]   = "f64.convert_u/i32";
		tmp[F64_CONVERT_I64_S]   = "f64.convert_s/i64";
		tmp[F64_CONVERT_I64_U]   = "f64.convert_u/i64";
		tmp[F64_REINTERPRET_I64] = "f64.reinterpret/i64";

		tmp[I32_LOAD]            = "i32.load";
		tmp[I64_LOAD]            = "i64.load";
		tmp[F32_LOAD]            = "f32.load";
		tmp[F64_LOAD]            = "f64.load";
		tmp[I32_LOAD8_S]         = "i32.load8_s";
		tmp[I32_LOAD8_U]         = "i32.load8_u";
		tmp[I32_LOAD16_S]        = "i32.load16_s";
		tmp[I32_LOAD16_U]        = "i32.load16_u";
		tmp[I64_LOAD8_S]         = "i64.load8_s";
		tmp[I64_LOAD8_U]         = "i64.load8_u";
		tmp[I64_LOAD16_S]        = "i64.load16_s";
		tmp[I64_LOAD16_U]        = "i64.load16_u";
		tmp[I64_LOAD32_S]        = "i64.load32_s";
		tmp[I64_LOAD32_U]        = "i64.load32_u";
		tmp[I32_STORE]           = "i32.store";
		tmp[I64_STORE]           = "i64.store";
		tmp[F32_STORE]           = "f32.store";
		tmp[F64_STORE]           = "f64.store";
		tmp[I32_STORE8]          = "i32.store8";
		tmp[I32_STORE16]         = "i32.store16";
		tmp[I64_STORE8]          = "i64.store8";
		tmp[I64_STORE16]         = "i64.store16";
		tmp[I64_STORE32]         = "i64.store32";

		tmp[GROW_MEMORY]         = "grow_memory";
		tmp[CURRENT_MEMORY]      = "current_memory";
		return tmp;
	}()/* invoke lambda */}; /* names */
	return names;
}

namespace detail {
struct WriteCode {
	constexpr WriteCode() = default;
};
}
inline constexpr const WriteCode writecode;


auto operator<<(WriteCode wc, Opcode oc) 
{ return static_cast<unsigned>(oc); }

std::ostream& operator<<(std::ostream& os, Opcode oc)
{ return os << opcode_names()[oc]; }

} /* namespace opc */
} /* namespace wasm */





#endif /* WASM_INSTRUCTION_H */
