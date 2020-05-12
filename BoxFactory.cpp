#include "Figure.hpp"
#include "Box.hpp"
#include "Factory.hpp"
#include "BoxFactory.hpp"

Figure* BoxFactory::Create()
{
	return new Box();
}