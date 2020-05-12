#include <array>
using namespace std;

class Figure
{
public:
	virtual void output() = 0;
	array<double, 12> base_data;
	array<int, 3> color;
	int dimension;
	virtual ~Figure();
	Figure();
};