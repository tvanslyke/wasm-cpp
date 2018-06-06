#ifndef WASM_INSTRUCTION_H
#define WASM_INSTRUCTION_H
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include <array>
#include <bitset>
#include <iosfwd>
#include <ostream>
#include <iomanip>
#include "wasm_base.h"


namespace wasm {
namespace opc {

enum class OpCode: wasm_ubyte_t
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





namespace detail {

const std::bitset<256>& opcode_mask()
{
	static const std::bitset<256> bs {[]() -> std::bitset<256> {
		using int_type = std::underlying_type_t<OpCode>;
		std::bitset<256> tmp;
		tmp.set(static_cast<int_type>(OpCode::BLOCK));
		tmp.set(static_cast<int_type>(OpCode::LOOP));
		tmp.set(static_cast<int_type>(OpCode::BR));
		tmp.set(static_cast<int_type>(OpCode::BR_IF));
		tmp.set(static_cast<int_type>(OpCode::BR_TABLE));
		tmp.set(static_cast<int_type>(OpCode::IF));
		tmp.set(static_cast<int_type>(OpCode::ELSE));
		tmp.set(static_cast<int_type>(OpCode::END));
		tmp.set(static_cast<int_type>(OpCode::RETURN));
		tmp.set(static_cast<int_type>(OpCode::UNREACHABLE));

		tmp.set(static_cast<int_type>(OpCode::NOP));
		tmp.set(static_cast<int_type>(OpCode::DROP));
		tmp.set(static_cast<int_type>(OpCode::I32_CONST));
		tmp.set(static_cast<int_type>(OpCode::I64_CONST));
		tmp.set(static_cast<int_type>(OpCode::F32_CONST));
		tmp.set(static_cast<int_type>(OpCode::F64_CONST));
		tmp.set(static_cast<int_type>(OpCode::GET_LOCAL));
		tmp.set(static_cast<int_type>(OpCode::SET_LOCAL));
		tmp.set(static_cast<int_type>(OpCode::TEE_LOCAL));
		tmp.set(static_cast<int_type>(OpCode::GET_GLOBAL));
		tmp.set(static_cast<int_type>(OpCode::SET_GLOBAL));
		tmp.set(static_cast<int_type>(OpCode::SELECT));
		tmp.set(static_cast<int_type>(OpCode::CALL));
		tmp.set(static_cast<int_type>(OpCode::CALL_INDIRECT));

		tmp.set(static_cast<int_type>(OpCode::I32_ADD));
		tmp.set(static_cast<int_type>(OpCode::I32_SUB));
		tmp.set(static_cast<int_type>(OpCode::I32_MUL));
		tmp.set(static_cast<int_type>(OpCode::I32_DIV_S));
		tmp.set(static_cast<int_type>(OpCode::I32_DIV_U));
		tmp.set(static_cast<int_type>(OpCode::I32_REM_S));
		tmp.set(static_cast<int_type>(OpCode::I32_REM_U));
		tmp.set(static_cast<int_type>(OpCode::I32_AND));
		tmp.set(static_cast<int_type>(OpCode::I32_OR));
		tmp.set(static_cast<int_type>(OpCode::I32_XOR));
		tmp.set(static_cast<int_type>(OpCode::I32_SHL));
		tmp.set(static_cast<int_type>(OpCode::I32_SHR_S));
		tmp.set(static_cast<int_type>(OpCode::I32_SHR_U));
		tmp.set(static_cast<int_type>(OpCode::I32_ROTL));
		tmp.set(static_cast<int_type>(OpCode::I32_ROTR));
		tmp.set(static_cast<int_type>(OpCode::I32_CLZ));
		tmp.set(static_cast<int_type>(OpCode::I32_CTZ));
		tmp.set(static_cast<int_type>(OpCode::I32_POPCNT));
		tmp.set(static_cast<int_type>(OpCode::I32_EQZ));

		tmp.set(static_cast<int_type>(OpCode::I64_ADD));
		tmp.set(static_cast<int_type>(OpCode::I64_SUB));
		tmp.set(static_cast<int_type>(OpCode::I64_MUL));
		tmp.set(static_cast<int_type>(OpCode::I64_DIV_S));
		tmp.set(static_cast<int_type>(OpCode::I64_DIV_U));
		tmp.set(static_cast<int_type>(OpCode::I64_REM_S));
		tmp.set(static_cast<int_type>(OpCode::I64_REM_U));
		tmp.set(static_cast<int_type>(OpCode::I64_AND));
		tmp.set(static_cast<int_type>(OpCode::I64_OR));
		tmp.set(static_cast<int_type>(OpCode::I64_XOR));
		tmp.set(static_cast<int_type>(OpCode::I64_SHL));
		tmp.set(static_cast<int_type>(OpCode::I64_SHR_S));
		tmp.set(static_cast<int_type>(OpCode::I64_SHR_U));
		tmp.set(static_cast<int_type>(OpCode::I64_ROTL));
		tmp.set(static_cast<int_type>(OpCode::I64_ROTR));
		tmp.set(static_cast<int_type>(OpCode::I64_CLZ));
		tmp.set(static_cast<int_type>(OpCode::I64_CTZ));
		tmp.set(static_cast<int_type>(OpCode::I64_POPCNT));
		tmp.set(static_cast<int_type>(OpCode::I64_EQZ));

		tmp.set(static_cast<int_type>(OpCode::F32_ADD));
		tmp.set(static_cast<int_type>(OpCode::F32_SUB));
		tmp.set(static_cast<int_type>(OpCode::F32_MUL));
		tmp.set(static_cast<int_type>(OpCode::F32_DIV));
		tmp.set(static_cast<int_type>(OpCode::F32_SQRT));
		tmp.set(static_cast<int_type>(OpCode::F32_MIN));
		tmp.set(static_cast<int_type>(OpCode::F32_MAX));
		tmp.set(static_cast<int_type>(OpCode::F32_CEIL));
		tmp.set(static_cast<int_type>(OpCode::F32_FLOOR));
		tmp.set(static_cast<int_type>(OpCode::F32_TRUNC));
		tmp.set(static_cast<int_type>(OpCode::F32_NEAREST));
		tmp.set(static_cast<int_type>(OpCode::F32_ABS));
		tmp.set(static_cast<int_type>(OpCode::F32_NEG));
		tmp.set(static_cast<int_type>(OpCode::F32_COPYSIGN));

		tmp.set(static_cast<int_type>(OpCode::F64_ADD));
		tmp.set(static_cast<int_type>(OpCode::F64_SUB));
		tmp.set(static_cast<int_type>(OpCode::F64_MUL));
		tmp.set(static_cast<int_type>(OpCode::F64_DIV));
		tmp.set(static_cast<int_type>(OpCode::F64_SQRT));
		tmp.set(static_cast<int_type>(OpCode::F64_MIN));
		tmp.set(static_cast<int_type>(OpCode::F64_MAX));
		tmp.set(static_cast<int_type>(OpCode::F64_CEIL));
		tmp.set(static_cast<int_type>(OpCode::F64_FLOOR));
		tmp.set(static_cast<int_type>(OpCode::F64_TRUNC));
		tmp.set(static_cast<int_type>(OpCode::F64_NEAREST));
		tmp.set(static_cast<int_type>(OpCode::F64_ABS));
		tmp.set(static_cast<int_type>(OpCode::F64_NEG));
		tmp.set(static_cast<int_type>(OpCode::F64_COPYSIGN));

		tmp.set(static_cast<int_type>(OpCode::I32_EQ));
		tmp.set(static_cast<int_type>(OpCode::I32_NE));
		tmp.set(static_cast<int_type>(OpCode::I32_LT_S));
		tmp.set(static_cast<int_type>(OpCode::I32_LT_U));
		tmp.set(static_cast<int_type>(OpCode::I32_GT_S));
		tmp.set(static_cast<int_type>(OpCode::I32_GT_U));
		tmp.set(static_cast<int_type>(OpCode::I32_LE_S));
		tmp.set(static_cast<int_type>(OpCode::I32_LE_U));
		tmp.set(static_cast<int_type>(OpCode::I32_GE_S));
		tmp.set(static_cast<int_type>(OpCode::I32_GE_U));

		tmp.set(static_cast<int_type>(OpCode::I64_EQ));
		tmp.set(static_cast<int_type>(OpCode::I64_NE));
		tmp.set(static_cast<int_type>(OpCode::I64_LT_S));
		tmp.set(static_cast<int_type>(OpCode::I64_LT_U));
		tmp.set(static_cast<int_type>(OpCode::I64_GT_S));
		tmp.set(static_cast<int_type>(OpCode::I64_GT_U));
		tmp.set(static_cast<int_type>(OpCode::I64_LE_S));
		tmp.set(static_cast<int_type>(OpCode::I64_LE_U));
		tmp.set(static_cast<int_type>(OpCode::I64_GE_S));
		tmp.set(static_cast<int_type>(OpCode::I64_GE_U));

		tmp.set(static_cast<int_type>(OpCode::F32_EQ));
		tmp.set(static_cast<int_type>(OpCode::F32_NE));
		tmp.set(static_cast<int_type>(OpCode::F32_LT));
		tmp.set(static_cast<int_type>(OpCode::F32_GT));
		tmp.set(static_cast<int_type>(OpCode::F32_LE));
		tmp.set(static_cast<int_type>(OpCode::F32_GE));

		tmp.set(static_cast<int_type>(OpCode::F64_EQ));
		tmp.set(static_cast<int_type>(OpCode::F64_NE));
		tmp.set(static_cast<int_type>(OpCode::F64_LT));
		tmp.set(static_cast<int_type>(OpCode::F64_GT));
		tmp.set(static_cast<int_type>(OpCode::F64_LE));
		tmp.set(static_cast<int_type>(OpCode::F64_GE));

		tmp.set(static_cast<int_type>(OpCode::I32_WRAP));
		tmp.set(static_cast<int_type>(OpCode::I32_TRUNC_F32_S));
		tmp.set(static_cast<int_type>(OpCode::I32_TRUNC_F32_U));
		tmp.set(static_cast<int_type>(OpCode::I32_TRUNC_F64_S));
		tmp.set(static_cast<int_type>(OpCode::I32_TRUNC_F64_U));
		tmp.set(static_cast<int_type>(OpCode::I32_REINTERPRET_F32));

		tmp.set(static_cast<int_type>(OpCode::I64_EXTEND_S));
		tmp.set(static_cast<int_type>(OpCode::I64_EXTEND_U));
		tmp.set(static_cast<int_type>(OpCode::I64_TRUNC_F32_S));
		tmp.set(static_cast<int_type>(OpCode::I64_TRUNC_F32_U));
		tmp.set(static_cast<int_type>(OpCode::I64_TRUNC_F64_S));
		tmp.set(static_cast<int_type>(OpCode::I64_TRUNC_F64_U));
		tmp.set(static_cast<int_type>(OpCode::I64_REINTERPRET_F64));

		tmp.set(static_cast<int_type>(OpCode::F32_DEMOTE));
		tmp.set(static_cast<int_type>(OpCode::F32_CONVERT_I32_S));
		tmp.set(static_cast<int_type>(OpCode::F32_CONVERT_I32_U));
		tmp.set(static_cast<int_type>(OpCode::F32_CONVERT_I64_S));
		tmp.set(static_cast<int_type>(OpCode::F32_CONVERT_I64_U));
		tmp.set(static_cast<int_type>(OpCode::F32_REINTERPRET_I32));

		tmp.set(static_cast<int_type>(OpCode::F64_PROMOTE));
		tmp.set(static_cast<int_type>(OpCode::F64_CONVERT_I32_S));
		tmp.set(static_cast<int_type>(OpCode::F64_CONVERT_I32_U));
		tmp.set(static_cast<int_type>(OpCode::F64_CONVERT_I64_S));
		tmp.set(static_cast<int_type>(OpCode::F64_CONVERT_I64_U));
		tmp.set(static_cast<int_type>(OpCode::F64_REINTERPRET_I64));

		tmp.set(static_cast<int_type>(OpCode::I32_LOAD));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD));
		tmp.set(static_cast<int_type>(OpCode::F32_LOAD));
		tmp.set(static_cast<int_type>(OpCode::F64_LOAD));
		tmp.set(static_cast<int_type>(OpCode::I32_LOAD8_S));
		tmp.set(static_cast<int_type>(OpCode::I32_LOAD8_U));
		tmp.set(static_cast<int_type>(OpCode::I32_LOAD16_S));
		tmp.set(static_cast<int_type>(OpCode::I32_LOAD16_U));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD8_S));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD8_U));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD16_S));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD16_U));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD32_S));
		tmp.set(static_cast<int_type>(OpCode::I64_LOAD32_U));
		tmp.set(static_cast<int_type>(OpCode::I32_STORE));
		tmp.set(static_cast<int_type>(OpCode::I64_STORE));
		tmp.set(static_cast<int_type>(OpCode::F32_STORE));
		tmp.set(static_cast<int_type>(OpCode::F64_STORE));
		tmp.set(static_cast<int_type>(OpCode::I32_STORE8));
		tmp.set(static_cast<int_type>(OpCode::I32_STORE16));
		tmp.set(static_cast<int_type>(OpCode::I64_STORE8));
		tmp.set(static_cast<int_type>(OpCode::I64_STORE16));
		tmp.set(static_cast<int_type>(OpCode::I64_STORE32));

		tmp.set(static_cast<int_type>(OpCode::GROW_MEMORY));
		tmp.set(static_cast<int_type>(OpCode::CURRENT_MEMORY));
		return tmp;
	}()};
	return bs;
}


