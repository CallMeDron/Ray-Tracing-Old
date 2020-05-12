#include "Figure.hpp"

Figure::Figure()
{
	array<int, 3> arr1 = { 0 };
	color = arr1;
	array<double, 12> arr2 = { 0.0 };
	base_data = arr2;
	dimension = 0;
}

Figure::~Figure() {}