#include "Figure.hpp"
#include "Sphere.hpp"
#include "Factory.hpp"
#include "SphereFactory.hpp"

Figure* SphereFactory::Create()
{
	return new Sphere();
}