const std::array<const char*, 256>& opcode_names()
{
	static const std::array<const char*, 256> names { [](){ // immediately-invoked lambda
		std::array<const char*, 256> tmp;
		tmp.fill(nullptr);

		using int_type = std::underlying_type_t<OpCode>;

		tmp[static_cast<int_type>(OpCode::BLOCK)]               = "block";
		tmp[static_cast<int_type>(OpCode::LOOP)]                = "loop";
		tmp[static_cast<int_type>(OpCode::BR)]                  = "br";
		tmp[static_cast<int_type>(OpCode::BR_IF)]               = "br_if";
		tmp[static_cast<int_type>(OpCode::BR_TABLE)]            = "br_table";
		tmp[static_cast<int_type>(OpCode::IF)]                  = "if";
		tmp[static_cast<int_type>(OpCode::ELSE)]                = "else";
		tmp[static_cast<int_type>(OpCode::END)]                 = "end";
		tmp[static_cast<int_type>(OpCode::RETURN)]              = "return";
		tmp[static_cast<int_type>(OpCode::UNREACHABLE)]         = "unreachable";

		tmp[static_cast<int_type>(OpCode::NOP)]                 = "nop";
		tmp[static_cast<int_type>(OpCode::DROP)]                = "drop";
		tmp[static_cast<int_type>(OpCode::I32_CONST)]           = "i32.const";
		tmp[static_cast<int_type>(OpCode::I64_CONST)]           = "i64.const";
		tmp[static_cast<int_type>(OpCode::F32_CONST)]           = "f32.const";
		tmp[static_cast<int_type>(OpCode::F64_CONST)]           = "f64.const";
		tmp[static_cast<int_type>(OpCode::GET_LOCAL)]           = "get_local";
		tmp[static_cast<int_type>(OpCode::SET_LOCAL)]           = "set_local";
		tmp[static_cast<int_type>(OpCode::TEE_LOCAL)]           = "tee_local";
		tmp[static_cast<int_type>(OpCode::GET_GLOBAL)]          = "get_global";
		tmp[static_cast<int_type>(OpCode::SET_GLOBAL)]          = "set_global";
		tmp[static_cast<int_type>(OpCode::SELECT)]              = "select";
		tmp[static_cast<int_type>(OpCode::CALL)]                = "call";
		tmp[static_cast<int_type>(OpCode::CALL_INDIRECT)]       = "call_indirect";

		tmp[static_cast<int_type>(OpCode::I32_ADD)]             = "i32.add";
		tmp[static_cast<int_type>(OpCode::I32_SUB)]             = "i32.sub";
		tmp[static_cast<int_type>(OpCode::I32_MUL)]             = "i32.mul";
		tmp[static_cast<int_type>(OpCode::I32_DIV_S)]           = "i32.div_s";
		tmp[static_cast<int_type>(OpCode::I32_DIV_U)]           = "i32.div_u";
		tmp[static_cast<int_type>(OpCode::I32_REM_S)]           = "i32.rem_s";
		tmp[static_cast<int_type>(OpCode::I32_REM_U)]           = "i32.rem_u";
		tmp[static_cast<int_type>(OpCode::I32_AND)]             = "i32.and";
		tmp[static_cast<int_type>(OpCode::I32_OR)]              = "i32.or";
		tmp[static_cast<int_type>(OpCode::I32_XOR)]             = "i32.xor";
		tmp[static_cast<int_type>(OpCode::I32_SHL)]             = "i32.shl";
		tmp[static_cast<int_type>(OpCode::I32_SHR_S)]           = "i32.shr_s";
		tmp[static_cast<int_type>(OpCode::I32_SHR_U)]           = "i32.shr_u";
		tmp[static_cast<int_type>(OpCode::I32_ROTL)]            = "i32.rotl";
		tmp[static_cast<int_type>(OpCode::I32_ROTR)]            = "i32.rotr";
		tmp[static_cast<int_type>(OpCode::I32_CLZ)]             = "i32.clz";
		tmp[static_cast<int_type>(OpCode::I32_CTZ)]             = "i32.ctz";
		tmp[static_cast<int_type>(OpCode::I32_POPCNT)]          = "i32.popcnt";
		tmp[static_cast<int_type>(OpCode::I32_EQZ)]             = "i32.eqz";

		tmp[static_cast<int_type>(OpCode::I64_ADD)]             = "i64.add";
		tmp[static_cast<int_type>(OpCode::I64_SUB)]             = "i64.sub";
		tmp[static_cast<int_type>(OpCode::I64_MUL)]             = "i64.mul";
		tmp[static_cast<int_type>(OpCode::I64_DIV_S)]           = "i64.div_s";
		tmp[static_cast<int_type>(OpCode::I64_DIV_U)]           = "i64.div_u";
		tmp[static_cast<int_type>(OpCode::I64_REM_S)]           = "i64.rem_s";
		tmp[static_cast<int_type>(OpCode::I64_REM_U)]           = "i64.rem_u";
		tmp[static_cast<int_type>(OpCode::I64_AND)]             = "i64.and";
		tmp[static_cast<int_type>(OpCode::I64_OR)]              = "i64.or";
		tmp[static_cast<int_type>(OpCode::I64_XOR)]             = "i64.xor";
		tmp[static_cast<int_type>(OpCode::I64_SHL)]             = "i64.shl";
		tmp[static_cast<int_type>(OpCode::I64_SHR_S)]           = "i64.shr_s";
		tmp[static_cast<int_type>(OpCode::I64_SHR_U)]           = "i64.shr_u";
		tmp[static_cast<int_type>(OpCode::I64_ROTL)]            = "i64.rotl";
		tmp[static_cast<int_type>(OpCode::I64_ROTR)]            = "i64.rotr";
		tmp[static_cast<int_type>(OpCode::I64_CLZ)]             = "i64.clz";
		tmp[static_cast<int_type>(OpCode::I64_CTZ)]             = "i64.ctz";
		tmp[static_cast<int_type>(OpCode::I64_POPCNT)]          = "i64.popcnt";
		tmp[static_cast<int_type>(OpCode::I64_EQZ)]             = "i64.eqz";

		tmp[static_cast<int_type>(OpCode::F32_ADD)]             = "f32.add";
		tmp[static_cast<int_type>(OpCode::F32_SUB)]             = "f32.sub";
		tmp[static_cast<int_type>(OpCode::F32_MUL)]             = "f32.mul";
		tmp[static_cast<int_type>(OpCode::F32_DIV)]             = "f32.div";
		tmp[static_cast<int_type>(OpCode::F32_SQRT)]            = "f32.sqrt";
		tmp[static_cast<int_type>(OpCode::F32_MIN)]             = "f32.min";
		tmp[static_cast<int_type>(OpCode::F32_MAX)]             = "f32.max";
		tmp[static_cast<int_type>(OpCode::F32_CEIL)]            = "f32.ceil";
		tmp[static_cast<int_type>(OpCode::F32_FLOOR)]           = "f32.floor";
		tmp[static_cast<int_type>(OpCode::F32_TRUNC)]           = "f32.trunc";
		tmp[static_cast<int_type>(OpCode::F32_NEAREST)]         = "f32.nearest";
		tmp[static_cast<int_type>(OpCode::F32_ABS)]             = "f32.abs";
		tmp[static_cast<int_type>(OpCode::F32_NEG)]             = "f32.neg";
		tmp[static_cast<int_type>(OpCode::F32_COPYSIGN)]        = "f32.copysign";

		tmp[static_cast<int_type>(OpCode::F64_ADD)]             = "f64.add";
		tmp[static_cast<int_type>(OpCode::F64_SUB)]             = "f64.sub";
		tmp[static_cast<int_type>(OpCode::F64_MUL)]             = "f64.mul";
		tmp[static_cast<int_type>(OpCode::F64_DIV)]             = "f64.div";
		tmp[static_cast<int_type>(OpCode::F64_SQRT)]            = "f64.sqrt";
		tmp[static_cast<int_type>(OpCode::F64_MIN)]             = "f64.min";
		tmp[static_cast<int_type>(OpCode::F64_MAX)]             = "f64.max";
		tmp[static_cast<int_type>(OpCode::F64_CEIL)]            = "f64.ceil";
		tmp[static_cast<int_type>(OpCode::F64_FLOOR)]           = "f64.floor";
		tmp[static_cast<int_type>(OpCode::F64_TRUNC)]           = "f64.trunc";
		tmp[static_cast<int_type>(OpCode::F64_NEAREST)]         = "f64.nearest";
		tmp[static_cast<int_type>(OpCode::F64_ABS)]             = "f64.abs";
		tmp[static_cast<int_type>(OpCode::F64_NEG)]             = "f64.neg";
		tmp[static_cast<int_type>(OpCode::F64_COPYSIGN)]        = "f64.copysign";

		tmp[static_cast<int_type>(OpCode::I32_EQ)]              = "i32.eq";
		tmp[static_cast<int_type>(OpCode::I32_NE)]              = "i32.ne";
		tmp[static_cast<int_type>(OpCode::I32_LT_S)]            = "i32.lt_s";
		tmp[static_cast<int_type>(OpCode::I32_LT_U)]            = "i32.lt_u";
		tmp[static_cast<int_type>(OpCode::I32_GT_S)]            = "i32.gt_s";
		tmp[static_cast<int_type>(OpCode::I32_GT_U)]            = "i32.gt_u";
		tmp[static_cast<int_type>(OpCode::I32_LE_S)]            = "i32.le_s";
		tmp[static_cast<int_type>(OpCode::I32_LE_U)]            = "i32.le_u";
		tmp[static_cast<int_type>(OpCode::I32_GE_S)]            = "i32.ge_s";
		tmp[static_cast<int_type>(OpCode::I32_GE_U)]            = "i32.ge_u";

		tmp[static_cast<int_type>(OpCode::I64_EQ)]              = "i64.eq";
		tmp[static_cast<int_type>(OpCode::I64_NE)]              = "i64.ne";
		tmp[static_cast<int_type>(OpCode::I64_LT_S)]            = "i64.lt_s";
		tmp[static_cast<int_type>(OpCode::I64_LT_U)]            = "i64.lt_u";
		tmp[static_cast<int_type>(OpCode::I64_GT_S)]            = "i64.gt_s";
		tmp[static_cast<int_type>(OpCode::I64_GT_U)]            = "i64.gt_u";
		tmp[static_cast<int_type>(OpCode::I64_LE_S)]            = "i64.le_s";
		tmp[static_cast<int_type>(OpCode::I64_LE_U)]            = "i64.le_u";
		tmp[static_cast<int_type>(OpCode::I64_GE_S)]            = "i64.ge_s";
		tmp[static_cast<int_type>(OpCode::I64_GE_U)]            = "i64.ge_u";

		tmp[static_cast<int_type>(OpCode::F32_EQ)]              = "f32.eq";
		tmp[static_cast<int_type>(OpCode::F32_NE)]              = "f32.ne";
		tmp[static_cast<int_type>(OpCode::F32_LT)]              = "f32.lt";
		tmp[static_cast<int_type>(OpCode::F32_GT)]              = "f32.gt";
		tmp[static_cast<int_type>(OpCode::F32_LE)]              = "f32.le";
		tmp[static_cast<int_type>(OpCode::F32_GE)]              = "f32.ge";

		tmp[static_cast<int_type>(OpCode::F64_EQ)]              = "f64.eq";
		tmp[static_cast<int_type>(OpCode::F64_NE)]              = "f64.ne";
		tmp[static_cast<int_type>(OpCode::F64_LT)]              = "f64.lt";
		tmp[static_cast<int_type>(OpCode::F64_GT)]              = "f64.gt";
		tmp[static_cast<int_type>(OpCode::F64_LE)]              = "f64.le";
		tmp[static_cast<int_type>(OpCode::F64_GE)]              = "f64.ge";

		tmp[static_cast<int_type>(OpCode::I32_WRAP)]            = "i32.wrap/i64";
		tmp[static_cast<int_type>(OpCode::I32_TRUNC_F32_S)]     = "i32.trunc_s/f32";
		tmp[static_cast<int_type>(OpCode::I32_TRUNC_F32_U)]     = "i32.trunc_u/f32";
		tmp[static_cast<int_type>(OpCode::I32_TRUNC_F64_S)]     = "i32.trunc_s/f64";
		tmp[static_cast<int_type>(OpCode::I32_TRUNC_F64_U)]     = "i32.trunc_u/f64";
		tmp[static_cast<int_type>(OpCode::I32_REINTERPRET_F32)] = "i32.reinterpret/f32";

		tmp[static_cast<int_type>(OpCode::I64_EXTEND_S)]        = "i64.extend_s/i32";
		tmp[static_cast<int_type>(OpCode::I64_EXTEND_U)]        = "i64.extend_u/i32";
		tmp[static_cast<int_type>(OpCode::I64_TRUNC_F32_S)]     = "i64.trunc_s/f32";
		tmp[static_cast<int_type>(OpCode::I64_TRUNC_F32_U)]     = "i64.trunc_u/f32";
		tmp[static_cast<int_type>(OpCode::I64_TRUNC_F64_S)]     = "i64.trunc_s/f64";
		tmp[static_cast<int_type>(OpCode::I64_TRUNC_F64_U)]     = "i64.trunc_u/f64";
		tmp[static_cast<int_type>(OpCode::I64_REINTERPRET_F64)] = "i64.reinterpret/f64";

		tmp[static_cast<int_type>(OpCode::F32_DEMOTE)]          = "f32.demote/f64";
		tmp[static_cast<int_type>(OpCode::F32_CONVERT_I32_S)]   = "f32.convert_s/i32";
		tmp[static_cast<int_type>(OpCode::F32_CONVERT_I32_U)]   = "f32.convert_u/i32";
		tmp[static_cast<int_type>(OpCode::F32_CONVERT_I64_S)]   = "f32.convert_s/i64";
		tmp[static_cast<int_type>(OpCode::F32_CONVERT_I64_U)]   = "f32.convert_u/i64";
		tmp[static_cast<int_type>(OpCode::F32_REINTERPRET_I32)] = "f32.reinterpret/i32";

		tmp[static_cast<int_type>(OpCode::F64_PROMOTE)]         = "f64.promote";
		tmp[static_cast<int_type>(OpCode::F64_CONVERT_I32_S)]   = "f64.convert_s/i32";
		tmp[static_cast<int_type>(OpCode::F64_CONVERT_I32_U)]   = "f64.convert_u/i32";
		tmp[static_cast<int_type>(OpCode::F64_CONVERT_I64_S)]   = "f64.convert_s/i64";
		tmp[static_cast<int_type>(OpCode::F64_CONVERT_I64_U)]   = "f64.convert_u/i64";
		tmp[static_cast<int_type>(OpCode::F64_REINTERPRET_I64)] = "f64.reinterpret/i64";

		tmp[static_cast<int_type>(OpCode::I32_LOAD)]            = "i32.load";
		tmp[static_cast<int_type>(OpCode::I64_LOAD)]            = "i64.load";
		tmp[static_cast<int_type>(OpCode::F32_LOAD)]            = "f32.load";
		tmp[static_cast<int_type>(OpCode::F64_LOAD)]            = "f64.load";
		tmp[static_cast<int_type>(OpCode::I32_LOAD8_S)]         = "i32.load8_s";
		tmp[static_cast<int_type>(OpCode::I32_LOAD8_U)]         = "i32.load8_u";
		tmp[static_cast<int_type>(OpCode::I32_LOAD16_S)]        = "i32.load16_s";
		tmp[static_cast<int_type>(OpCode::I32_LOAD16_U)]        = "i32.load16_u";
		tmp[static_cast<int_type>(OpCode::I64_LOAD8_S)]         = "i64.load8_s";
		tmp[static_cast<int_type>(OpCode::I64_LOAD8_U)]         = "i64.load8_u";
		tmp[static_cast<int_type>(OpCode::I64_LOAD16_S)]        = "i64.load16_s";
		tmp[static_cast<int_type>(OpCode::I64_LOAD16_U)]        = "i64.load16_u";
		tmp[static_cast<int_type>(OpCode::I64_LOAD32_S)]        = "i64.load32_s";
		tmp[static_cast<int_type>(OpCode::I64_LOAD32_U)]        = "i64.load32_u";
		tmp[static_cast<int_type>(OpCode::I32_STORE)]           = "i32.store";
		tmp[static_cast<int_type>(OpCode::I64_STORE)]           = "i64.store";
		tmp[static_cast<int_type>(OpCode::F32_STORE)]           = "f32.store";
		tmp[static_cast<int_type>(OpCode::F64_STORE)]           = "f64.store";
		tmp[static_cast<int_type>(OpCode::I32_STORE8)]          = "i32.store8";
		tmp[static_cast<int_type>(OpCode::I32_STORE16)]         = "i32.store16";
		tmp[static_cast<int_type>(OpCode::I64_STORE8)]          = "i64.store8";
		tmp[static_cast<int_type>(OpCode::I64_STORE16)]         = "i64.store16";
		tmp[static_cast<int_type>(OpCode::I64_STORE32)]         = "i64.store32";

		tmp[static_cast<int_type>(OpCode::GROW_MEMORY)]         = "grow_memory";
		tmp[static_cast<int_type>(OpCode::CURRENT_MEMORY)]      = "current_memory";

		return tmp;
	}()/* invoke lambda */}; /* names */
	return names;
}
} /* namespace detail */


