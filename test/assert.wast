;; assert function

(module
	(func $assert (export "assert") (param i32)
		get_local 0 
		if 
			unreachable
		end
	)
) 
