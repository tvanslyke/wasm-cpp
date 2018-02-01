#ifndef PARSE_WASM_MODULE_DEF_H
#define PARSE_WASM_MODULE_DEF_H
#include "wasm_base.h"
#include "parse/leb128/leb128.h"
#include <string>
#include <vector>
#include <algorithm>


struct wasm_module_section_def
{
	wasm_uint8_t id;
	std::string name;
	std::string data;
};

enum wasm_module_section_code
{
	MODULE_CUSTOM 		= 0,
	MODULE_TYPE 		= 1,
	MODULE_IMPORT 		= 2,
	MODULE_FUCNTION 	= 3,
	MODULE_TABLE		= 4,
	MODULE_MEMORY		= 5,
	MODULE_GLOBAL		= 6,
	MODULE_EXPORT		= 7,
	MODULE_START		= 8,
	MODULE_ELEMENT		= 9,
	MODULE_CODE		= 10,
	MODULE_DATA		= 11
};

struct wasm_module_def
{
	std::vector<wasm_module_section_def> sections;
};

template <class CharIt>
[[nodiscard]]
std::pair<wasm_module_section_def, CharIt>
parse_module_section(CharIt begin, CharIt end)
{
	assert(begin < end);
	// forward declare the module section fields.
	// this makes the use of std::tie() more readable
	wasm_uint8_t id;
	std::string name;
	std::string data;
	// total length of the section
	wasm_uint32_t payload_len;
	std::tie(id, begin, std::ignore) = leb128_decode_uint7(begin, end);
	assert(begin < end);
	std::tie(payload_len, begin, std::ignore) = leb128_decode_uint32(begin, end);
	if(id == MODULE_CUSTOM)
	{
		assert(begin < end);
		// custom section, extract the 'name'
		wasm_uint32_t name_len;
		std::size_t name_len_bytes = 0;
		std::tie(name_len, begin, name_len_bytes) = leb128_decode_uint32(begin, end);
		assert(begin < end);
		name.resize(name_len);
		std::copy_n(begin, name_len, name.begin());
		assert(payload_len >= (name_len + name_len_bytes));
		payload_len -= (name_len + name_len_bytes);
	}
	data.resize(payload_len);
	std::copy_n(begin, payload_len, data.begin());
	return wasm_module_section_def{id, std::move(name), std::move(data)};
}

template <class CharIt>
CharIt ensure_module_header(CharIt begin, CharIt end)
{
	using char_type = std::iterator_traits<CharIt>::value_type;
	constexpr std::size_t header_size = 8;
	constexpr char_type expect_header[8] = {'\0', 'a', 's', 'm', 0, 0, 0, 1};
	for(std::size_t i = 0; i < 8; ++i)
	{
		if(begin == end)
			throw std::invalid_argument("Provided range is too short to be a WASM module.");
		else if(expect_header[i] != *begin++)
			throw std::invalid_argument("Provided range is not prefixed with the expected WASM header.");
	}
	return begin;
}

template <class CharIt>
wasm_module_def parse_module(CharIt begin, CharIt end)
{
	using char_type = std::iterator_traits<CharIt>::value_type;
	begin = ensure_module_header(begin, end);
	std::vector<module_section_def> sections;
	wasm_uint8_t prev_section_code = 0;
	for(module_section_def current_section; begin != end;)
	{
		std::tie(current_section, begin) = parse_module_section(begin, end);
		auto this_section_code = current_section.id;
		if(this_section_code > 0 and this_section_code <= prev_section_code)
		{
			throw std::runtime_error("Repeated or out-of-order 'known section' "
						 "encountered while parsing module definition");
		}
		sections.push_back(std::move(current_section));
	}
	return wasm_module_def{std::move(sections)};
}



#endif /* PARSE_WASM_MODULE_DEF_H */ 
