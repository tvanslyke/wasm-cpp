#ifndef PARSE_OBJECTS_WASM_PROGRAM_OBJECTS_H
#define PARSE_OBJECTS_WASM_PROGRAM_OBJECTS_H
#include "parse/objects/WasmObject.h"
#include <unordered_map>
#include <string>
#include "parse/wasm_module_def.h"

struct WasmModuleObject
{
	template <class String>
	WasmModuleObject(String&& name_):
		name(name_)
	{
		
	}

	template <class String>
	void add_import(String&& field_name, WasmObject* imp);
	{
		auto [pos, success] = imports.try_emplace(std::forward<String>(field_name), imp);
		if(not success)
			double_import_or_export_error(imp, *pos, "import");
		get_index_space(imp->get_type()).add_import(imp);
	}

	template <class String>
	void add_export(String&& field_name, WasmObject* exp, wasm_uint32_t index);
	{
		auto [pos, success] = exports.try_emplace(std::forward<String>(field_name), imp);
		if(not success)
			double_import_or_export_error(imp, *pos, "export");
		get_index_space(imp->get_type()).add_export(index, imp);
	}

	WasmObject* get_import(const std::string& field_name)
	{ return imports.at(field_name); }

	WasmObject* get_export(const std::string& field_name)
	{ return imports.at(field_name); }

	void reserve_signatures(wasm_uint32_t count)
	{ signatures.reserve(count); }

	void reserve_functions(wasm_uint32_t count)
	{ functions.reserve_definitions(count); }

	void reserve_tables(wasm_uint32_t count)
	{ tables.reserve_definitions(count); } 

	void reserve_globals(wasm_uint32_t count)
	{ globals.reserve_definitions(count); }

	void reserve_memories(wasm_uint32_t count)
	{ memories.reserve_definitions(count); }

	WasmObject* index_at(WasmObjectKind tp, std::size_t idx)
	{
		return get_index_space(tp).at(idx);
	}

	void add_signature(WasmFunctionSignature* sig);
	void add_function(WasmFunctionObject* function);
	void add_table(WasmTableObject* table);
	void add_memory(WasmMemoryObject* memory);
	void add_global(WasmGlobalObject* global);

	const WasmFunctionSignature* get_signature(wasm_uint32_t index);
	const WasmFunctionObject* get_function(wasm_uint32_t index);
private:
	struct ModuleIndexSpace
	{
		ModuleIndexSpace(WasmObjectKind tp):
			type(tp)
		{
			
		}
		void add_import(WasmObject* object)
		{
			assert(object->get_type() == type);
			imports.push_back(object);
		}
		
		void add_export(wasm_uint32_t index, WasmObject* object)
		{
			assert(object->get_type() == type);
			if(index > imports.size())
			{
				
			}
			else 
			{
				
			}
			reserve(index + 1);
			assert(at(index) == nullptr);
			items[index] = object;
		}
		
		WasmObject*& operator[](std::size_t idx)
		{
			if(idx < imports.size())
				return imports[idx];
			else
				return items[idx - imports.size()]; 
		}
		
		const WasmObject*& operator[](std::size_t idx) const
		{
			if(idx < imports.size())
				return imports[idx];
			else
				return items[idx - imports.size()]; 
		}

		WasmObject*& at(std::size_t idx)
		{
			index_check(idx);
			return (*this)[idx]; 
		}
		
		const WasmObject*& at(std::size_t idx) const
		{
			index_check(idx);
			return (*this)[idx]; 
		}

		void add(WasmObject* object)
		{
			assert(object->get_type() == type);
			at(import_count + (pos++)) = object;
		}

		void reserve(wasm_uint32_t count)
		{
			assert(items.size() < count);
			if(count > items.size())
				items.resize(count, nullptr); 
		}

		void reserve_definitions(wasm_uint32_t count)
		{
			count += import_count;
			reserve(count);
		}
		
