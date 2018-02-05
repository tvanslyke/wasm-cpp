#ifndef WASM_INSTRUCTION_H
#define WASM_INSTRUCTION_H
#include <cstddef>
#include <cstdint>
#include <array>


namespace wasm_opcode {

using wasm_opcode_t = std::uint_least8_t;
enum wasm_instruction: wasm_opcode_t {
	
	// BLOCK INSTRUCTIONS
	BLOCK			= 0x02, 
	LOOP			= 0x03,
	BR			= 0x0c,
	BR_IF			= 0x0d,
	BR_TABLE		= 0x0e,
	IF			= 0x04,
	ELSE			= 0x05,
	END			= 0x0b,
	RETURN			= 0x0f,
	UNREACHABLE		= 0x00,

	// BASIC INSTRUCTIONS
	NOP 			= 0x01,
	DROP 			= 0x1a,
	I32_CONST 		= 0x41,
	I64_CONST 		= 0x42,
	F32_CONST 		= 0x43,
	F64_CONST 		= 0x44,
	GET_LOCAL 		= 0x20,
	SET_LOCAL 		= 0x21,
	TEE_LOCAL 		= 0x22,
	GET_GLOBAL 		= 0x23,
	SET_GLOBAL 		= 0x24,
	SELECT 			= 0x1b,
	CALL 			= 0x10,
	CALL_INDIRECT 		= 0x11,
	
	// INTEGER ARITHMETIC INSTRUCTIONS
	// int32
	I32_ADD			= 0x6a,
	I32_SUB			= 0x6b,
	I32_MUL			= 0x6c,
	I32_DIV_S		= 0x6d,
	I32_DIV_U		= 0x6e,
	I32_REM_S		= 0x6f,
	I32_REM_U		= 0x70,
	I32_AND			= 0x71,
	I32_OR			= 0x72,
	I32_XOR			= 0x73,
	I32_SHL			= 0x74,
	I32_SHR_S		= 0x75,
	I32_SHR_U		= 0x76,
	I32_ROTL		= 0x77,
	I32_ROTR		= 0x78,
	I32_CLZ			= 0x67,
	I32_CTZ			= 0x68,
	I32_POPCNT		= 0x69,
	I32_EQZ			= 0x45,
	// int64
	I64_ADD			= 0x7c,
	I64_SUB			= 0x7d,
	I64_MUL			= 0x7e,
	I64_DIV_S		= 0x7f,
	I64_DIV_U		= 0x80,
	I64_REM_S		= 0x81,
	I64_REM_U		= 0x82,
	I64_AND			= 0x83,
	I64_OR			= 0x84,
	I64_XOR			= 0x85,
	I64_SHL			= 0x86,
	I64_SHR_S		= 0x87,
	I64_SHR_U		= 0x88,
	I64_ROTL		= 0x89,
	I64_ROTR		= 0x8a,
	I64_CLZ			= 0x79,
	I64_CTZ			= 0x7a,
	I64_POPCNT		= 0x7b,
	I64_EQZ			= 0x50,

	// FLOATING POINT ARITHMETIC INSTRUCTIONS
	// float32
	F32_ADD			= 0x92, 
	F32_SUB	    		= 0x93,    
	F32_MUL	    		= 0x94,    
	F32_DIV	    		= 0x95,    
	F32_SQRT    		= 0x91,    
	F32_MIN	    		= 0x96,    
	F32_MAX	    		= 0x97,    
	F32_CEIL    		= 0x8d,    
	F32_FLOOR   		= 0x8e,    
	F32_TRUNC   		= 0x8f,    
	F32_NEAREST 		= 0x90,    
	F32_ABS	    		= 0x8b,    
	F32_NEG	    		= 0x8c,    
	F32_COPYSIGN		= 0x98,	
	// float64
	F64_ADD	    		= 0xa0,    
	F64_SUB	    		= 0xa1,    
	F64_MUL	    		= 0xa2,    
	F64_DIV	    		= 0xa3,    
	F64_SQRT    		= 0x9f,    
	F64_MIN	    		= 0xa4,    
	F64_MAX	    		= 0xa5,    
	F64_CEIL    		= 0x9b,    
	F64_FLOOR   		= 0x9c,    
	F64_TRUNC   		= 0x9d,    
	F64_NEAREST 		= 0x9e,    
	F64_ABS	    		= 0x99,    
	F64_NEG	    		= 0x9a,    
	F64_COPYSIGN		= 0xa6,	

