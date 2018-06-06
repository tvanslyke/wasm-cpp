#ifndef MODULE_MODULE_H
#define MODULE_MODULE_H

#include <string>
#include <vector>
#include "utilities/SimpleVector.h"
#include "vm/CallStack.h"
#include "module/WasmGlobal.h"
#include <boost/container_hash/hash.hpp>

namespace wasm {

struct LinkError:
	public std::logic_error
{
	LinkError(const std::string& str, const WasmModule& imp, const WasmModule& exp):
		std::logic_error(std::move(str)), importer(imp), exporter(exp)
	{
		
	}
		
	virtual ~LinkError() = default;

	const WasmModule& importer;
	const WasmModule& exporter;
};

struct BadImportError:
	public LinkError
{

	BadImportError(const std::string& msg, const WasmModule& imp, const WasmModule& exp, std::string&& f):
		LinkError(
			msg, imp, exp 
			
		),
		exporter(exp), importer(imp), field_name(std::move(f))
	{
		
	}

	virtual ~BadImportError() = default;

	const std::string field_name;
};

struct NullImportError:
{
	NullImportError(const WasmModule& imp, const WasmModule& exp, std::string&& f):
		BadImportError(
			"Attempt to import value before it was defined in the exporting module.",
			imp, exp, std::move(f)
		)
	{
		
	}

	virtual ~NullImportError() = default;
};

struct FieldNotFoundError:
	public BadImportError
{
	FieldNotFoundError(const WasmModule& imp, const WasmModule& exp, std::string&& f):
		BadImportError(
			"Attempt to import field from module that exports no such field.",
			imp, exp, std::move(f)
		)
	{
		
	}
	
	virtual ~FieldNotFoundError() = default;
};

struct ImportKindMismatchError:
	public BadImportError
{
	ImportKindMismatchError(const WasmModule& imp, const WasmModule& exp, std::string&& f):
		BadImportError(
			"Expected kind of module import does not match the actual kind.",
			imp, exp, std::move(f)
		)
	{
		
	}
	
	virtual ~ImportKindMismatch() = default;
};

struct ImportTypeMismatchError:
	public BadImportError
{
	ImportTypeMismatchError(const WasmModule& imp, const WasmModule& exp, std::string&& f):
		BadImportError(
			"Expected type of module import does not match the actual type.",
			imp, exp, std::move(f)
		)
	{
		
	}

	virtual ~ImportTypeMismatch() = default;
};

struct DoubleImportError:
	public BadImportError
{
	ImportTypeMismatchError(const WasmModule& imp, const WasmModule& exp, std::string&& f):
		BadImportError("Attempt to import field twice.", imp, exp, std::move(f))
	{
		
	}

	virtual ~ImportTypeMismatch() = default;
};

struct UnresolvedImportError:
	public std::logic_error
{
	using std::logic_error::logic_error;
};

struct WasmModule {

	WasmModule(std::sting&& name, std::string&& path, ModuleDef&& module_def):	
		WasmModule(
			std::move(name),
			std::move(path),
			std::move(module_def),
			std::array<std::size_t, 4u>{0u, 0u, 0u, 0u}
		)
	{
		
	}


	using signature_vector_type = SimpleVector<WasmFunctionSignature>;
	using export_type = std::variant<
		WasmFunction**, WasmTable**, WasmMemory**, WasmGlobal**
	>;
	using export_map_type = std::unordered_map<std::string, export_type>;

private:
	using import_type = std::variant<
		WasmFunctionSignature, parse::Table, parse::Memory, parse::GlobalType
	>;
	using import_map_type = std::unordered_map<
		std::string,
		std::unordered_map<
			std::string,
			std::pair<
				std::size_t,
				import_type
			>
		>
	>;

	template <class T>
	const std::string& find_export_name(const T** exported_field) const
	{
		for(const auto& [name, exp]: exports_)
		{
			if(std::holds_alternative<T**>(exp) and (std::get<T**>(exp) == exported_field))
				return name;
		}
		assert(false and "Internal error: exported field is not an export.");
	}

