class Factory
{
public:
	virtual Figure* Create() = 0;
	virtual ~Factory() {}
};
