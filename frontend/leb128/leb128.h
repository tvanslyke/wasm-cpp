#ifndef LEB128_H
#define LEB128_H

const ubyte_t* 
leb128_parse_unsigned(const ubyte_t* begin, const ubyte_t* end, uint_least64_t* dest, size_t width);

const ubyte_t* 
leb128_parse_signed(const ubyte_t* begin, const ubyte_t* end, int_least64_t* dest, size_t width);

#endif /* LEB128_H */