		wasm_uint32_t size() const
		{
			return static_cast<wasm_uint32_t>(imports.size() + items.size());
		}
	private:
		void index_check(std::size_t idx) const 
		{
			if(idx >= size())
			{
				throw std::out_of_range("Attempt to access a " 
					+ type.name() 
					+ " at out-of-bounds index "
					+ std::to_string(idx)
					+ " (max allowed is "
					+ std::to_string(items.size()) 
					+ ") during module instantiation.");
			}
		}
		WasmObjectKind type;
		std::vector<WasmObject*> items(0);
		std::vector<WasmObject*> imports(0);
		wasm_uint32_t pos = 0;
	};

	void double_import_or_export_error(WasmObject* obj, WasmObject* existing, const std::string& imp_or_exp)
	{
		throw std::runtime_error("Attempt to add " 
			+ imp_or_exp + " " + field_name
			+ " with type "
			+ obj->get_type().name()
			+ " to module "
			+ name 
			+ ", but an " 
			+ imp_or_exp
			+ " with that name already exists in module "
			+ name 
			+ " and has type "
			+ existing->get_type().name() 
			+ ".";
	}
	
	ModuleIndexSpace& get_index_space(WasmTypeObject tp)
	{
		return *(std::array<ModuleIndexSpace*, 4>{
			&functions, 
			&tables,
			&memories,
			&globals
		}[tp.value()]);
	}
	std::string name;
	std::unordered_map<std::string, WasmObject*> imports;
	std::unordered_map<std::string, WasmObject*> exports;
	std::vector<WasmFunctionSignature*> signatures;
	ModuleIndexSpace functions(WasmObjectKind::function_typecode);
	ModuleIndexSpace tables(WasmObjectKind::table_typecode);
	ModuleIndexSpace memories(WasmObjectKind::memory_typecode);
	ModuleIndexSpace globals(WasmObjectKind::global_typecode);
};


struct WasmProgramObjects
{
	
	void add_module(wasm_module_def&& module_def, bool is_main = false);
	void instantiate();
private:

	void add_exports();
	void add_imports();
	void add_globals();
	void add_memories();
	void add_functions();
	void add_tables();
	WasmObject* add_empty_object(WasmObjectKind type)
	{
		switch(type.value())
		{
		case WasmObjectKind::function_typecode:
			functions.push_back(std::make_unique<WasmFunctionObject>(nullptr));
			return functions.back().get();
			break;
		case WasmObjectKind::table_typecode:
			tables.push_back(std::make_unique<WasmTableObject>());
			return tables.back().get();
			break;
		case WasmObjectKind::memory_typecode:
			memories.push_back(std::make_unique<WasmMemoryObject>());
			return memories.back().get();
			break;
		case WasmObjectKind::global_typecode:
			globals.push_back(std::make_unique<WasmGlobalObject>());
			return globals.back().get();
			break;
		}
	]

	
	WasmModuleObject& get_module(const std::string& name)
	{ return modules.at(name_map.at(name)); }
	
	const WasmModuleObject& get_module(const std::string& name) const
	{ return modules.at(name_map.at(name)); }
	
	struct SignatureCompare {
		using sig_ptr_t = std::unique_ptr<WasmFunctionSignature>;
		bool operator()(const sig_prt_t & left, const sig_prt_t & right) const 
		{ return (*left) < (*right); }
	};

	std::vector<wasm_module_def> module_defs;
	std::vector<WasmModuleObject> modules;
	std::unordered_map<std::string, std::size_t> name_map;
	FlatSet<std::unique_ptr<WasmFunctionSignature>, SignatureCompare> signatures;
	std::vector<std::unique_ptr<WasmFunctionObject>> functions;
	std::vector<std::unique_ptr<WasmTableObject>> tables;
	std::vector<std::unique_ptr<WasmMemoryObject>> memories;
	std::vector<std::unique_ptr<WasmGlobalObject>> globals;
	std::string main_module_name = "";
	bool instantiated = false;
};


#endif /* PARSE_OBJECTS_WASM_PROGRAM_OBJECTS_H */
