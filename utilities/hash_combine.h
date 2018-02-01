#ifndef HASH_COMBINE_H
#define HASH_COMBINE_H
#include <cstddef>

inline std::size_t hash_combine(std::size_t left_hash, std::size_t right_hash)
{
	return left_hash ^ (right_hash + 0x9e3779b9 + (left_hash << 6) + (right_hash >> 2));
}


#endif /* HASH_COMBINE_H */
