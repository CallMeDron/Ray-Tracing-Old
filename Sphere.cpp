#include "Figure.hpp"
#include "Sphere.hpp"

Sphere::Sphere()
{
	color[0] = 255;
	color[1] = 0;
	color[2] = 0;
	dimension = 4;
}

void Sphere::output()
{
	cout << "Sphere ";
	for (int i = 0; i < dimension; i++) cout << base_data[i] << " ";
	cout << endl;
	return;
}