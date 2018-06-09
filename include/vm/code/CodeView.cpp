#include "vm/code/CodeView.h"

namespace wasm::opc {

struct CodeView
{
	struct Iterator;
	using value_type = WasmInstruction;
	using pointer = WasmInstruction*;
	using const_pointer = const WasmInstruction*;
	using reference = WasmInstruction&;
	using const_reference = WasmInstruction&;
	using size_type = std::string_view::size_type;
	using difference_type = std::string_view::difference_type;
	using iterator = Iterator;
	using const_iterator = iterator;

private:

	struct InstructionVisitor {

		template <class ... T>
		WasmInstruction operator()(
			const char* first,
			const char* last,
			const char* pos,
			OpCode op,
			T&& ... args
		)
		{
			assert(first < pos);
			assert(pos <= last);
			return make_instr(
				std::string_view(first, pos - first),
				op,
				last,
				std::forward<T>(args)...
			);
		}

	private:
		// Overload for instructions with immediate operands
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last)
		{ return WasmInstruction(view, op, last, std::monostate()); }

		template <
			class T,
			/* enable overloads for the simple alternatives. */
			class = std::enable_if_t<
				std::disjunction_v<
					std::is_same_v<std::decay_t<T>, wasm_uint32_t>,
					std::is_same_v<std::decay_t<T>, wasm_sint32_t>,
					std::is_same_v<std::decay_t<T>, wasm_sint64_t>,
					std::is_same_v<std::decay_t<T>, wasm_float32_t>,
					std::is_same_v<std::decay_t<T>, wasm_float64_t>
				>
			>
		>
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, T&& arg)
		{ return WasmInstruction(view, op, last, std::forward<T>(arg)); }

		/// Loop overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, LanguageType tp)
		{ return WasmInstruction(view, op, last, tp); }

		/// Branch table overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, const char* base, wasm_uint32_t len)
		{
			assert(last > base);
			std::size_t byte_count = base - last;
			std::size_t bytes_needed = sizeof(wasm_uint32_t) * (len + 1u);
			assert(byte_count >= bytes_needed);
			// the table and 'view' should end at the same byte address
			assert(view.data() + view.size() == (base + (len + 1u) * sizeof(wasm_uint32_t)));
			using buffer_type = const char[sizeof(wasm_uint32_t)];
			auto table = gsl::span<buffer_type>(reinterpret_cast<buffer_type*>(base), len + 1u);
			return WasmInstruction(view, op, last, table);
		}

		/// Block overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, LanguageType tp, wasm_uint32_t label)
		{ return WasmInstruction(view, op, last, BlockImmediate(tp, label)); }

		/// Memory overload
		WasmInstruction make_instr(std::string_view view, OpCode op, const char* last, wasm_uint32_t flags, wasm_uint32_t offset)
		{ return WasmInstruction(view, op, last, MemoryImmediate(flags, offset)); }

		/// Invalid opcode overload
		[[noreturn]]
		WasmInstruction make_instr(std::string_view, OpCode, const char*, const BadOpCodeError& err)
		{ throw err; }

	};

public:
	
	struct Iterator {
		using value_type = CodeView::value_type;
		using difference_type = CodeView::difference_type;
		using pointer = CodeView::pointer;
		using reference = CodeView::value_type;
		using iterator_category = std::input_iterator_tag;
		
		friend bool operator==(const Iterator& left, const Iterator& right)
		{ return left.code_ == right.code_; }
		
		friend bool operator!=(const Iterator& left, const Iterator& right)
		{ return not (left == right); }
	private:
		gsl::not_null<CodeView*> code_;
	};

	CodeView(const WasmFunction& func):
		function_(&func),
		code_(code(func))
	{
		
	}

	const WasmFunction* function() const
	{ return function_; }

	OpCode current_op() const
	{
		assert(ready());
		return static_cast<WasmInstruction>(code_.front());
	}

	bool done() const
	{ return not code_.empty(); }

	std::optional<WasmInstruction> next_instruction() const
	{
		assert(ready());
		if(code_.size() == 0)
			return std::nullopt;
		return visit_opcode(InstructionVisitor{}, code_.data(), code_.data() + code_.size());
	}

	const char* pos() const
	{ return code_.data(); }

	void advance(const CodeView& other)
	{
		assert(function() == other.function());
		assert(ready());
		assert(code_.data() + code_.size() == other.code_.data() + other.code_.size());
		assert(code_.data() < other.code_.data());
		code_ = other.code_;
	}

private:

	bool ready() const
	{
		if(done())
			return false;
		assert(opcode_exists(code_.front()));
		return true;
	}
	const WasmFunction* function_;
	std::string_view code_;
};


} /* namespace wasm::opc */
