#include <benchmark/benchmark.hpp>
#include <cstdint>
#include <array>

std::uint32_t leb128_decode_uint32(const unsigned char* data)
{
	constexpr std::array<std::uint32_t, 5> masks = {
		0b0000'0000'0000'0000'0000'0000'0111'1111u,
		0b0000'0000'0000'0000'0011'1111'1111'1111u,
		0b0000'0000'0001'1111'1111'1111'1111'1111u,
		0b0000'1111'1111'1111'1111'1111'1111'1111u
	};
	std::uint32_t val{0};
	auto byte = *data++;
	val |= std::uint32_t(byte) << 0;
	if(static_cast<bool>(byte & 0b1000'0000u))
		return val & masks[0];
	byte = *data++;
	val |= std::uint32_t(byte) << 7;
	if(static_cast<bool>(byte & 0b1000'0000u))
		return val & masks[1];
	byte = *data++;
	val |= std::uint32_t(byte) << 14;
	if(static_cast<bool>(byte & 0b1000'0000u))
		return val & masks[2];
	byte = *data++;
	val |= std::uint32_t(byte) << 21;
	if(static_cast<bool>(byte & 0b1000'0000u))
		return val & masks[3];
	byte = *data++;
	val |= std::uint32_t(byte) << 28;
	return val;
}



static void BM_leb128_uint32_unrolled_1byte(benchmark::State& state)
{
	for(auto _: state)
	{
		
	}
}
BENCHMARK(BM_leb128_uint32_unrolled_1byte);





