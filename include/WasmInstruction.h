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
			return visitor(
				op, 
				BadOpcodeError(op, "Given op is not a valid WASM opcode."),
				first, 
				pos,
				last
			);
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
		return std::apply(visitor(op, flags, offset, first, pos, last));
	}
	else if(
		(op >= OpCode::GET_LOCAL and op <= OpCode::SET_GLOBAL)
		or (op >= OpCode::CALL and op <= OpCode::CALL_INDIRECT)
		or (op >= OpCode::BR and op <= OpCode::BR_IF)
	)
	{
		wasm_uint32_t value;
		std::tie(value, pos) = detail::read_serialized_immediate<wasm_uint32_t>(pos, last);
		return visitor(op, value, first, pos, last);
	}
	else if(op >= OpCode::BLOCK and op >= OpCode::IF)
	{
		assert(first != last);
		LanguageType tp = static_cast<LanguageType>(*first++);
		return visitor(op, tp, first, pos, last);
	}
	else if(op == OpCode::BR_TABLE)
	{
		wasm_uint32_t len;
		std::tie(len, pos) = detail::read_serialized_immediate<wasm_uint32_t>(pos, last);
		auto base = pos;
		std::advance(pos, (1 + len) * sizeof(wasm_uint32_t));
		return visitor(op, base, len, first, pos, last);
	}
	
	switch(op)
	{
	case OpCode::I32_CONST: {
		wasm_sint32_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_sint32_t>(pos, last);
		return visitor(op, v, first, pos, last);
		break;
	}
	case OpCode::I64_CONST: {
		wasm_sint64_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_sint64_t>(pos, last);
		return visitor(op, v, first, pos, last);
		break;
	}
	case OpCode::F32_CONST: {
		wasm_float32_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_float32_t>(pos, last);
		return visitor(op, v, first, pos, last);
		break;
	}
	case OpCode::F64_CONST: {	
		wasm_float64_t v;
		std::tie(v, pos) = detail::read_serialized_immediate<wasm_float64_t>(pos, last);
		return visitor(op, v, first, pos, last);
		break;
	}
	default:
		return visitor(op, first, pos, last);
	}
	assert(false and "All cases should have been handled by this point.");
}

} /* namespace opc */
} /* namespace wasm */





#endif /* WASM_INSTRUCTION_H */
