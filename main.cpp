#include "Figure.hpp"
#include "functions.hpp"

int main()
{
	vector<double> settings = GetSettings();
	if (settings.empty()) return -1;

	vector<Figure*> figures = GetFigures();
	if (figures.empty()) return -1;

	for (auto it = figures.begin(); it < figures.end(); it++) (*it)->output();

	RayTracing(settings, figures);

	for (auto it = figures.begin(); it < figures.end(); it++) delete (*it);

	return 0;
}