	template <ExternalKind Kind>
	std::pair<const std::string&, const std::string&> find_import_name(wasm_uint32_t index) const
	{
		using pair_type = std::pair<const std::string&, const std::string&>;
		for(const auto& [module_name, import_map]: imports_)
		{
			for(const auto& [field_name, import_def]: import_map)
			{
				const auto& [idx, tp] = import_def;
				if(idx != index)
					continue;
				switch(Kind)
				{
				case ExternalKind::Function:
					if(std::holds_alternative<WasmFunctionSignature>(tp))
						return pair_type(module_name, field_name);
					break;
				case ExternalKind::Table:
					if(std::holds_alternative<parse::Table>(tp))
						return pair_type(module_name, field_name);
					break;
				case ExternalKind::Memory:
					if(std::holds_alternative<parse::Memory>(tp))
						return pair_type(module_name, field_name);
					break;
				case ExternalKind::Memory:
					if(std::holds_alternative<parse::Memory>(tp))
						return pair_type(module_name, field_name);
					break;
				default:
					assert(false);
				}
			}
		}
		assert(false and "Internal error: imported field is not an import.");
	}

	template <class T>
	const auto& safely_access_index_space(const ConstSimpleVector<T*>& space, wasm_uint32_t index) const
	{
		static_assert(
			std::disjunction<
				std::is_same<T, WasmFunction>,
				std::is_same<T, WasmTable>,
				std::is_same<T, WasmMemory>,
				std::is_same<T, WasmGlobal>
			>
		);
		static const std::string import_type_name
			= std::is_same_v<T, WasmFunction> ? "function" 
			: std::is_same_v<T, WasmTable> ? "table" 
			: std::is_same_v<T, WasmMemory> ? "memory" 
			: "global";
		constexpr ExternalKind kind
			= std::is_same_v<T, WasmFunction> ? ExternalKind::Function
			: std::is_same_v<T, WasmTable> ? ExternalKind::Table
			: std::is_same_v<T, WasmMemory> ? ExternalKind::Memory
			: ExternalKind::Global;
		const auto*& item = space.at(index);
		if(not f)
		{
			const auto& [module_name, field_name] = find_import_name(&f);
			throw UnresolvedExportError(
				"Attempt to access imported "
				+ import_type_name + " '"	
				+ module_name + "." + field_name
				+ "' from module '" + name()
				+ "' before import is resolved."
			);
		}
		return *item;
	}

public:
	const WasmFunction& function_at(wasm_uint32_t index) const
	{ return safely_access_index_space(functions_, index); }

	WasmFunction& function_at(wasm_uint32_t index)
	{ return const_cast<WasmFunction&>(as_const(*this).function_at(index)); }

	const WasmTable& table_at(wasm_uint32_t index) const
	{ return safely_access_index_space(tables_, index); }

	WasmTable& table_at(wasm_uint32_t index)
	{ return const_cast<WasmTable&>(as_const(*this).table_at(index)); }

	const WasmMemory& memory_at(wasm_uint32_t index) const
	{ return safely_access_index_space(memories_, index); }

	WasmMemory& memory_at(wasm_uint32_t index)
	{ return const_cast<WasmMemory&>(as_const(*this).memory_at(index)); }

	const WasmGlobal& memory_at(wasm_uint32_t index) const
	{ return safely_access_index_space(globals_, index); }

	WasmGlobal& memory_at(wasm_uint32_t index)
	{ return const_cast<WasmGlobal&>(as_const(*this).global_at(index)); }

	const std::string& name()
	{ return name_; }

	const std::string& path()
	{ return path_; }

	std::optional<const WasmFunction*> start(const WasmModule& self)
	{ return start_ ? *start_ : std::nullopt; }