	// INTEGER COMPARISON INSTRUCTIONS
	// int32
	I32_EQ  		= 0x46,
	I32_NE  		= 0x47,
	I32_LT_S		= 0x48,
	I32_LT_U		= 0x49,
	I32_GT_S		= 0x4a,
	I32_GT_U		= 0x4b,
	I32_LE_S		= 0x4c,
	I32_LE_U		= 0x4d,
	I32_GE_S		= 0x4e,
	I32_GE_U		= 0x4f,
	// int64
	I64_EQ  		= 0x51,
	I64_NE  		= 0x52,
	I64_LT_S		= 0x53,
	I64_LT_U		= 0x54,
	I64_GT_S		= 0x55,
	I64_GT_U		= 0x56,
	I64_LE_S		= 0x57,
	I64_LE_U		= 0x58,
	I64_GE_S		= 0x59,
	I64_GE_U		= 0x5a,
	
	// FLOATING POINT COMPARISON INSTRUCTIONS
	// float32
	F32_EQ  		= 0x5b,
	F32_NE  		= 0x5c,
	F32_LT			= 0x5d,
	F32_GT			= 0x5e,
	F32_LE			= 0x5f,
	F32_GE			= 0x60,
	// float64
	F64_EQ  		= 0x61,
	F64_NE  		= 0x62,
	F64_LT			= 0x63,
	F64_GT			= 0x64,
	F64_LE			= 0x65,
	F64_GE			= 0x66,


	// CONVERSION INSTRUCTIONS
	// to int32
	I32_WRAP		= 0xa7,
	I32_TRUNC_F32_S		= 0xa8,
	I32_TRUNC_F32_U		= 0xa9,
	I32_TRUNC_F64_S		= 0xaa,
	I32_TRUNC_F64_U		= 0xab,
	I32_REINTERPRET_F32	= 0xbc,
	
	// to int64
	I64_EXTEND_S 		= 0xac,
	I64_EXTEND_U		= 0xad,
	I64_TRUNC_F32_S		= 0xae,
	I64_TRUNC_F32_U		= 0xaf,
	I64_TRUNC_F64_S		= 0xb0,
	I64_TRUNC_F64_U		= 0xb1,
	I64_REINTERPRET_F64	= 0xbd,

	// to float32
	F32_DEMOTE              = 0xb6,
	F32_CONVERT_I32_S       = 0xb2,
	F32_CONVERT_I32_U       = 0xb3,
	F32_CONVERT_I64_S       = 0xb4,
	F32_CONVERT_I64_U       = 0xb5,
	F32_REINTERPRET_I32     = 0xbe,

	// to float64
	F64_PROMOTE             = 0xbb,
	F64_CONVERT_I32_S       = 0xb7,
	F64_CONVERT_I32_U       = 0xb8,
	F64_CONVERT_I64_S       = 0xb9,
	F64_CONVERT_I64_U       = 0xba,
	F64_REINTERPRET_I64     = 0xbf,


	// LOAD AND STORE INSTRUCTIONS

	I32_LOAD		= 0x28,
	I64_LOAD		= 0x29,
	F32_LOAD		= 0x2a,
	F64_LOAD		= 0x2b,
	I32_LOAD8_S		= 0x2c,
	I32_LOAD8_U		= 0x2d,
	I32_LOAD16_S		= 0x2e,
	I32_LOAD16_U		= 0x2f,
	I64_LOAD8_S		= 0x30,
	I64_LOAD8_U		= 0x31,
	I64_LOAD16_S		= 0x32,
	I64_LOAD16_U		= 0x33,
	I64_LOAD32_S		= 0x34,
	I64_LOAD32_U		= 0x35,
	I32_STORE		= 0x36,
	I64_STORE		= 0x37,
	F32_STORE		= 0x38,
	F64_STORE		= 0x39,
	I32_STORE8		= 0x3a,
	I32_STORE16		= 0x3b,
	I64_STORE8		= 0x3c,
	I64_STORE16		= 0x3d,
	I64_STORE32		= 0x3e,

	// MEMORY INSTRUCTIONS
	GROW_MEMORY 		= 0x40,
	CURRENT_MEMORY 		= 0x3f,
};

} /* namespace wasm_opcode */

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

static bool wasm_instruction_dne(wasm_opcode::wasm_opcode_t opcode)
{
	// most compilers should be able to optimize this very well.
	// very brittle function; sensitive to changes in spec
	return (opcode > 191) 
		or ((opcode > 17) and (opcode < 26))
		or ((opcode >  5) and (opcode < 11))
		or ((opcode > 27) and (opcode < 32))
		or ((opcode > 36) and (opcode < 40));

}




#endif /* WASM_INSTRUCTION_H */
