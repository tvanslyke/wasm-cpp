#ifndef MODULE_MODULE_H
#define MODULE_MODULE_H

#include <string>
#include <vector>
#include "utilities/SimpleVector.h"
#include "vm/CallStack.h"
#include "module/WasmGlobal.h"
#include <boost/container_hash/hash.hpp>

namespace wasm {

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
	WasmModule(std::sting&& name, std::string&& path, ModuleDef&& module_def, & spaces):

private:

	using signature_vector_type = SimpleVector<WasmFunctionSignature>;
	using export_map_type = std::unordered_map<
		std::string,
		std::variant<
			WasmFunction**,
			WasmTable**,
			WasmMemory**,
			WasmGlobal**
		>
	>;
	using import_map_type = std::unordered_map<
		std::string,
		std::unordered_map<
			std::string,
			std::pair<
				std::size_t, 
				std::variant<
					WasmFunctionSignature,
					parse::LinearMemory,
					parse::Memory,
					parse::GlobalType
				>
			>
		>
	>;

	template <class T>
	struct RAII_SegmentInitializer
	{
		RAII_SegmentInitializer(const WasmGlobal* const& ofs, T* base, std::size_t len, T&& value):
			offset_(ofs), base_(base), length_(len), value_(std::move(value))
		{
			
		}

		~RAII_SegmentInitializer()
		{
			auto ofs = read_if<wasm_sint32_t>(offset_);
			assert(ofs);
			std::fill_n(base_ + *ofs, length_, value_);
		}

	private:
		// value of the offset
		const WasmGlobal* const& offset_;
		// base pointer of the segment to initialize
		T* const base_;
		// length of the segment
		const std::size_t length_;
		// value to initialize with.
		const T value_;
	}

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
		start_(module_def.start_section ? &functions_[*module_def.start_section] : nullptr),
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
				global_deps_[std::addressof(dep)].push_back(glbl);
			}
		}
		for(const auto& [dep, _] : global_deps_)
		{
			if(dep)
				
		}
	}

	void notify_dependents(parse::WasmGlobal** dep)
	{
		// initialize table element segments
		if(auto pos = elem_segs_.find(dep); if pos != elem_segs_.end())
		{
			
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
	const WasmFunction** start_;
	/// Maps field names to exports for this module.
	export_map_type exports_;
	/// Uninitialized element segments.
	std::unordered_map<WasmGlobal**, std::vector<parse::DataSegment*>> elem_segs_;
	/// Uninitializes data segments.
	std::unordered_map<WasmGlobal**, std::vector<parse::DataSegment*>> data_segs_;
	/// Uninitialized globals with dependencies.
	std::unordered_map<WasmGlobal**, std::vector<WasmGlobal*>> global_deps_;
};

} /* namespace wasm */

#endif /* MODULE_MODULE_H */
