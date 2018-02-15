
Block			= 0X02 
Loop			= 0X03
Br			= 0X0C
Br_if			= 0X0D
Br_table		= 0X0E
If			= 0X04
Else			= 0X05
End			= 0X0B
Return			= 0X0F
Unreachable		= 0X00

# basic instructions
Nop 			= 0X01
Drop 			= 0X1A
I32_const 		= 0X41
I64_const 		= 0X42
F32_const 		= 0X43
F64_const 		= 0X44
Get_local 		= 0X20
Set_local 		= 0X21
Tee_local 		= 0X22
Get_global 		= 0X23
Set_global 		= 0X24
Select 			= 0X1B
Call 			= 0X10
Call_indirect 		= 0X11

# integer arithmetic instructions
# INT32
I32_add			= 0X6A
I32_sub			= 0X6B
I32_mul			= 0X6C
I32_div_s		= 0X6D
I32_div_u		= 0X6E
I32_rem_s		= 0X6F
I32_rem_u		= 0X70
I32_and			= 0X71
I32_or			= 0X72
I32_xor			= 0X73
I32_shl			= 0X74
I32_shr_s		= 0X75
I32_shr_u		= 0X76
I32_rotl		= 0X77
I32_rotr		= 0X78
I32_clz			= 0X67
I32_ctz			= 0X68
I32_popcnt		= 0X69
I32_eqz			= 0X45
# INT64
I64_add			= 0X7C
I64_sub			= 0X7D
I64_mul			= 0X7E
I64_div_s		= 0X7F
I64_div_u		= 0X80
I64_rem_s		= 0X81
I64_rem_u		= 0X82
I64_and			= 0X83
I64_or			= 0X84
I64_xor			= 0X85
I64_shl			= 0X86
I64_shr_s		= 0X87
I64_shr_u		= 0X88
I64_rotl		= 0X89
I64_rotr		= 0X8A
I64_clz			= 0X79
I64_ctz			= 0X7A
I64_popcnt		= 0X7B
I64_eqz			= 0X50

# floating point arithmetic instructions
# FLOAT32
F32_add			= 0X92
F32_sub	    		= 0X93
F32_mul	    		= 0X94
F32_div	    		= 0X95
F32_sqrt    		= 0X91
F32_min	    		= 0X96
F32_max	    		= 0X97
F32_ceil    		= 0X8D
F32_floor   		= 0X8E
F32_trunc   		= 0X8F
F32_nearest 		= 0X90
F32_abs	    		= 0X8B
F32_neg	    		= 0X8C
F32_copysign		= 0X98
# FLOAT64
F64_add	    		= 0XA0
F64_sub	    		= 0XA1
F64_mul	    		= 0XA2
F64_div	    		= 0XA3
F64_sqrt    		= 0X9F
F64_min	    		= 0XA4
F64_max	    		= 0XA5
F64_ceil    		= 0X9B
F64_floor   		= 0X9C
F64_trunc   		= 0X9D
F64_nearest 		= 0X9E
F64_abs	    		= 0X99
F64_neg	    		= 0X9A
F64_copysign		= 0XA6

# integer comparison instructions
# INT32
I32_eq  		= 0X46
I32_ne  		= 0X47
I32_lt_s		= 0X48
I32_lt_u		= 0X49
I32_gt_s		= 0X4A
I32_gt_u		= 0X4B
I32_le_s		= 0X4C
I32_le_u		= 0X4D
I32_ge_s		= 0X4E
I32_ge_u		= 0X4F
# INT64
I64_eq  		= 0X51
I64_ne  		= 0X52
I64_lt_s		= 0X53
I64_lt_u		= 0X54
I64_gt_s		= 0X55
I64_gt_u		= 0X56
I64_le_s		= 0X57
I64_le_u		= 0X58
I64_ge_s		= 0X59
I64_ge_u		= 0X5A

# floating point comparison instructions
# FLOAT32
F32_eq  		= 0X5B
F32_ne  		= 0X5C
F32_lt			= 0X5D
F32_gt			= 0X5E
F32_le			= 0X5F
F32_ge			= 0X60
# FLOAT64
F64_eq  		= 0X61
F64_ne  		= 0X62
F64_lt			= 0X63
F64_gt			= 0X64
F64_le			= 0X65
F64_ge			= 0X66


# conversion instructions
# TO INT32
I32_wrap		= 0XA7
I32_trunc_f32_s		= 0XA8
I32_trunc_f32_u		= 0XA9
I32_trunc_f64_s		= 0XAA
I32_trunc_f64_u		= 0XAB
I32_reinterpret_f32	= 0XBC

# TO INT64
I64_extend_s 		= 0XAC
I64_extend_u		= 0XAD
I64_trunc_f32_s		= 0XAE
I64_trunc_f32_u		= 0XAF
I64_trunc_f64_s		= 0XB0
I64_trunc_f64_u		= 0XB1
I64_reinterpret_f64	= 0XBD

# TO FLOAT32
F32_demote              = 0XB6
F32_convert_i32_s       = 0XB2
F32_convert_i32_u       = 0XB3
F32_convert_i64_s       = 0XB4
F32_convert_i64_u       = 0XB5
F32_reinterpret_i32     = 0XBE

# TO FLOAT64
F64_promote             = 0XBB
F64_convert_i32_s       = 0XB7
F64_convert_i32_u       = 0XB8
F64_convert_i64_s       = 0XB9
F64_convert_i64_u       = 0XBA
F64_reinterpret_i64     = 0XBF


# load and store instructions

I32_load		= 0X28
I64_load		= 0X29
F32_load		= 0X2A
F64_load		= 0X2B
I32_load8_s		= 0X2C
I32_load8_u		= 0X2D
I32_load16_s		= 0X2E
I32_load16_u		= 0X2F
I64_load8_s		= 0X30
I64_load8_u		= 0X31
I64_load16_s		= 0X32
I64_load16_u		= 0X33
I64_load32_s		= 0X34
I64_load32_u		= 0X35
I32_store		= 0X36
I64_store		= 0X37
F32_store		= 0X38
F64_store		= 0X39
I32_store8		= 0X3A
I32_store16		= 0X3B
I64_store8		= 0X3C
I64_store16		= 0X3D
I64_store32		= 0X3E

# memory instructions
Grow_memory 		= 0X40
Current_memory 		= 0X3F





