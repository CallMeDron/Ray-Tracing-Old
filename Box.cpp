#include "Figure.hpp"
#include "Box.hpp"

Box::Box()
{
	color[0] = 0;
	color[1] = 255;
	color[2] = 0;
	dimension = 6;
}

void Box::output()
{
	cout << "Box ";
	for (int i = 0; i < dimension; i++) cout << base_data[i] << " ";
	cout << endl;
	return;
}