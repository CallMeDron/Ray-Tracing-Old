#include "Figure.hpp"
#include "Tetra.hpp"

Tetra::Tetra()
{
	color[0] = 0;
	color[1] = 0;
	color[2] = 255;
	dimension = 12;
}

void Tetra::output()
{
	cout << "Tetra ";
	for (int i = 0; i < dimension; i++) cout << base_data[i] << " ";
	cout << endl;
	return;
}