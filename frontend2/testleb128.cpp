#include <iterator>
#include <fstream>
#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "binparse.h"

namespace qi = boost::spirit::qi;



std::string leb128_encode_s(std::intmax_t value)
{
	std::string encoding;
	std::uintmax_t uvalue;
	std::memcpy(&uvalue, &value, sizeof(uvalue));
	const bool neg = value < 0;
	for(bool more = true; more; )
	{
		unsigned char byte = uvalue & std::uintmax_t(0b0111'1111u);
		uvalue >>= 7;
		if(neg)
			uvalue |= (~std::uintmax_t(0u)) << (CHAR_BIT * sizeof(std::uintmax_t) - 7);
		bool signbit = static_cast<bool>(byte & 0b0100'0000u);
		if((uvalue == 0 and (not signbit)) or ((uvalue == (~std::uintmax_t(0u))) and signbit))
			more = false;
		else
			byte |= 0b1000'0000u;
		encoding.push_back(byte);
	}
	return encoding;
}


std::string leb128_encode_u(std::uintmax_t value)
{
	std::string encoding;
	do {
		unsigned char byte = static_cast<unsigned char>(value & 0b0111'1111u);
		value >>= 7;
		if(value > 0u)
			byte |= 0b1000'0000u;
		encoding.push_back(byte);
	} while(value > 0u);
	return encoding;
}

void print_encoding(std::string_view sv)
{
	for(auto chr: sv)
		std::cout << (unsigned)chr << ", ";
}

int main(int argc, char** argv)
{
	for(std::uint8_t uv = 0; uv < 0b0111'1111u; ++uv)
	{
		std::string uenc(leb128_encode_u(uv));
		std::uintmax_t v{0};
		assert(qi::parse(begin(uenc), end(uenc), wasm::parse::varuint7[boost::phoenix::ref(v) = qi::_1]));
		if(v != uv)
			std::cerr << std::uintmax_t(uv) << " != " << v << std::endl;
		assert(v == uv);
	}
	
	for(std::uint8_t uv = 0b1000'0000u; uv < 0b1111'1111; ++uv)
	{
		std::string uenc(leb128_encode_u(uv));
		assert(not qi::parse(begin(uenc), end(uenc), wasm::parse::varuint7));
	}
	
	for(std::int8_t sv = -(1 << 6); sv < ((1 << 6) - 1); ++sv)
	{
		std::string senc(leb128_encode_s(sv));
		signed char v;
		bool matched = qi::parse(begin(senc), end(senc), wasm::parse::varint7[boost::phoenix::ref(v) = qi::_1]);
		if(not matched)
		{
			std::cerr << int(sv) << " -> ";
			for(auto chr: senc)
				std::cerr << int(chr) << ", ";
			std::cerr << std::endl;
		}
		assert(matched);
		assert(v == sv);
	}
	
	std::string enc;
	std::uintmax_t uv_in{0};
	std::uintmax_t uv_out{0};
	std::intmax_t sv_in{0};
	std::intmax_t sv_out{0};
	bool matched = false;

	enc = leb128_encode_u(uv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varuint32[boost::phoenix::ref(uv_out) = qi::_1]
	);
	assert(matched);
	assert(uv_in == uv_out);
	
	uv_in = std::numeric_limits<std::uint32_t>::max();
	enc = leb128_encode_u(uv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varuint32[boost::phoenix::ref(uv_out) = qi::_1]
	);
	assert(matched);
	assert(uv_in == uv_out);
	
	uv_in = static_cast<std::uintmax_t>(12345678);
	enc = leb128_encode_u(uv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varuint32[boost::phoenix::ref(uv_out) = qi::_1]
	);
	assert(matched);
	assert(uv_in == uv_out);

	uv_in = static_cast<std::uintmax_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
	enc = leb128_encode_u(uv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varuint32[boost::phoenix::ref(uv_out) = qi::_1]
	);
	assert(not matched);
	
	sv_in = 0;
	enc = leb128_encode_s(sv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varint32[boost::phoenix::ref(sv_out) = qi::_1]
	);
	assert(matched);
	assert(sv_in == sv_out);
	
	sv_in = std::numeric_limits<std::int32_t>::max();
	enc = leb128_encode_u(sv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varint32[boost::phoenix::ref(sv_out) = qi::_1]
	);
	assert(matched);
	assert(sv_in == sv_out);
	
	sv_in = std::numeric_limits<std::int32_t>::min();
	enc = leb128_encode_u(sv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varint32[boost::phoenix::ref(sv_out) = qi::_1]
	);
	assert(matched);
	assert(sv_in == sv_out);
	
	sv_in = 12345678;
	enc = leb128_encode_u(sv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varint32[boost::phoenix::ref(sv_out) = qi::_1]
	);
	assert(matched);
	assert(sv_in == sv_out);

	sv_in = static_cast<std::uintmax_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
	enc = leb128_encode_u(sv_in);
	matched = qi::parse(
		begin(enc), end(enc), 
		wasm::parse::varint32[boost::phoenix::ref(sv_out) = qi::_1]
	);
	assert(not matched);
	

		
	return 0;
}