std::ostream& operator<<(std::ostream& os, OpCode oc)
{
	const char* name = detail::opcode_names()[static_cast<unsigned>(oc)];
	if(not name)
	{
		os << "bad_opcode(0x" << std::hex << std::setw(2) << std::setfill('0');
		os << static_cast<unsigned>(oc) << ')';
	}
	else
	{
		os << name;
	}
	return os;
}

inline bool opcode_exists(std::underlying_type_t<OpCode> oc) [[gnu::pure]]
{
	return detail::opcode_mask().test(oc);
}

namespace detail {

template <class It>
std::tuple<wasm_uint32_t, wasm_uint32_t, It> read_memory_immediate(It first, It last) [[gnu::pure]]
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
std::pair<T, It> read_serialized_immediate(It first, It last) [[gnu::pure]]
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
				BadOpcodeError(op, "Given op is not a valid WASM opcode."),
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
		was_uint32_t label;
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

struct BranchTableImmediate:
{
	template <class ... T>
	BranchTableImmediate(T&& ... args):
		table_(std::forward<T>(args)...)
	{
		
	}

	wasm_uint32_t at(wasm_uint32_t idx) const
	{
		idx = std::min(table_.size() - 1u, idx);
		wasm_uint32_t depth;
		std::memcpy(&depth, table_.data() + idx, sizeof(depth));
		return depth;
	}

private:
	const gsl::span<const char[sizeof(wasm_uint32_t)]> table_;
};

struct WasmInstruction
{
	using null_immediate_type         = std::monostate;
	using i32_immediate_type          = wasm_sint32_t;
	using i64_immediate_type          = wasm_sint64_t;
	using f32_immediate_type          = wasm_float32_t;
	using f64_immediate_type          = wasm_float64_t;
	using offset_immediate_type       = wasm_uint32_t;
	using memory_immediate_type       = MemoryImmediate;
	using block_immediate_type        = BlockImmediate;
	using loop_immediate_type         = LanguageType;
	using branch_table_immediate_type = BranchTableImmediate;