	gsl::span<const WasmFunctionSignature> types(const WasmModule& self)
	{
		return gsl::span<const WasmFunctionSignature>(
			self.signatures_.data(), self.signatures_.size()
		);
	}

	friend void link(WasmModule& left, WasmModule& right)
	{ left.import_fields_from(right); }

	bool is_fully_linked() const
	{
		if(imports.empty())
		{
			assert(elem_segs_.empty());
			assert(data_segs_.empty());
			return true;
		}
		else
		{
			return false;
		}
	}

private:

	static SimpleVector<WasmFunctionSignature> read_type_section(ModuleDef&& module_def)
	{
		auto make_sig = [](parse::FunctionSignature&& sig_def) {
			return WasmFunctionSignature(std::move(sig_def));
		};
		return SimpleVector<WasmFunctionSignature>(
			make_transform_iterator(std::move(def).type_section.begin(), make_sig),
			make_transform_iterator(std::move(def).type_section.end(), make_sig)
		);
	}

	template <class T>
	ConstSimpleVector<T*> make_index_space(std::size_t import_count, ConstSimpleVector<T>& defs)
	{
		ConstSimpleVector<T*> s(import_count + defs.size());
		auto pos = s.begin();
		auto pointer_to = [](auto& v){ return std::addressof(v); };
		pos = std::fill_n(pos, import_count, nullptr);
		pos = std::transform(defs.begin(), defs.end(), pos, pointer_to);
		return s;
	}

	WasmModule(std::sting&& name, std::string&& path, ModuleDef&& module_def, std::array<std::size_t, 4u>& spaces):
		name_(std::move(name)),
		path_(std::move(path)),
		types_(read_type_section(std::move(module_def))),
		imports_(read_import_section(std::move(module_def), spaces)),
		defs_(std::move(module_def)),
		functions_(make_index_space(spaces[std::size_t(ExternalKind::Function)], defs_.functions_)),
		tables_(make_index_space(spaces[std::size_t(ExternalKind::Table)], defs_.tables_)),
		memories_(make_index_space(spaces[std::size_t(ExternalKind::Memory)], defs_.memories_)),
		globals_(make_index_space(spaces[std::size_t(ExternalKind::Global)], defs_.globals_)),
		start_(
			module_def.start_section ?
			const_cast<const WasmFunction* const* const>(
				std::addressof(functions_[*module_def.start_section])
			) : nullptr
		),
		exports_(read_export_section(std::move(module_def)))
	{
		for(auto& glbl: globals_)
		{
			if(not glbl)
				continue;
			if(glbl->has_dependency())
			{
				auto [idx , tp] = glbl.get_dependency();
				assert(idx < globals_.size());
				auto& dep = globals_[idx];
				global_deps_[const_cast<const WasmGlobal* const*>(std::addressof(dep))].push_back(glbl);
			}
		}
		for(const auto& [dep, _] : global_deps_)
		{
			assert(dep);
			if(*dep)
				notify_dependents(dep);
		}
		if(module_def.element_section)
		{
			for(auto& seg: module_def.element_section)
				if(not initialize_elem_segment(seg))
					elem_segs_[get_segment_dependency(seg)].push_back(std::move(seg));
		}
		if(module_def.data_section)
		{
			for(auto& seg: module_def.data_section)
				if(not initialize_data_segment(seg))
					data_segs_[get_segment_dependency(seg)].push_back(std::move(seg));
		}
	}

