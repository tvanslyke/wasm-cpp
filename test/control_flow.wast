;; assert function

(module 
	(import "assert" "assert" (func $assert (param i32)))
	(func $main (export "main") (local i32)
		i32.const 1
		i32.const 2
		i32.ne
		if (result f32)
			i32.const 100
			f32.convert_s/i32
		else
			unreachable
		end
		f32.const 100.0
		f32.eq
		call $assert
		i32.const 10
		set_local 0 
		loop
			get_local 0 
			get_local 0 
			i32.const 1
			i32.sub
			set_local 0
			br_if 0
			get_local 0 
			i32.eqz
			call $assert
		end
	)
) 
