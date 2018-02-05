#include "wasm_call_stack.h"
#include "function/wasm_function_storage.h"

typedef struct wasm_frame_base_ {
	struct wasm_frame_* ret;
	const opcode_t* program_counter;
	const std::size_t locals_count;
} wasm_frame_base;

typedef struct wasm_frame_
{
	wasm_frame_base base;
	wasm_value_t[] locals;
} wasm_frame;



static const wasm_frame_base base_frame = {
	.ret = NULL,
	.program_counter = NULL,
	.locals_count = 0
};

static const size_t frame_align = alignof(wasm_frame);

static size_t WasmFrameBase_LocalsCount(const wasm_frame_base* self)
{
	return self->locals_count;
}

static const opcode_t* WasmFrameBase_Code(const wasm_frame_base* self)
{
	return self->program_counter;
};

static const opcode_t* WasmFrameBase_CodeNext(wasm_frame_base* self)
{
	return ++(self->program_counter);
};

static const opcode_t* WasmFrameBase_CodeAdvance(wasm_frame_base* self, size_t count)
{
	self->program_counter += count;
	return self->program_counter;
};

static const opcode_t* WasmFrameBase_CodeJump(wasm_frame_base* self, const opcode_t* label)
{
	const opcode_t* old = self->program_counter;
	self->program_counter = label;
	return old;
};

static wasm_frame* WasmFrameBase_ReturnAddress(wasm_frame_base* self)
{
	return self->ret;
}

static size_t WasmFrameBase_SizeOf(const wasm_frame_base* self)
{
	size_t next_frame_offset = sizeof(*self) + WasmFrameBase_LocalsCount(self) * sizeof(wasm_value_t);
	size_t align_err = (next_frame_offset % frame_align);
	if(align_err)
		align_err = frame_align - align_err;
	return next_frame_offset + align_err;
}

static wasm_frame_base* WasmFrame_Base(wasm_frame* self)
{
	return &(self->base);
}

static const wasm_frame_base* WasmFrame_ConstBase(const wasm_frame* self)
{
	return &(self->base);
}

int WasmFrame_InitFrameBase(void* self, size_t* total_bytes)
{
	const size_t frame_size = WasmFrameBase_SizeOf(&base_frame);
	if(*total_bytes < frame_size)
		return -1;
	memcpy(self, &base_frame, frame_size);
	*total_bytes -= frame_size;
	return 0;
};

size_t WasmFrame_LocalsCount(const wasm_frame* self)
{
	return WasmFrameBase_LocalsCount(WasmFrame_ConstBase(self));
};

const opcode_t* WasmFrame_Code(const wasm_frame* self)
{
	return WasmFrameBase_Code(WasmFrame_ConstBase(self));
};

const opcode_t* WasmFrame_CodeNext(wasm_frame* self)
{
	return WasmFrameBase_CodeNext(WasmFrame_Base(self));
};

const opcode_t* WasmFrame_CodeAdvance(wasm_frame* self, size_t count)
{
	return WasmFrameBase_CodeAdvance(WasmFrame_Base(self));
};

static wasm_frame* WasmFrame_ReturnAddress(wasm_frame* self)
{
	return WasmFrameBase_ReturnAddress(WasmFrame_Base(self));
}

size_t WasmFrame_SizeOf(const wasm_frame* self)
{
	size_t next_frame_offset = sizeof(*self) + WasmFrame_LocalsCount(self) * sizeof(wasm_value_t);
	size_t align_err = (next_frame_offset % frame_align);
	if(align_err)
		align_err = frame_align - align_err;
	return next_frame_offset + align_err;
}

wasm_frame* WasmFrame_PushFrame(wasm_frame* self, const struct wasm_function_storage* func, size_t* bytes_total)
{
	const size_t offset = WasmFrame_SizeOf(self);
	wasm_frame_base base = {
		.ret = self,
		.program_counter = FunctionStorage_Code(func),
		.locals_count = FunctionStorage_LocalsCount(func),
	};
	size_t bytes_used = WasmFrame_SizeOf(base);
	if((*bytes_total) < bytes_used)
		return NULL;
	
	void* next_frame = (char*)self + offset;
	memcpy(next_frame, &base, sizeof(base)); 
	*bytes_total -= bytes_used;
	return next_frame;
};

wasm_frame* WasmFrame_PopFrame(wasm_frame* self, size_t* bytes_total)
{
	wasm_frame* prev = WasmFrame_ReturnAddress(self);
	*bytes_total -= WasmFrame_SizeOf(self);
	return prev;
}

int WasmFrame_IsBaseFrame(wasm_frame* self)
{
	return !WasmFrame_ReturnAddress(self);
}


typedef struct wasm_call_stack_base_ {
	const size_t allocated;
	size_t remaining;
	wasm_frame* current_frame;
} wasm_call_stack_base;

typedef struct wasm_call_stack_ {
	wasm_call_stack_base base;
	wasm_frame_base[] frames;
} wasm_call_stack;



static size_t* WasmCallStackBase_RemainingPointer(wasm_call_stack_base* self)
{
	return &(self->remaining);
}