	void import_field(
		const ModuleDef& other,
		const std::string& field_name,
		const import_type& import_def,
		const export_type& export_def
	)
	{
		assert(not import_def.valueless_by_exception());
		assert(not export_def.valueless_by_exception());
		if(import_def.index() != export_def.index())
			throw ImportKindMismatchError(*this, other, field_name);
		const auto& [import_index, import_tp] = import_def;
		auto visit_import_export = [&](const auto& imported, auto** exported) {
			using import_external_kind_type = std::decay_t<decltype(imported)>;
			using exported_type = std::decay_t<decltype(**exported)>;
			using export_external_kind_type = typename exported_type::wasm_external_kind_type;
			assert(exported);
			auto do_import = [&](auto& index_space) {
				auto*& pos = index_space.at(import_index);
				if(not pos)
			};
			constexpr bool external_kinds_match = std::is_same_v<
				import_external_kind_type,
				export_external_kind_type
			>;
			if constexpr(external_kinds_match)
			{
				if(not *exported)
					throw NullImportError(*this, other, field_name);
				if(not matches(**exported, imported))
					throw ImportTypeMismatchError(*this, other, field_name);
				using index_space_type = ConstSimpleVector<exported_type>;
				auto& index_space = std::get<index_space_type&>(
					std::tie(functions_, tables_, memories_, globals_)
				);
				auto*& elem = index_space.at(import_index);
				assert((not elem)); // shouldn't be possible... uh oh
				elem = *exported;
				if constexpr(std::is_same_v<exported_type, WasmGlobal>)
				{
					using global_key_type = const parse::WasmGlobal* const*;
					_notify_dependents(const_cast<global_key_type>(&elem));
				}
			}
			else
			{
				throw ImportKindMismatchError(*this, other, field_name);
			}
		};
		std::visit(visit_import_export, import_tp, export_def);
	}

	std::pair<std::size_t, std::size_t> import_fields_from(const ModuleDef& other)
	{
		auto module_pos = imports_.find(name(other));
		if(module_pos == imports_.end())
			return std::make_pair(0, 0);
		auto& imps = *module_pos;
		std::size_t total = imps.size();
		for(auto pos = imps.begin(); pos != imps.end(); pos = imps.erase(pos))
		{
			const auto& [field_name, import_def] = *pos;
			if(auto pos = other.exports_.find(field_name); pos != other.exports_.end())
			{
				const export_type& export_def = pos->second;
				import_field(other, field_name, import_def, export_def);
			}
		}
		if(imps.empty())
		{
			imports_.erase(module_pos);
			return std::make_pair(total, total);
		}
		else
		{
			return std::make_pair(total - imps.size(), total);
		}
	}

	template <class Segment>
	const WasmGlobal* const* get_segment_dependency(const Segment& seg)
	{
		assert(std::holds_alternative<wasm_uint32_t>(seg.offset));
		return const_cast<const WasmGlobal* const*>(
			std::addressof(globals_.at(seg.offset))
		);
	}

	std::optional<wasm_sint32_t> try_get_offset(const std::variant<wasm_sint32_t, wasm_uint32_t>& ofs)
	{
		assert(not ofs.valueless_by_exception());
		if(std::holds_alternative<wasm_sint32_t>(ofs))
			return std::get<wasm_sint32_t>(ofs_var);
		auto globals_offset = std::get<wasm_uint32_t>(ofs_var);
		if(auto g = globals_.at(globals_offset); g and not g->has_dependency())
			return read<wasm_sint32_t>(*g);
		else
			return std::nullopt;
	}

	template <class Segment, class IndexSpace>
	auto get_segment(Segment& seg, IndexSpace& ind_sp)
	{
		auto& [index, ofs_var, data] = seg;
		assert(ind_sp.size() > index);
		auto maybe_offset = try_get_offset(ofs_var);
		if(not maybe_offset)
			return false;
		assert(*maybe_offset >= 0);
		std::size_t offset = *maybe_offset;
		auto& item = ind_sp.at(offset);
		return item.get_segment(offset, data.size());
	}

	bool initialize_elem_segment(parse::ElemSegment&& seg)
	{
		auto slice = get_segment(seg, tables_);
		auto& inds = seg.indices;
		for(std::size_t i = 0; i < slice.size(); ++i)
		{
			const WasmFunction* const& fn = functions_.at(inds[i]);
			slice[i].emplace_wasm_function(std::addressof(fn));
		}
		return true;
	}

