#ifndef BINPARSE_TYPES_H
#define BINPARSE_TYPES_H

#include <optional>
#include <variant>
#include <ostream>
#include <tuple>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <boost/fusion/include/adapt_struct.hpp>
#include "../../include/wasm_base.h"
#include "../../include/WasmInstruction.h"

namespace wasm::parse {

struct GlobalDef;
struct ResizableLimits;
struct GlobalType;
struct Table;
struct Memory;
struct FunctionSignature;
struct ImportEntry;
struct GlobalEntry;
struct ExportEntry;
struct ElemSegment;
struct DataSegment;
struct LocalEntry;
struct FunctionBody;
struct ModuleDef;

std::ostream& operator<<(std::ostream& os, const GlobalDef&);
std::ostream& operator<<(std::ostream& os, const ResizableLimits&);
std::ostream& operator<<(std::ostream& os, const GlobalType&);
std::ostream& operator<<(std::ostream& os, const Table&);
std::ostream& operator<<(std::ostream& os, const Memory&);
std::ostream& operator<<(std::ostream& os, const FunctionSignature&);
std::ostream& operator<<(std::ostream& os, const ImportEntry&);
std::ostream& operator<<(std::ostream& os, const GlobalEntry&);
std::ostream& operator<<(std::ostream& os, const ExportEntry&);
std::ostream& operator<<(std::ostream& os, const ElemSegment&);
std::ostream& operator<<(std::ostream& os, const DataSegment&);
std::ostream& operator<<(std::ostream& os, const LocalEntry&);
std::ostream& operator<<(std::ostream& os, const FunctionBody&);
std::ostream& operator<<(std::ostream& os, const ModuleDef&);

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
	os << '[';
	if(vec.size() > 0)
	{
		os << vec.front();
		for(std::size_t i = 1; i < vec.size(); ++i)
			os << ", " << vec[i];
	}
	os << ']';
	return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt)
{
	if(opt)
		os << *opt;
	else
		os << "None";
	return os;
}

template <class ... T, class = std::enable_if_t<(sizeof...(T) > 0u)>>
std::ostream& operator<<(std::ostream& os, const std::variant<T...>& var)
{
	std::visit([&](const auto& value) -> void { os << value; }, var);
	return os;
}

} /* namespace wasm::parse */


struct wasm::parse::GlobalDef
{
	using value_t = std::variant<
		wasm_sint32_t, 
		wasm_sint64_t, 
		wasm_float32_t, 
		wasm_float64_t
	>;
	value_t value;
	bool is_const;
};

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::GlobalDef,
	(wasm::parse::GlobalDef::value_t, value),
	(bool, is_const)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const GlobalDef& def)
{
	os << "GlobalDef(value = " << def.value << ", is_const = " << def.is_const << ')';
	return os;
}

struct wasm::parse::ResizableLimits {
	std::uint32_t initial;
	std::optional<std::uint32_t> maximum;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::ResizableLimits,
	(std::uint32_t, initial),
	(std::optional<std::uint32_t>, maximum)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const ResizableLimits& lims)
{
	return os << "ResizableLimits(initial = " << lims.initial << ", maximum = " << lims.maximum << ')';
}

struct wasm::parse::GlobalType:
	public std::pair<LanguageType, bool>
{
	using std::pair<LanguageType, bool>::pair;
	static constexpr const ExternalKind external_kind_v 
		= LanguageType::Global;

	const LanguageType& type() const
	{ return first; }

	LanguageType& type()
	{ return first; }

	const bool& is_const() const
	{ return second; }

	bool& is_const()
	{ return second; }
};

GlobalType global_type(const GlobalDef& def)
{
	assert(not def.value.valueless_by_exception());
	idx = def.value.index();
	assert(idx < 4u);
	auto tp = std::holds_alternative<std::int32_t>(def.value) ? LanguageType::i32
		: std::holds_alternative<std::int64_t>(def.value) ? LanguageType::i64
		: std::holds_alternative<float>(def.value) ? LanguageType::f32
		: LanguageType::f64;
	return GlobalType(tp, def.is_const);
}

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::GlobalType,
	(wasm::LanguageType, first),
	(bool, second)
)

