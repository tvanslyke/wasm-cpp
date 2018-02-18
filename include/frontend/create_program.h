#ifndef FRONTEND_GLUE_CREATE_PROGRAM_H
#define FRONTEND_GLUE_CREATE_PROGRAM_H

struct wasm_program_state;

wasm_program_state create_program(int argc, const char** argv);

#endif /* FRONTEND_GLUE_CREATE_PROGRAM_H */