	union immediate_type
	{
		// no immediate (most instructions)
		null_immediate_type         null_immed
		// i32.const
		i32_immediate_type          i32_immed;
		// i64.const
		i64_immediate_type          i64_immed;
		// f32.const
		f32_immediate_type          f32_immed;
		// f64.const
		f64_immediate_type          f64_immed;
		// offset: br, br_if, else (precomputed jump), call, call_indirect
		//         get_local, set_local, tee_local, get_global, set_global
		offset_immediate_type       offset_immed;
		// memory immediate: i32.load (0x28) through i64.store32 (0x3e)
		memory_immediate_type       memory_immed;
		// block immediate (signature + precomputed jump): if, block
		block_immediate_type        block_immed;
		// loop (signature)
		loop_immediate_type         loop_immed;
		// branch table unaligned array of offset_immediate_type (native byte order)
		// branch depths + one default depth.  
		branch_table_immediate_type br_table_immed;
	};

	using tagged_immediate_type = std::variant<
		null_immediate_type,
		i32_immediate_type,
		i64_immediate_type,
		f32_immediate_type,
		f64_immediate_type,
		offset_immediate_type,
		memory_immediate_type,
		block_immediate_type,
		loop_immediate_type,
		branch_table_immediate_type
	>;


private:
	template <class T>
	struct Tag{};