std::ostream& wasm::parse::operator<<(std::ostream& os, const GlobalType& glob)
{
	return os << "GlobalType(type = " << glob.type() << ", is_const = " << glob.is_const() << ')';
}


struct wasm::parse::Table:
	public ResizableLimits
{
	static constexpr const ExternalKind external_kind_v 
		= LanguageType::Table;
	using ResizableLimits::ResizableLimits;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::Table,
	(std::uint32_t, initial),
	(std::optional<std::uint32_t>, maximum)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const Table& table)
{
	return os << "Table(" << static_cast<const ResizableLimits&>(table) << ")";
}

struct wasm::parse::Memory:
	public ResizableLimits
{
	static constexpr const ExternalKind external_kind_v 
		= LanguageType::Memory;
	using ResizableLimits::ResizableLimits;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::Memory,
	(std::uint32_t, initial),
	(std::optional<std::uint32_t>, maximum)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const Memory& mem)
{
	return os << "Memory(" << static_cast<const ResizableLimits&>(mem) << ")";
}

struct wasm::parse::FunctionSignature {
	std::basic_string<LanguageType> param_types;
	std::optional<LanguageType> return_type;
};

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::FunctionSignature,
	(std::basic_string<wasm::LanguageType>, param_types),
	(std::optional<wasm::LanguageType>,     return_type)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const FunctionSignature& sig)
{
	os << "FunctionSignature(params = (";
	if(sig.param_types.size() > 0)
	{
		std::basic_string_view<LanguageType> view(sig.param_types);
		os << view.front();
		view.remove_prefix(1);
		for(const auto& tp: view)
			os << ", " << tp;
	}
	os << "), return_type = " << sig.return_type << ')';
	return os;
}

struct wasm::parse::ImportEntry {
	using variant_t = std::variant<std::uint32_t, Table, Memory, GlobalType>;
	std::string module_name;
	std::string field_name;
	variant_t entry_type;

	ExternalKind kind() const
	{
		switch(entry_type.index())
		{	
		case 0u:
			return ExternalKind::Function;
		case 1u:
			return ExternalKind::Table;
		case 2u:
			return ExternalKind::Memory;
		case 3u:
			return ExternalKind::Global;
		default:
			assert(false);
		}
	}
};

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::ImportEntry,
	(std::string, module_name),
	(std::string, field_name),
	(wasm::parse::ImportEntry::variant_t, entry_type)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const ImportEntry& ent)
{
	os << "ImportEntry(module_name = \"" << ent.module_name << '\"';
	os << ", field_name = \"" << ent.field_name << '\"';
	os << ", entry_type = " << ent.entry_type << ')';
	return os;
}

struct wasm::parse::GlobalEntry {
	GlobalDef value;
	std::optional<std::uint32_t> depends;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::GlobalEntry,
	(wasm::parse::GlobalDef, value),
	(std::optional<std::uint32_t>, depends)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const GlobalEntry& ent)
{
	return os << "GlobalEntry(value = " << ent.value << ", depends = " << ent.depends << ')';
}

struct wasm::parse::ExportEntry {
	std::string name;
	wasm::ExternalKind kind;
	std::uint32_t index;
};

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::ExportEntry,
	(std::string, name),
	(wasm::ExternalKind, kind),
	(std::uint32_t, index)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const ExportEntry& ent)
{
	os << "ExportEntry(name = \"" << ent.name << '\"';
	os << ", kind = " << ent.kind;
	os << ", index = " << ent.index;
	os << ')';
	return os;
}

struct wasm::parse::ElemSegment {
	using variant_t = std::variant<std::int32_t, std::uint32_t>;
	std::uint32_t index;
	variant_t offset;
	std::vector<std::uint32_t> indices;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::ElemSegment,
	(std::uint32_t, index),
	(wasm::parse::ElemSegment::variant_t, offset),
	(std::vector<std::uint32_t>, indices)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const ElemSegment& seg)
{
	os << "ElemSegment(index = " << seg.index;
	os << ", offset = ";
	if(seg.offset.index() == 0)
		os << seg.offset;
	else
		os << "Globals[" << std::get<1>(seg.offset) << ']';
	
	os << ", indices = " << seg.indices;
	os << ')';
	return os;
}

