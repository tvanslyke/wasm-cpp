import cparse
import array

code = bytearray([1, 2, 3, 4, 5, 6, 7])
funcs = array.array('I', [1, 2, 3, 4, 5, 6, 7])
tables = array.array('I', [1, 2, 3, 4, 5, 6, 7])
mems = array.array('I', [1, 2, 3, 4, 5, 6, 7])
globs = array.array('I', [1, 2, 3, 4, 5, 6, 7])

cparse.finalize_code(code, funcs, tables, mems, globs)
