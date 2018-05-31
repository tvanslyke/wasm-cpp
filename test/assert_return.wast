(module
 (export "assert_return" (func $assert_return))
 (func $assert_return (param f32 f32)
    get_local 0
    get_local 1
    f32.ne
    if
      unreachable
    end
 )
)