struct wasm::parse::DataSegment {
	using variant_t = std::variant<std::int32_t, std::uint32_t>;
	std::uint32_t index;
	variant_t offset;
	std::vector<std::uint8_t> data;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::DataSegment,
	(std::uint32_t, index),
	(wasm::parse::DataSegment::variant_t, offset),
	(std::vector<std::uint8_t>, data)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const DataSegment& seg)
{
	os << "DataSegment(index = " << seg.index;
	os << ", offset = ";
	if(seg.offset.index() == 0)
		os << seg.offset;
	else
		os << "Globals[" << std::get<1>(seg.offset) << ']';
	os << ", data = " << seg.data;
	os << ')';
	return os;
}

struct wasm::parse::LocalEntry {
	std::uint32_t count;
	LanguageType type;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::LocalEntry,
	(std::uint32_t, count),
	(wasm::LanguageType, type)
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const LocalEntry& ent)
{
	os << "LocalEntry(count = " << ent.count;
	os << ", type = " << ent.type;
	os << ')';
	return os;
}

struct wasm::parse::FunctionBody {
	std::basic_string<wasm::LanguageType> locals;
	std::string code;
};
BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::FunctionBody,
	(std::vector<wasm::parse::LocalEntry>, locals)
	(std::string, code)
)

std::ostream& wasm::parse::operator<<(std::ostream& os, const FunctionBody& body)
{
	os << "FunctionBody(locals = " << body.locals;
	std::vector<unsigned> code(body.code.size());
	std::transform(body.code.begin(), body.code.end(), code.begin(),
		[](auto c){ return static_cast<unsigned char>(c); }
	);
	os << ", code = " << code;
	os << ')';
	return os;
}

struct wasm::parse::ModuleDef {
	std::string module_name;
	std::optional<std::vector<FunctionSignature>> type_section;
	std::optional<std::vector<ImportEntry>>       import_section;
	std::optional<std::vector<std::uint32_t>>     function_section;
	std::optional<std::vector<Table>>             table_section;
	std::optional<std::vector<Memory>>            memory_section;
	std::optional<std::vector<GlobalEntry>>       global_section;
	std::optional<std::vector<ExportEntry>>       export_section;
	std::optional<std::uint32_t>                  start_section;
	std::optional<std::vector<ElemSegment>>       element_section;
	std::optional<std::vector<FunctionBody>>      code_section;
	std::optional<std::vector<DataSegment>>       data_section;
};

BOOST_FUSION_ADAPT_STRUCT(
	wasm::parse::ModuleDef,
	(std::optional<std::vector<wasm::parse::FunctionSignature>>, type_section)
	(std::optional<std::vector<wasm::parse::ImportEntry>>      , import_section),
	(std::optional<std::vector<std::uint32_t>>                 , function_section),
	(std::optional<std::vector<wasm::parse::Table>>            , table_section),
	(std::optional<std::vector<wasm::parse::Memory>>           , memory_section),
	(std::optional<std::vector<wasm::parse::GlobalEntry>>      , global_section),
	(std::optional<std::vector<wasm::parse::ExportEntry>>      , export_section),
	(std::optional<std::uint32_t>                              , start_section),
	(std::optional<std::vector<wasm::parse::ElemSegment>>      , element_section),
	(std::optional<std::vector<wasm::parse::FunctionBody>>     , code_section),
	(std::optional<std::vector<wasm::parse::DataSegment>>      , data_section),
)
std::ostream& wasm::parse::operator<<(std::ostream& os, const ModuleDef& body)
{
	os << "ModuleDef(\n\t";
	os << "type_section = "       << body.type_section;
	os << ",\n\timport_section = "   << body.import_section;
	os << ",\n\tfunction_section = " << body.function_section;
	os << ",\n\ttable_section = "    << body.table_section;
	os << ",\n\tmemory_section = "   << body.memory_section;
	os << ",\n\tglobal_section = "   << body.global_section;
	os << ",\n\texport_section = "   << body.export_section;
	os << ",\n\tstart_section = "    << body.start_section;
	os << ",\n\telement_section = "  << body.element_section;
	os << ",\n\tcode_section = "     << body.code_section;
	os << ",\n\tdata_section = "     << body.data_section;
	os << "\n)";
	return os;
}

#endif /* BINPARSE_TYPES_H */
