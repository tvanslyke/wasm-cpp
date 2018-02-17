#ifndef FRONTEND_GLUE_READ_PROGRAM_DEF_H
#define FRONTEND_GLUE_READ_PROGRAM_DEF_H

struct wasm_program_state;
extern "C" struct PyObject;

wasm_program_state read_program_def(PyObject* program_def)

#endif /* FRONTEND_GLUE_READ_PROGRAM_DEF_H */