	bool initialize_data_segment(parse::DataSegment&& seg)
	{
		auto slice = table.get_segment(seg, memories_);
		std::copy(seg.data.begin(), seg.data.end(), slice.begin());
		return true;
	}

	void initialize_global_dep(parse::WasmGlobal* const* dest, const parse::WasmGlobal& src)
	{
		(*dest)->init_dep(src);
		// add const qualifier at bottom level
		nodify_dependents(const_cast<const parse::WasmGlobal* const*>(dest));
	}

	template <class T>
	void initialize_global_const(parse::WasmGlobal* const* dest, const T& value)
	{
		(*dest)->init_const(value);
		// add const qualifier at bottom level
		nodify_dependents(const_cast<const parse::WasmGlobal* const*>(dest));
	}

	template <class T>
	void initialize_global_mut(parse::WasmGlobal* const* dest, const T& value)
	{
		(*dest)->init_mut(value);
		// add const qualifier at bottom level
		_notify_dependents(const_cast<const parse::WasmGlobal* const*>(dest));
	}

	void _notify_dependents(const parse::WasmGlobal* const* dep)
	{
		wasm_uint32_t offset = static_cast<wasm_uint32_t>(dep - globals_.data());
		assert(static_cast<std::ptrdiff_t>(offset) == dep - globals_.data());
		// initialize table element segments
		if(auto pos = elem_segs_.find(dep); pos != elem_segs_.end())
		{
			for(auto& seg: *pos)
			{
				initialize_elem_segment(std::move(seg));
			}
			elem_segs_.erase(pos);
		}
		if(auto pos = data_segs_.find(dep); pos != data_segs_.end())
		{
			for(auto& seg: *pos)
				initialize_data_segment(std::move(seg));
			data_segs.erase(pos);
		}
		if(auto pos = global_deps_.find(dep); pos != global_deps_.end())
		{
			for(WasmGlobal* const* glbl: *pos)
				// recursively initialize globals.
				initialize_global_dep(glbl, **dep);
			global_deps_.erase(pos);
		}
	}


	export_map_type read_export_section(ModuleDef&& module_def)
	{
		export_map_type exports;
		for(parse::ExportEntry& entry: module_def.export_section)
		{
			auto add_export = [&](auto& index_space) {
				auto [_, success] = exports.try_emplace(std::move(entry.name), &(index_space[entry.index]));
				assert(success);
			};
			switch(entry.kind)
			{
			case ExternalKind::Function: 
				add_export(functions_);
				break;
			case ExternalKind::Table:
				add_export(tables_);
				break;
			case ExternalKind::Memory:
				add_export(memories_);
				break;
			case ExternalKind::Global:
				add_export(globals_);
				break;
			default:
				assert(false);
			}
		}
		return exports;
	}

	import_map_type read_import_section(ModuleDef&& module_def, std::array<std::size_t, 4u>& spaces)
	{
		import_map_type imports;
		if(module_def.import_section)
		{
			for(auto&& entry: *module_def.import_section)
			{
				auto& module_map = imports[std::move(entry.module_name)];
				auto& field_name = entry.field_name;
				
				auto emplace_import = [&](auto& space, auto&& ... args) {
					auto [pos, success] = module_map.try_emplace(
						std::move(field_name), space, std::forward<decltype(args)>(args)...
					);
					assert(success);
					space += 1u;
				};
				
				auto visit_import = [&](auto&& entry_type) {
					using type = std::decay_t<decltype(entry_type)>;
					if constexpr(std::is_same_v<type, std::uint32_t>)
						emplace_import(spaces[std::size_t(ExternalKind::Function)], signatures[entry_type]);
					else if constexpr(std::is_same_v<type, parse::Table>)
						emplace_import(spaces[std::size_t(ExternalKind::Table)], std::move(entry_type));
					else if constexpr(std::is_same_v<type, parse::Memory>)
						emplace_import(spaces[std::size_t(ExternalKind::Memory)], std::move(entry_type));
					else if constexpr(std::is_same_v<type, parse::GlobalType>)
						emplace_import(spaces[std::size_t(ExternalKind::Global)], std::move(entry_type));
					else
						assert(false && "Unreachable.");
				};
				std::visit(visit_import, std::move(entry));
			}
		}
		return imports;
	}