static size_t WasmCallStackBase_Remaining(const wasm_call_stack_base* self)
{
	return self->remaining;
}

static size_t WasmCallStackBase_Allocated(const wasm_call_stack_base* self)
{
	return self->allocated;
}

static const wasm_frame* WasmCallStackBase_ConstCurrentFrame(const wasm_call_stack_base* self)
{
	return self->current_frame;
}

static wasm_frame* WasmCallStackBase_CurrentFrame(wasm_call_stack_base* self)
{
	return self->current_frame;
}

static wasm_frame* WasmCallStackBase_SetCurrentFrame(wasm_call_stack_base* self, wasm_frame* new_frame)
{
	wasm_frame* old_frame = WasmCallStackBase_CurrentFrame(self);
	self->current_frame = new_frame;
	return old_frame;
}

}


static wasm_call_stack_base* WasmCallStack_Base(wasm_call_stack* self)
{
	return &(self->base);
}

static const wasm_call_stack_base* WasmCallStack_ConstBase(const wasm_call_stack* self)
{
	return &(self->base);
}

static size_t* WasmCallStack_RemainingPointer(wasm_call_stack_base* self)
{
	return WasmCallStackBase_RemainingPointer(WasmCallStack_Base(self));
}

static size_t WasmCallStack_Remaining(const wasm_call_stack_base* self)
{
	return WasmCallStackBase_Remaining(WasmCallStack_ConstBase(self));
}

static size_t WasmCallStack_Allocated(const wasm_call_stack_base* self)
{
	return WasmCallStackBase_Allocated(WasmCallStack_ConstBase(self));
}

static const wasm_frame* WasmCallStack_ConstCurrentFrame(const wasm_call_stack_base* self)
{
	return WasmCallStackBase_ConstCurrentFrame(WasmCallStack_ConstBase(self));
}

static wasm_frame* WasmCallStack_CurrentFrame(wasm_call_stack_base* self)
{
	return WasmCallStackBase_CurrentFrame(WasmCallStack_Base(self));
}

static wasm_frame* WasmCallStack_SetCurrentFrame(wasm_call_stack_base* self, wasm_frame* new_frame)
{
	return WasmCallStackBase_SetCurrentFrame(WasmCallStack_Base(self), new_frame);
}

static wasm_frame* WasmCallStack_BaseFrame(wasm_call_stack* self)
{
	return self->frames;
}

static const wasm_frame* WasmCallStack_ConstBaseFrame(const wasm_call_stack* self)
{
	return self->frames;
}


wasm_call_stack* WasmCallStack_New(size_t size)
{
	if(size < sizeof(wasm_frame_base))
		return NULL;
	void* stack_mem = malloc(sizeof(wasm_call_stack) + size);
	if(!stack_mem)
		return NULL;
	char* stack_bytes = (char*)stack_mem;

	{
		wasm_call_stack_base base = {
			.allocated = size,
			.remaining = size,
			.current_frame = NULL
		};
		memcpy(stack_mem, &base, sizeof(base));
	}
	
	wasm_call_stack* self = stack_mem;
	WasmStackFrame_InitFrameBase(WasmCallStack_BaseFrame(self), WasmCallStack_RemainingPointer(self));
	WasmCallStack_SetCurrentFrame(self, WasmCallStack_BaseFrame(self));
	return self;
}

wasm_frame* WasmCallStack_Delete(wasm_call_stack* self)
{
	free(self);
}



int WasmCallStack_PushFrame(wasm_call_stack* self, const struct wasm_function_storage* func)
{
	wasm_frame* current_frame = WasmCallStack_CurrentFrame(self);
	size_t* rem = WasmCallStack_RemainingPointer(self);
	wasm_frame* new_frame = WasmFrame_PushFrame(current_frame, func, rem);
	if(!new_frame)
		return -1;
	WasmCallStack_SetCurrentFrame(self, new_frame);
	return 0;
}


void WasmCallStack_FastPopFrame(wasm_call_stack* self)
{
	wasm_frame* current_frame = WasmCallStack_CurrentFrame(self);
	size_t* rem = WasmCallStack_RemainingPointer(self);
	wasm_frame* prev_frame = WasmFrame_PopFrame(current_frame, rem);
	WasmCallStack_SetCurrentFrame(self, prev_frame);
}

int WasmCallStack_PopFrame(wasm_call_stack* self)
{
	if(WasmFrame_IsBaseFrame(WasmCallStack_CurrentFrame(self)))
		return -1;
	WasmCallStack_FastPopFrame(self);
	return 0;
}

const opcode_t* WasmCallStack_Code(const wasm_call_stack* self)
{
	return WasmFrame_Code(WasmCallStack_ConstCurrentFrame(self));
};

const opcode_t* WasmCallStack_CodeNext(wasm_frame* self)
{
	return WasmFrame_CodeNext(WasmCallStack_CurrentFrame(self));
};

const opcode_t* WasmCallStack_CodeAdvance(wasm_frame* self, size_t count)
{
	return WasmFrame_CodeAdvance(WasmCallStack_CurrentFrame(self), count);
};