	void _assert_invariants() const
	{
		assert(opcode_exists(opcode));
		assert(source.size() >= 1u);
		if(not validate(null_immediate_type{}))
			assert(source.size() > 1u);
	}

	bool validate(Tag<null_immediate_type>) const
	{
		_assert_invariants();
		return ((opcode > OpCode::I32_EQZ) 
			or (opcode == OpCode::UNREACHABLE)
			or (opcode == OpCode::NOP)
			or (opcode == OpCode::ELSE)
			or (opcode == OpCode::END)
			or (opcode == OpCode::RETURN)
			or (opcode == OpCode::DROP)
			or (opcode == OpCode::SELECT)
			or (opcode == OpCode::CURRENT_MEMORY)
			or (opcode == OpCode::GROW_MEMORY)
		);
	}
	
	bool validate(Tag<offset_immediate_type>) const
	{
		_assert_invariants();
		return ((op >= OpCode::GET_LOCAL and op <= OpCode::SET_GLOBAL)
			or (op >= OpCode::CALL and op <= OpCode::CALL_INDIRECT)
			or (op >= OpCode::BR and op <= OpCode::BR_IF)
			or (op == OpCode::ELSE)
		);
	}

	bool validate(Tag<i32_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::I32_CONST);
	}

	bool validate(Tag<i64_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::I64_CONST);
	}

	bool validate(Tag<f32_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::F32_CONST);
	}

	bool validate(Tag<f64_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::F64_CONST);
	}

	bool validate(Tag<memory_immediate_type>) const
	{
		_assert_invariants();
		return (opcode >= OpCode::I32_LOAD)
			and (opcode >= OpCode::I64_STORE32);
	}

	bool validate(Tag<block_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::BLOCK)
			or (opcode == OpCode::IF);
	}

	bool validate(Tag<branch_table_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::BR_TABLE);
	}

	bool validate(Tag<loop_immediate_type>) const
	{
		_assert_invariants();
		return (opcode == OpCode::LOOP);
	}

	template <class T>
	void assert_valid(Tag<T>) const
	{
		assert(validate(d
	}

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, null_immediate_type null_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{null_immed}
	{ assert_valid(Tag<null_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, i32_immediate_type i32_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{i32_immed}
	{ assert_valid(Tag<i32_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, i64_immediate_type i64_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{i64_immed}
	{ assert_valid(Tag<i64_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, f32_immediate_type f32_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{f32_immed}
	{ asser_valid(Tag<f32_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, f64_immediate_type f64_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{f64_immed}
	{ assert_valid(Tag<f64_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, offset_immediate_type offset_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{offset_immed}
	{ assert_valid(Tag<offset_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, loop_immediate_type loop_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{loop_immed}
	{ assert_valid(Tag<loop_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, memory_immediate_type memory_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{memory_immed}
	{ assert_valid(Tag<memory_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, block_immediate_type block_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{block_immed}
	{ assert_valid(Tag<loop_immediate_type>{}); }

	WasmInstruction(std::string_view src, OpCode op, const char* end_pos, branch_table_immediate_type br_table_immed):
		source(src), opcode(op), end(end_pos), raw_immediate_{br_table_immed}
	{ assert_valid(Tag<branch_table_immediate_type>{}); }

public:

	const immediate_type& raw_immediate() const
	{ return raw_immediate_; }

	tagged_immediate_type tagged_immediate() const
	{
		// This if() covers 20-something ops.
		if(opcode >= OpCode::I32_LOAD and opcode <= I64_STORE32)
		{
			return tagged_immediate_type(
				std::in_place_type<memory_immediate_type>,
				raw().memory_immed
			);
		}
		switch(opcode)
		{
		// case for ops with no immediate
		default:
			assert(opcode_exists(static_cast<std::size_t>(opcode)));
			return tagged_immediate_type(
				std::in_place_type<null_immediate_type>,
				raw().null_immed
			);
		// cases for block immediate
		case OpCode::BLOCK: [[fallthrough]];
		case OpCode::IF:
			return tagged_immediate_type(
				std::in_place_type<block_immediate_type>,
				raw().block_immed
			);
		// cases for uint32 immediate
		case OpCode::ELSE:          [[fallthrough]];
		case OpCode::BR:            [[fallthrough]];
		case OpCode::BR_IF:         [[fallthrough]];
		case OpCode::CALL:          [[fallthrough]];
		case OpCode::CALL_INDIRECT: [[fallthrough]];
		case OpCode::GET_LOCAL:     [[fallthrough]];
		case OpCode::SET_LOCAL:     [[fallthrough]];
		case OpCode::TEE_LOCAL:     [[fallthrough]];
		case OpCode::GET_GLOBAL:    [[fallthrough]];
		case OpCode::SET_GLOBAL:
			return tagged_immediate_type(
				std::in_place_type<offset_immediate_type>,
				raw().offset_immed
			);
		// case for loop immediate
		case OpCode::LOOP:
			return tagged_immediate_type(
				std::in_place_type<loop_immediate_type>,
				raw().loop_immed
			);
		// case for loop immediate
		case OpCode::LOOP:
			return tagged_immediate_type(
				std::in_place_type<loop_immediate_type>,
				raw().loop_immed
			);
		// case for br_table immediate
		case OpCode::BR_TABLE:
			return tagged_immediate_type(
				std::in_place_type<branch_table_immediate_type>,
				raw().br_table_immed
			);
		// case for i32.const immediate
		case OpCode::I32_CONST:
			return tagged_immediate_type(
				std::in_place_type<i32_immediate_type>,
				raw().i32_immed
			);
		// case for i64.const immediate
		case OpCode::I64_CONST:
			return tagged_immediate_type(
				std::in_place_type<i64_immediate_type>,
				raw().i64_immed
			);
		// case for f32.const immediate
		case OpCode::F32_CONST:
			return tagged_immediate_type(
				std::in_place_type<f32_immediate_type>,
				raw().f32_immed
			);
		// case for f64.const immediate
		case OpCode::F64_CONST:
			return tagged_immediate_type(
				std::in_place_type<f64_immediate_type>,
				raw().f64_immed
			);
		}
		assert(false);
	}

	
	OpCode opcode() const
	{ return opcode_; }

	std::string_view source() const
	{ return source_; }

	std::string_view full_source() const
	{ return std::string_view(source().data(), end_ - source().data()); }

	const char* end_pos() const
	{ return end_; }

	template <class CallStackType>
	WasmInstruction execute(WasmModule& module, CallStackType& call_stack) 
	{
		switch(opcode)
		{
		case OpCode::UNREACHABLE:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::UNREACHABLE>();
			break;
		case OpCode::NOP:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::NOP>();
			break;
		case OpCode::BLOCK:
			assert_valid(Tag<block_immediate_type>{});
			op_func<OpCode::BLOCK>();
			break;
		case OpCode::LOOP:
			assert_valid(Tag<loop_immediate_type>{});
			op_func<OpCode::LOOP>();
			break;
		case OpCode::IF:
			assert_valid(Tag<block_immediate_type>{});
			op_func<OpCode::IF>();
			break;
		case OpCode::ELSE:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::ELSE>();
			break;
		case OpCode::END:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::END>();
			break;
		case OpCode::BR:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::BR>();
			break;
		case OpCode::BR_IF:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::BR_IF>();
			break;
		case OpCode::BR_TABLE:
			assert_valid(Tag<branch_table_immediate_type>{});
			op_func<OpCode::BR_TABLE>();
			break;
		case OpCode::RETURN:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::RETURN>();
			break;
		case OpCode::CALL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::CALL>();
			break;
		case OpCode::CALL_INDIRECT:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::CALL_INDIRECT>();
			break;
		case OpCode::DROP:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::DROP>(current_frame(call_stack));
			break;
		case OpCode::SELECT:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::SELECT>(current_frame(call_stack));
			break;
		case OpCode::GET_LOCAL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::GET_LOCAL>(
				current_frame(call_stack), raw_immediate().offset_immed, 
			);
			break;
		case OpCode::SET_LOCAL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::SET_LOCAL>(
				current_frame(call_stack), raw_immediate().offset_immed, 
			);
			break;
		case OpCode::TEE_LOCAL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::TEE_LOCAL>(
				current_frame(call_stack), raw_immediate().offset_immed
			);
			break;
		case OpCode::GET_GLOBAL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::GET_GLOBAL>(
				current_frame(call_stack), module, raw_immediate().offset_immed, 
			);
			break;
		case OpCode::SET_GLOBAL:
			assert_valid(Tag<offset_immediate_type>{});
			op_func<OpCode::SET_GLOBAL>(
				current_frame(call_stack), module, raw_immediate().offset_immed, 
			);
			break;

		/// MEMORY OPS
		// load
		case OpCode::I32_LOAD:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_LOAD>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::F32_LOAD:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::F32_LOAD>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::F64_LOAD:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::F64_LOAD>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// i32 extending loads
		case OpCode::I32_LOAD8_S:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_LOAD8_S>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I32_LOAD8_U:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_LOAD8_U>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I32_LOAD16_S:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_LOAD16_S>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I32_LOAD16_U:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_LOAD16_U>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// i64 extending loads
		case OpCode::I64_LOAD8_S:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD8_S>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD8_U:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD8_U>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD16_S:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD16_S>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD16_U:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD16_U>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD32_S:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD32_S>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_LOAD32_U:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_LOAD32_U>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// store
		case OpCode::I32_STORE:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_STORE>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_STORE:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_STORE>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::F32_STORE:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::F32_STORE>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::F64_STORE:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::F64_STORE>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// i32 wrapping stores 
		case OpCode::I32_STORE8:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_STORE8>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I32_STORE16:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I32_STORE16>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// i64 wrapping stores 
		case OpCode::I64_STORE8:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_STORE8>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_STORE16:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_STORE16>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		case OpCode::I64_STORE32:
			assert_valid(Tag<memory_immediate_type>{});
			const auto& immed = raw_immediate().memory_immed;
			op_func<OpCode::I64_STORE32>(
				current_frame(call_stack), module, flags(immed), offset(immed)
			);
			break;
		// misc
		case OpCode::CURRENT_MEMORY:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::CURRENT_MEMORY>(current_frame(call_stack), module);
			break;
		case OpCode::GROW_MEMORY:
			assert_valid(Tag<null_immediate_type>{});
			op_func<OpCode::GROW_MEMORY>(current_frame(call_stack), module);
			break;
		/// CONST OPERATIONS
		case OpCode::I32_CONST:
			assert_valid(Tag<i32_immediate_type>{});
			op_func<OpCode::I32_CONST>(current_frame(call_stack), raw_immediate().i32_immed);
			break;
		case OpCode::I64_CONST:
			assert_valid(Tag<i64_immediate_type>{});
			op_func<OpCode::I64_CONST>(current_frame(call_stack), raw_immediate().i64_immed);
			break;
		case OpCode::F32_CONST:
			assert_valid(Tag<f32_immediate_type>{});
			op_func<OpCode::F32_CONST>(current_frame(call_stack), raw_immediate().f32_immed);
			break;
		case OpCode::F64_CONST:
			assert_valid(Tag<f64_immediate_type>{});
			op_func<OpCode::F64_CONST>(current_frame(call_stack), raw_immediate().f32_immed);
			break;
		/// COMPARISON OPERATIONS
		// i32 comparisons
		case OpCode::I32_EQZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_EQZ>(current_frame(call_stack));
			break;
		case OpCode::I32_EQ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_EQ>(current_frame(call_stack));
			break;
		case OpCode::I32_NE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_NE>(current_frame(call_stack));
			break;
		case OpCode::I32_LT_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_LT_S>(current_frame(call_stack));
			break;
		case OpCode::I32_LT_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_LT_U>(current_frame(call_stack));
			break;
		case OpCode::I32_GT_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_GT_S>(current_frame(call_stack));
			break;
		case OpCode::I32_GT_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_GT_U>(current_frame(call_stack));
			break;
		case OpCode::I32_LE_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_LE_S>(current_frame(call_stack));
			break;
		case OpCode::I32_LE_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_LE_U>(current_frame(call_stack));
			break;
		case OpCode::I32_GE_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_GE_S>(current_frame(call_stack));
			break;
		case OpCode::I32_GE_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_GE_U>(current_frame(call_stack));
			break;
		// i64 comparisons
		case OpCode::I64_EQZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_EQZ>(current_frame(call_stack));
			break;
		case OpCode::I64_EQ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_EQ>(current_frame(call_stack));
			break;
		case OpCode::I64_NE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_NE>(current_frame(call_stack));
			break;
		case OpCode::I64_LT_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_LT_S>(current_frame(call_stack));
			break;
		case OpCode::I64_LT_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_LT_U>(current_frame(call_stack));
			break;
		case OpCode::I64_GT_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_GT_S>(current_frame(call_stack));
			break;
		case OpCode::I64_GT_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_GT_U>(current_frame(call_stack));
			break;
		case OpCode::I64_LE_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_LE_S>(current_frame(call_stack));
			break;
		case OpCode::I64_LE_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_LE_U>(current_frame(call_stack));
			break;
		case OpCode::I64_GE_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_GE_S>(current_frame(call_stack));
			break;
		case OpCode::I64_GE_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_GE_U>(current_frame(call_stack));
			break;
		// f32 comparisons
		case OpCode::F32_EQ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_EQ>(current_frame(call_stack));
			break;
		case OpCode::F32_NE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_NE>(current_frame(call_stack));
			break;
		case OpCode::F32_LT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_LT>(current_frame(call_stack));
			break;
		case OpCode::F32_GT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_GT>(current_frame(call_stack));
			break;
		case OpCode::F32_LE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_LE>(current_frame(call_stack));
			break;
		case OpCode::F32_GE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_GE>(current_frame(call_stack));
			break;
		// f64 comparisons
		case OpCode::F64_EQ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_EQ>(current_frame(call_stack));
			break;
		case OpCode::F64_NE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_NE>(current_frame(call_stack));
			break;
		case OpCode::F64_LT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_LT>(current_frame(call_stack));
			break;
		case OpCode::F64_GT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_GT>(current_frame(call_stack));
			break;
		case OpCode::F64_LE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_LE>(current_frame(call_stack));
			break;
		case OpCode::F64_GE:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_GE>(current_frame(call_stack));
			break;
		/// NUMERIC OPERATIONS
		// i32 operations
		case OpCode::I32_CLZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_CLZ>();
			break;
		case OpCode::I32_CTZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_CTZ>();
			break;
		case OpCode::I32_POPCNT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_POPCNT>();
			break;
		case OpCode::I32_ADD:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_ADD>();
			break;
		case OpCode::I32_SUB:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_SUB>();
			break;
		case OpCode::I32_MUL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_MUL>();
			break;
		case OpCode::I32_DIV_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_DIV_S>();
			break;
		case OpCode::I32_DIV_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_DIV_U>();
			break;
		case OpCode::I32_REM_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_REM_S>();
			break;
		case OpCode::I32_REM_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_REM_U>();
			break;
		case OpCode::I32_AND:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_AND>();
			break;
		case OpCode::I32_OR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_OR>();
			break;
		case OpCode::I32_XOR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_XOR>();
			break;
		case OpCode::I32_SHL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_SHL>();
			break;
		case OpCode::I32_SHR_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_SHR_S>();
			break;
		case OpCode::I32_SHR_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_SHR_U>();
			break;
		case OpCode::I32_ROTL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_ROTL>();
			break;
		case OpCode::I32_ROTR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_ROTR>();
			break;
		// i64 operations
		case OpCode::I64_CLZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_CLZ>();
			break;
		case OpCode::I64_CTZ:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_CTZ>();
			break;
		case OpCode::I64_POPCNT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_POPCNT>();
			break;
		case OpCode::I64_ADD:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_ADD>();
			break;
		case OpCode::I64_SUB:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_SUB>();
			break;
		case OpCode::I64_MUL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_MUL>();
			break;
		case OpCode::I64_DIV_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_DIV_S>();
			break;
		case OpCode::I64_DIV_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_DIV_U>();
			break;
		case OpCode::I64_REM_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_REM_S>();
			break;
		case OpCode::I64_REM_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_REM_U>();
			break;
		case OpCode::I64_AND:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_AND>();
			break;
		case OpCode::I64_OR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_OR>();
			break;
		case OpCode::I64_XOR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_XOR>();
			break;
		case OpCode::I64_SHL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_SHL>();
			break;
		case OpCode::I64_SHR_S:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_SHR_S>();
			break;
		case OpCode::I64_SHR_U:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_SHR_U>();
			break;
		case OpCode::I64_ROTL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_ROTL>();
			break;
		case OpCode::I64_ROTR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_ROTR>();
			break;
		// f32 operations
		case OpCode::F32_ABS:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_ABS>();
			break;
		case OpCode::F32_NEG:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_NEG>();
			break;
		case OpCode::F32_CEIL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_CEIL>();
			break;
		case OpCode::F32_FLOOR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_FLOOR>();
			break;
		case OpCode::F32_TRUNC:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_TRUNC>();
			break;
		case OpCode::F32_NEAREST:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_NEAREST>();
			break;
		case OpCode::F32_SQRT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_SQRT>();
			break;
		case OpCode::F32_ADD:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_ADD>();
			break;
		case OpCode::F32_SUB:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_SUB>();
			break;
		case OpCode::F32_MUL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_MUL>();
			break;
		case OpCode::F32_DIV:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_DIV>();
			break;
		case OpCode::F32_MIN:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_MIN>();
			break;
		case OpCode::F32_MAX:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_MAX>();
			break;
		case OpCode::F32_COPYSIGN:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_COPYSIGN>();
			break;
		// f64 operations
		case OpCode::F64_ABS:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_ABS>();
			break;
		case OpCode::F64_NEG:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_NEG>();
			break;
		case OpCode::F64_CEIL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_CEIL>();
			break;
		case OpCode::F64_FLOOR:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_FLOOR>();
			break;
		case OpCode::F64_TRUNC:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_TRUNC>();
			break;
		case OpCode::F64_NEAREST:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_NEAREST>();
			break;
		case OpCode::F64_SQRT:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_SQRT>();
			break;
		case OpCode::F64_ADD:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_ADD>();
			break;
		case OpCode::F64_SUB:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_SUB>();
			break;
		case OpCode::F64_MUL:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_MUL>();
			break;
		case OpCode::F64_DIV:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_DIV>();
			break;
		case OpCode::F64_MIN:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_MIN>();
			break;
		case OpCode::F64_MAX:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_MAX>();
			break;
		case OpCode::F64_COPYSIGN:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_COPYSIGN>();
			break;
		/// CONVERSION OPERATIONS
		case OpCode::I32_WRAP_I64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_WRAP_I64>();
			break;
		// float-to-int32 tuncating conversion
		case OpCode::I32_TRUNC_S_F32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_TRUNC_S_F32>();
			break;
		case OpCode::I32_TRUNC_U_F32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_TRUNC_U_F32>();
			break;
		case OpCode::I32_TRUNC_S_F64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_TRUNC_S_F64>();
			break;
		case OpCode::I32_TRUNC_U_F64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I32_TRUNC_U_F64>();
			break;
		// int32-to-int64 extending conversion
		case OpCode::I64_EXTEND_S_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_EXTEND_S_I32>();
			break;
		case OpCode::I64_EXTEND_U_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_EXTEND_U_I32>();
			break;
		// float-to-int64 truncating conversion
		case OpCode::I64_TRUNC_S_F32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_TRUNC_S_F32>();
			break;
		case OpCode::I64_TRUNC_U_F32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_TRUNC_U_F32>();
			break;
		case OpCode::I64_TRUNC_S_F64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_TRUNC_S_F64>();
			break;
		case OpCode::I64_TRUNC_U_F64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::I64_TRUNC_U_F64>();
			break;
		// int-to-float32 conversion
		case OpCode::F32_CONVERT_S_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_CONVERT_S_I32>();
			break;
		case OpCode::F32_CONVERT_U_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_CONVERT_U_I32>();
			break;
		case OpCode::F32_CONVERT_S_I64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_CONVERT_S_I64>();
			break;
		case OpCode::F32_CONVERT_U_I64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_CONVERT_U_I64>();
			break;
		// float64-to-float32 demoting conversion
		case OpCode::F32_DEMOTE_F64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F32_DEMOTE_F64>();
			break;
		// int-to-float64 conversion
		case OpCode::F64_CONVERT_S_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_CONVERT_S_I32>();
			break;
		case OpCode::F64_CONVERT_U_I32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_CONVERT_U_I32>();
			break;
		case OpCode::F64_CONVERT_S_I64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_CONVERT_S_I64>();
			break;
		case OpCode::F64_CONVERT_U_I64:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_CONVERT_U_I64>();
			break;
		// float32-to-float64 promoting conversion
		case OpCode::F64_PROMOTE_F32:
			assert_valid(Tag<null_immediate_tag>{});
			op_func<OpCode::F64_PROMOTE_F32>();
			break;
		}
	}
	
	
	
private:
	
	void _assert_invariants() const
	{
		assert(opcode_exists(opcode()));
		assert(end_pos() > source().data());
		assert(source().size() > 0u);
		std::visit(
			[](const auto& immed) {
				using immed_type = std::decay_t<decltype(immed)>;
				assert(validate(Tag<immed_type>{}));
			},
			tagged_immediate()
		);
		
	}

	std::pair<CodeView, const char*> jump_over_if() const
	{
		assert_valid(Tag<block_immediate_type>{});
		assert(opcode() == OpCode::IF);
		wasm_uint32_t ofs = raw_immediate().block_immed;
		assert(ofs > 0u);
		assert(end_pos() > source().data());
		auto dist = static_cast<std::size_t>(end_pos() - source().data());
		assert(ofs < dist);
		auto dest_pos = source().data() + ofs;
		constexpr auto end_op = static_cast<unsigned char>(OpCode::END);
		constexpr auto else_op = static_cast<unsigned char>(OpCode::ELSE);
		if(dest_pos[-1] == end_op)
		{
			return std::make_pair(CodeView(dest_pos, end_pos()), nullptr);
		}
		else 
		{
			assert(dest_pos[-(1 + static_cast<std::ptrdiff_t>(sizeof(ofs)))] == else_op);
			auto else_pos = dest_pos - (1 + static_cast<std::ptrdiff_t>(sizeof(ofs)));
			std::memcpy(&ofs, else_pos + 1, sizeof(ofs));
			return std::make_pair(CodeView(dest_pos, end_pos()), else_pos + ofs);
		}
	}

	CodeView branch_to(const char* pos) const
	{
		assert(pos[-1] == static_cast<char>(OpCode::END));
		assert(pos > source().data());
		assert(pos < end_pos());
		return CodeView(pos, end_pos());
	}

	CodeView jump_over_else() const
	{
		assert_valid(Tag<offset_immediate_tag>{});
		assert(opcode() == OpCode::ELSE);
		wasm_uint32_t ofs = raw_immediate().block_immed;
		assert(ofs > 0u);
		assert(end_pos() > source().data());
		auto dist = static_cast<std::size_t>(end_pos() - source().data());
		assert(ofs < dist);
		auto dest_pos = source().data() + ofs;
		assert(dest_pos[-1] == static_cast<char>(OpCode::END));
		return CodeView(dest_pos, end_pos());
	}

	CodeView remaining_code() {
		assert(source().end() < end_pos());
		assert(opcode() != OpCode::IF);
		assert(opcode() != OpCode::ELSE);
		assert(opcode() != OpCode::BR);
		assert(opcode() != OpCode::BR_IF);
		assert(opcode() != OpCode::BR_TABLE);
		return CodeView(source().end(), end_pos());
	}

	/// Byte sequence from which this instruction was decoded.
	const std::string_view source_;
	/// Opcode for this instruction.
	const OpCode opcode_;
	/// Past-the-end pointer into the code from which this instruction was decoded
	const char* const end_;
	/// Raw union of possible immediate operand alternatives, discriminated by 'this->opcode'.
	const immediate_type raw_immediate_;
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
		code_(code(func))
	{
		
	}

	
	

	OpCode current_op() const
	{
		assert(ready());
		return static_cast<WasmInstruction>(code_.front());
	}
	
	bool done() const
	{ return not code_.empty(); }

	WasmInstruction next_instruction() const
	{
		assert(ready());
		return visit_opcode(InstructionVisitor{}, code_.data(), code_.data() + code_.size());
	}

	void advance(const WasmInstruction& instr)
	{
		assert(not done());
		assert(instr.source.data() == code_.data());
		instr._assert_invariants();
		assert(instr.source.size() <= code_.size());
		code.remove_prefix(instr.source.size());
	}

	bool ready() const
	{
		if(done())
			return false;
		assert(opcode_exists(code_.front()));
		return true;
	}
	std::string_view code_;
};


} /* namespace opc */
} /* namespace wasm */





#endif /* WASM_INSTRUCTION_H */
