#pragma once
#include <vector>

class CharColor;

class Color
{
public:
	double r;
	double g;
	double b;
	double a;

	Color() : r(0), g(0), b(0), a(1) {};
	Color(double red, double green, double blue, double alpha = 1) : r(red), g(green), b(blue), a(alpha) {}

	Color operator *(const int num);
	Color operator *(const double num);
	Color operator *=(int num);
	Color operator *=(double num);

	Color operator /(const int num);
	Color operator /(const double num);
	Color operator /=(int num);
	Color operator /=(double num);

	Color operator +(const Color& c);
	Color operator +=(Color& c);

	Color operator -(const Color& c);
	Color operator -=(Color& c);

	bool operator ==(Color& c);
	static Color MixColor(Color& c1, Color& c2);
	static Color lerp(const Color& a, const Color& b, double t);
	operator CharColor() const;

	static const Color black;
	static const Color lake;
	static const Color white;
	static const Color edgeOcean;
	static const Color ocean;
	static const Color coast;
};


class CharColor {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	CharColor(unsigned char c_r, unsigned char c_g, unsigned char c_b) : r(c_r), g(c_g), b(c_b) {};
	CharColor& operator=(const Color& a);
	CharColor& operator=(const Color* a);
};

struct AvgColor {
private:
	std::vector<Color> color_list;
	Color color;
	int num;
	
public:
	
	AvgColor() : color(Color(0, 0, 0, 1)), num(0) {};


	void operator +=(Color c) {
		c = Color(round(c.r * 100) / 100, round(c.g * 100) / 100, round(c.b * 100) / 100, 1);
		/*bool find = false;
		for (Color& temp : color_list) {
			if (temp == c) {
				find = true;
				break;
			}
		}
		if (!find) {
			color += c;
			num++;
			color_list.push_back(c);
		}*/
		color += c;
		num++;
	}

	Color GetColor() {
		//std::cout << (color).r << " " << (color).g << " " << (color).b << "\n";
		//std::cout << (color).r / num << " " << (color).g / num << " " << (color).b / num << "\n";
		//std::cout << (color / num).r << " " << (color / num).g << " " << (color / num).b << "\n\n";
		//std::cout << "num: " << num << "\n";
		return (color / num);
	}

	int GetNum() {
		return num;
	}
};