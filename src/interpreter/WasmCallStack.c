#include "interpreter/WasmCallStack.h"
#include "function/wasm_function_storage.h"
#include <stdalign.h>
#include <string.h>
#include <stdlib.h>

typedef struct WasmFrame_ WasmFrame;

typedef struct WasmFrameBase_ {
	const uint_least32_t function_index;
	struct WasmFrame_* ret;
	const opcode_t* program_counter;
	const size_t locals_count;
} WasmFrameBase;

struct WasmFrame_
{
	WasmFrameBase base;
	wasm_value_t locals[];
};



static const WasmFrameBase base_frame = {
	.ret = NULL,
	.program_counter = NULL,
	.locals_count = 0
};

static const size_t frame_align = alignof(WasmFrame);

static size_t WasmFrameBase_LocalsCount(const WasmFrameBase* self)
{
	return self->locals_count;
}

static size_t WasmFrameBase_FunctionIndex(const WasmFrameBase* self)
{
	return self->function_index;
}

static const opcode_t* WasmFrameBase_Code(const WasmFrameBase* self)
{
	return self->program_counter;
}

static const opcode_t* WasmFrameBase_CodeNext(WasmFrameBase* self)
{
	return ++(self->program_counter);
}

static const opcode_t* WasmFrameBase_CodeAdvance(WasmFrameBase* self, size_t count)
{
	self->program_counter += count;
	return self->program_counter;
}

static const opcode_t* WasmFrameBase_CodeJump(WasmFrameBase* self, const opcode_t* label)
{
	const opcode_t* old = self->program_counter;
	self->program_counter = label;
	return old;
}

static WasmFrame* WasmFrameBase_ReturnAddress(WasmFrameBase* self)
{
	return self->ret;
}

static size_t WasmFrameBase_SizeOf(const WasmFrameBase* self)
{
	size_t next_frame_offset = sizeof(*self) + WasmFrameBase_LocalsCount(self) * sizeof(wasm_value_t);
	size_t align_err = (next_frame_offset % frame_align);
	if(align_err)
		align_err = frame_align - align_err;
	return next_frame_offset + align_err;
}

static WasmFrameBase* WasmFrame_Base(WasmFrame* self)
{
	return &(self->base);
}

static const WasmFrameBase* WasmFrame_ConstBase(const WasmFrame* self)
{
	return &(self->base);
}

static int WasmFrame_InitFrameBase(void* self, size_t* total_bytes)
{
	const size_t frame_size = WasmFrameBase_SizeOf(&base_frame);
	if(*total_bytes < frame_size)
		return -1;
	memcpy(self, &base_frame, frame_size);
	*total_bytes -= frame_size;
	return 0;
}

static size_t WasmFrame_LocalsCount(const WasmFrame* self)
{
	return WasmFrameBase_LocalsCount(WasmFrame_ConstBase(self));
}

static wasm_value_t* WasmFrame_Locals(WasmFrame* self)
{
	return self->locals;
}

static size_t WasmFrame_FunctionIndex(const WasmFrame* self)
{
	return WasmFrameBase_FunctionIndex(WasmFrame_ConstBase(self));
}

static const opcode_t* WasmFrame_Code(const WasmFrame* self)
{
	return WasmFrameBase_Code(WasmFrame_ConstBase(self));
}

static const opcode_t* WasmFrame_CodeNext(WasmFrame* self)
{
	return WasmFrameBase_CodeNext(WasmFrame_Base(self));
}

static const opcode_t* WasmFrame_CodeAdvance(WasmFrame* self, size_t count)
{
	return WasmFrameBase_CodeAdvance(WasmFrame_Base(self), count);
}

static const opcode_t* WasmFrame_CodeJump(WasmFrame* self, const opcode_t* label)
{
	return WasmFrameBase_CodeJump(WasmFrame_Base(self), label);
}

static WasmFrame* WasmFrame_ReturnAddress(WasmFrame* self)
{
	return WasmFrameBase_ReturnAddress(WasmFrame_Base(self));
}

