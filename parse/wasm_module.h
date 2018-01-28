#ifndef PARSE_WASM_MODULE_H
#define PARSE_WASM_MODULE_H
#include "wasm_base.h"
#include "parse/leb128/leb128.h"
#include <string>
#include <vector>
#include <algorithm>


struct wasm_module_section
{
	const wasm_uint8_t id;
	const std::string name;
	const std::string data;
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

	union magic_cookie_t{ 
		wasm_uint32_t value = 0x647;
		wasm_byte_t bytes[4];
	};
	const magic_cookie_t magic_cookie;
	const wasm_uint32_t = 0x01;
	std::vector<wasm_module_section> sections;
};

template <class CharIt>
[[nodiscard]]
std::pair<wasm_module_section, CharIt>
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
	return wasm_module_section{id, std::move(name), std::move(data)};
}




#endif /* PARSE_WASM_MODULE_H */ 
