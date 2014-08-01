#pragma once

namespace invader {

class ShapingTransform
{
public:
	ShapingTransform(double t) { a = 1.0f - 1.0f/t; };
	double operator()(double value) { return value/(value + a*(value - 1.0f)); };

private:
	double a;
};

class LimitedShapingTransform
{
public:
	LimitedShapingTransform(double t) : shape(t) {};
	double operator()(double value) {
		if (value >= 1.0)
			return 1.0;
		else if (value <= -1.0)
			return -1.0;
		else if (value >= 0)
			return shape(value); 
		else
			return -shape(-value); 
	};

private:
	ShapingTransform shape;
};

} // namespace invader