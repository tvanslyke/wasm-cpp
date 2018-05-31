#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "x3binparse.h"

namespace x3 = boost::spirit::x3;


// struct Test {
// 	std::optional<std::vector<std::uint32_t>> funcs;
// 	std::optional<std::vector<wasm::parse::ExportEntry>> exports;
// };
// 
// BOOST_FUSION_ADAPT_STRUCT(
// 	Test,
// 	(std::optional<std::vector<std::uint32_t>>, funcs),
// 	(std::optional<std::vector<wasm::parse::ExportEntry>>, exports)
// )
// 	
// 
// inline const auto parser = x3::rule<struct parser_tag2, Test>{}
// 	=  (-wasm::parse::module_section<wasm::parse::SectionType::Function>(wasm::parse::function_section) >> x3::omit[*wasm::parse::module_section<wasm::parse::SectionType::Custom>(wasm::parse::custom_section)])
// 	>> (-wasm::parse::module_section<wasm::parse::SectionType::Export>(wasm::parse::export_section) >> x3::omit[*wasm::parse::module_section<wasm::parse::SectionType::Custom>(wasm::parse::custom_section)]);

int main(int argc, char** argv)
{
	assert(argc == 2);
	std::string contents{
	};
		
	{
		auto file = std::ifstream(argv[1], std::ios::binary | std::ios::in);
		assert(file);
		std::ostringstream str;
		str << file.rdbuf();
		contents = std::move(str.str());
	}
	for(int chr: contents)
		std::cout << static_cast<unsigned>(chr) << ", ";
	std::cout << std::endl;
	wasm::parse::ModuleDef test;
	try {
		bool matched = x3::parse(
			contents.begin(), 
			contents.end(), 
			wasm::parse::module,
			test
		);
		assert(matched);
	}
	catch(const x3::expectation_failure<std::string::iterator>& e)
	{
		auto pos = e.where();
		std::cerr << "POS = " << (pos - contents.begin()) << " / " << contents.size() << std::endl;
		wasm::parse::operator<<((std::cerr << "GOOD = "), std::vector<signed>(contents.begin() + 8, pos + 1)) << std::endl;
		throw e;
	}
	std::cout << test << std::endl;
	return 0;
}