static size_t WasmFrame_SizeOf(const WasmFrame* self)
{
	size_t next_frame_offset = sizeof(*self) + WasmFrame_LocalsCount(self) * sizeof(wasm_value_t);
	size_t align_err = (next_frame_offset % frame_align);
	if(align_err)
		align_err = frame_align - align_err;
	return next_frame_offset + align_err;
}

static WasmFrame* WasmFrame_PushFrame(WasmFrame* self, const struct wasm_function_storage* func, size_t* bytes_total, size_t index)
{
	// number of bytes to the beginning of the next frame
	const size_t offset = WasmFrame_SizeOf(self);
	WasmFrameBase base = {
		.function_index = index,
		.ret = self,
		.program_counter = FunctionStorage_Code(func),
		.locals_count = FunctionStorage_LocalsCount(func),
	};
	// see how large the next frame will be
	size_t bytes_used = WasmFrameBase_SizeOf(&base);
	// if it's too large, return  NULL to indicate stack overflow
	if((*bytes_total) < bytes_used)
		return NULL;
	// initialize the frame metadata (frame base)
	void* next_frame = (char*)self + offset;
	memcpy(next_frame, &base, sizeof(base)); 
	size_t locals_count = WasmFrame_LocalsCount(next_frame);
	if(locals_count > 0)
	{
		// set locals to zero (required by wasm standard)
		wasm_value_t* locals = WasmFrame_Locals(next_frame);
		memset(locals, 0,  locals_count * sizeof(*locals));
	}
	*bytes_total -= bytes_used;
	return next_frame;
}

static WasmFrame* WasmFrame_PopFrame(WasmFrame* self, size_t* bytes_total)
{
	WasmFrame* prev = WasmFrame_ReturnAddress(self);
	*bytes_total -= WasmFrame_SizeOf(self);
	return prev;
}

static int WasmFrame_IsBaseFrame(WasmFrame* self)
{
	return !WasmFrame_ReturnAddress(self);
}





typedef struct WasmCallStackBase_ {
	const size_t allocated;
	size_t remaining;
	WasmFrame* current_frame;
} WasmCallStackBase;

typedef struct WasmCallStack_ {
	WasmCallStackBase base;
	WasmFrame frames[];
} WasmCallStack;



static size_t* WasmCallStackBase_RemainingPointer(WasmCallStackBase* self)
{
	return &(self->remaining);
}

static size_t WasmCallStackBase_Remaining(const WasmCallStackBase* self)
{
	return self->remaining;
}

static size_t WasmCallStackBase_Allocated(const WasmCallStackBase* self)
{
	return self->allocated;
}

static const WasmFrame* WasmCallStackBase_ConstCurrentFrame(const WasmCallStackBase* self)
{
	return self->current_frame;
}

static WasmFrame* WasmCallStackBase_CurrentFrame(WasmCallStackBase* self)
{
	return self->current_frame;
}

static WasmFrame* WasmCallStackBase_SetCurrentFrame(WasmCallStackBase* self, WasmFrame* new_frame)
{
	WasmFrame* old_frame = WasmCallStackBase_CurrentFrame(self);
	self->current_frame = new_frame;
	return old_frame;
}



static WasmCallStackBase* WasmCallStack_Base(WasmCallStack* self)
{
	return &(self->base);
}

static const WasmCallStackBase* WasmCallStack_ConstBase(const WasmCallStack* self)
{
	return &(self->base);
}

static size_t* WasmCallStack_RemainingPointer(WasmCallStack* self)
{
	return WasmCallStackBase_RemainingPointer(WasmCallStack_Base(self));
}

#ifdef __GNUC__ 
__attribute__((unused))
#endif
static size_t WasmCallStack_Remaining(const WasmCallStack* self)
{
	return WasmCallStackBase_Remaining(WasmCallStack_ConstBase(self));
}

#ifdef __GNUC__ 
__attribute__((unused))
#endif
static size_t WasmCallStack_Allocated(const WasmCallStack* self)
{
	return WasmCallStackBase_Allocated(WasmCallStack_ConstBase(self));
}

static const WasmFrame* WasmCallStack_ConstCurrentFrame(const WasmCallStack* self)
{
	return WasmCallStackBase_ConstCurrentFrame(WasmCallStack_ConstBase(self));
}

