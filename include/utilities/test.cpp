#include "TransformIterator.h"
#include <vector>
#include <iostream>

int main()
{
	std::vector<int> v{1, 2, 3, 4, 5};
	auto tform = [](int x) { return double(x) * x; };
	std::vector<double> sq(
		wasm::make_transform_iterator(v.begin(), tform), 
		wasm::make_transform_iterator(v.end(), tform)
	);
	for(auto x: sq)
		std::cout << x << std::endl;
}
