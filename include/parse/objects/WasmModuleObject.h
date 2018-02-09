#ifndef PARSE_OBJECTS_WASM_MODULE_OBJECT_H
#define PARSE_OBJECTS_WASM_MODULE_OBJECT_H

struct WasmModuleObject
{
	WasmModuleObject(wasm_module_def&& def):
		definition(std::move(def))
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
	void initialize()
	{
		for(
	}
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
	wasm_module_def definition;
	std::unordered_map<std::string, std::unordered_map<std::string, WasmObjectKind>> imports;
	std::unordered_map<std::string, WasmObject*> exports;
	std::vector<WasmFunctionSignature*> signatures;
	ModuleIndexSpace functions(WasmObjectKind::function_typecode);
	ModuleIndexSpace tables(WasmObjectKind::table_typecode);
	ModuleIndexSpace memories(WasmObjectKind::memory_typecode);
	ModuleIndexSpace globals(WasmObjectKind::global_typecode);
};


#endif /* PARSE_OBJECTS_WASM_MODULE_OBJECT_H */