	struct ModuleDefinitions {
		
		ConstSimpleVector<WasmFunction> read_functions(ModuleDef&& module_def, const signature_vector_type& sigs)
		{
			if(module_def.function_section)
			{
				assert(module_def.type_section);
				assert(module_def.code_section);
				std::size_t count = module_def.function_section.size();
				assert(count == module_def.code_section.size());
				auto& func_sec = *module_def.function_section;
				auto& code_sec = *module_def.code_section;
				auto code_pos = code_sec.begin();
				auto& tform = [&](std::uint32_t ofs) {
					return WasmFunction(signatures.at(ofs), std::move(*code_pos++));
				};
				return ConstSimpleVector<WasmFunction>(
					make_transform_iterator(func_sec.begin(), tform),
					make_transform_iterator(func_sec.end(), tform)
				);
			}
			return ConstSimpleVector<WasmFunction>();
		}

		ConstSimpleVector<WasmGlobal> read_globals(ModuleDef&& module_def)
		{
			return ConstSimpleVector<WasmGlobal>(
				module_def.global_section.begin(),
				module_def.global_section.end()
			);
		}

		ConstSimpleVector<WasmLinearMemory> read_memories(ModuleDef&& module_def)
		{
			return ConstSimpleVector<WasmLinearMemory>(
				module_def.memory_section.begin(),
				module_def.memory_section.end()
			);
		}


		ModuleDefinitions(ModuleDef&& module_def):
			functions_(read_functions(std::move(module_def))),
			memories_(read_memories(std::move(module_def))),
			globals_(read_globals(std::move(module_def))),
			tables_(read_tables(std::move(module_def)))
		{
			
		}

		ConstSimpleVector<WasmFunction>     funtions_;
		ConstSimpleVector<WasmLinearMemory> memories_;
		ConstSimpleVector<WasmGlobal>       globals_;
		ConstSimpleVector<WasmTable>        tables_;
	};

	/// This module's 'name'.  Used in import/export resolution and debugging.
	const std::string name_;
	/// The filesystem path to this module.
	const std::string path_;
	/// The function signatures used in this module.
	const SimpleVector<WasmFunctionSignature> types_;
	/// Contains the functions, tables, memories, and globals that are explicitly defined in this modules (not imported).
	ModuleDefinitions defs_;
	/// Maps module + field names to not-yet-resolved imports for this module.
	import_map_type imports_;
	/// Function index space. Has pointers to all functions visible to this module.
	ConstSimpleVector<WasmFunction*> functions_;
	/// Memory index space. Has pointers to all memories visible to this module.
	ConstSimpleVector<WasmLinearMemory*> memories_;
	/// Table index space. Has pointers to all tables visible to this module.
	ConstSimpleVector<WasmTable*> tables_;
	/// Global index space. Has pointers to all globals visible to this module.
	ConstSimpleVector<WasmGlobal*> globals_;
	/// Pointer-to-pointer to the start function for this module.  Possibly null.  Points into 'functions_'.
	const WasmFunction* const* const start_;
	/// Maps field names to exports for this module.
	export_map_type exports_;
	/// Uninitialized element segments.
	std::unordered_map<const WasmGlobal* const*, std::vector<parse::ElemSegment>> elem_segs_;
	/// Uninitializes data segments.
	std::unordered_map<const WasmGlobal* const*, std::vector<parse::DataSegment>> data_segs_;
	/// Uninitialized globals with dependencies.
	std::unordered_map<const WasmGlobal* const*, std::vector<WasmGlobal* const*>> global_deps_;
};

} /* namespace wasm */

#endif /* MODULE_MODULE_H */