static WasmFrame* WasmCallStack_CurrentFrame(WasmCallStack* self)
{
	return WasmCallStackBase_CurrentFrame(WasmCallStack_Base(self));
}

static WasmFrame* WasmCallStack_SetCurrentFrame(WasmCallStack* self, WasmFrame* new_frame)
{
	return WasmCallStackBase_SetCurrentFrame(WasmCallStack_Base(self), new_frame);
}

static WasmFrame* WasmCallStack_BaseFrame(WasmCallStack* self)
{
	return self->frames;
}


#ifdef __GNUC__ 
__attribute__((unused))
#endif
static const WasmFrame* WasmCallStack_ConstBaseFrame(const WasmCallStack* self)
{
	return self->frames;
}




/* header-visible functions */

WasmCallStack* WasmCallStack_New(size_t size)
{
	if(size < sizeof(WasmFrameBase))
		return NULL;
	void* stack_mem = malloc(sizeof(WasmCallStack) + size);
	if(!stack_mem)
		return NULL;

	{
		WasmCallStackBase base = {
			.allocated = size,
			.remaining = size,
			.current_frame = NULL
		};
		memcpy(stack_mem, &base, sizeof(base));
	}
	
	WasmCallStack* self = stack_mem;
	WasmFrame_InitFrameBase(WasmCallStack_BaseFrame(self), WasmCallStack_RemainingPointer(self));
	WasmCallStack_SetCurrentFrame(self, WasmCallStack_BaseFrame(self));
	return self;
}

void WasmCallStack_Delete(void* self)
{
	free(self);
}

int WasmCallStack_PushFrame(WasmCallStack* self, const struct wasm_function_storage* func, size_t index)
{
	WasmFrame* current_frame = WasmCallStack_CurrentFrame(self);
	size_t* rem = WasmCallStack_RemainingPointer(self);
	WasmFrame* new_frame = WasmFrame_PushFrame(current_frame, func, rem, index);
	if(!new_frame)
		return -1;
	WasmCallStack_SetCurrentFrame(self, new_frame);
	return 0;
}

void WasmCallStack_FastPopFrame(WasmCallStack* self)
{
	WasmFrame* current_frame = WasmCallStack_CurrentFrame(self);
	size_t* rem = WasmCallStack_RemainingPointer(self);
	WasmFrame* prev_frame = WasmFrame_PopFrame(current_frame, rem);
	WasmCallStack_SetCurrentFrame(self, prev_frame);
}

int WasmCallStack_PopFrame(WasmCallStack* self)
{
	if(WasmFrame_IsBaseFrame(WasmCallStack_CurrentFrame(self)))
		return -1;
	WasmCallStack_FastPopFrame(self);
	return 0;
}

const opcode_t* WasmCallStack_Code(const WasmCallStack* self)
{
	return WasmFrame_Code(WasmCallStack_ConstCurrentFrame(self));
}

const opcode_t* WasmCallStack_CodeNext(WasmCallStack* self)
{
	return WasmFrame_CodeNext(WasmCallStack_CurrentFrame(self));
}

const opcode_t* WasmCallStack_CodeAdvance(WasmCallStack* self, size_t count)
{
	return WasmFrame_CodeAdvance(WasmCallStack_CurrentFrame(self), count);
}

const opcode_t* WasmCallStack_CodeJump(WasmCallStack* self, const opcode_t* label)
{
	return WasmFrame_CodeJump(WasmCallStack_CurrentFrame(self), label);
}

wasm_value_t* WasmCallStack_Locals(WasmCallStack* self)
{
	return WasmFrame_Locals(WasmCallStack_CurrentFrame(self));
}

size_t WasmCallStack_FunctionIndex(const WasmCallStack* self)
{
	return WasmFrame_FunctionIndex(WasmCallStack_ConstCurrentFrame(self));
}

size_t WasmCallStack_LocalsCount(const WasmCallStack* self)
{
	return WasmFrame_LocalsCount(WasmCallStack_ConstCurrentFrame(self));
}


