#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "x3binparse.h"

namespace x3 = boost::spirit::x3;


int main(int argc, char** argv)
{
	using wasm::opc::OpCode;
	using wasm::relax_enum;
	std::string contents(
		{
			char(relax_enum(OpCode::I32_CONST)),
			char(0b0000'0000),
			char(relax_enum(OpCode::IF)), 
			char((unsigned(relax_enum(wasm::LanguageType::i32)) & 0b0111'1111u)),
				char(relax_enum(OpCode::I32_CONST)),
				char(0b0000'0000),
			char(relax_enum(OpCode::ELSE)),
				char(relax_enum(OpCode::I32_CONST)),
				char(0b1111'1111),
				char(0b1111'1111),
				char(0b1111'1111),
				char(0b1111'1111),
				char(0b0000'0111),
			char(relax_enum(OpCode::END)),
			char(relax_enum(OpCode::F32_ADD)),
			char(relax_enum(OpCode::END))
		}
	);
	std::string out;
	try {
		bool matched = x3::parse(
			contents.begin(), 
			contents.end(), 
			wasm::parse::codeparse::function_body_code,
			out
		);
		assert(matched);
	}
	catch(const x3::expectation_failure<std::string::iterator>& e)
	{
		auto pos = e.where();
		std::cerr << "POS = " << (pos - contents.begin()) << " / " << contents.size() << std::endl;
		for(auto it = contents.begin(); it != contents.end(); ++it)
		{
			if(pos == it)
				std::cout << '[';
			std::cout << unsigned((unsigned char)*it);
			if(pos == it)
				std::cout << ']';
			std::cout << ", ";
		}
		std::cout << std::endl;
		throw e;
	}
	wasm::parse::write_code(std::cout, begin(out), end(out), true, true);
	std::cout << std::endl;
	return 0;
}
