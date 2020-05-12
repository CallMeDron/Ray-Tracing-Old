#include "Figure.hpp"
#include "Tetra.hpp"
#include "Factory.hpp"
#include "TetraFactory.hpp"

Figure* TetraFactory::Create()
{
	return new Tetra();
}