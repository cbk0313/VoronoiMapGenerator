#pragma once

struct Cell;

struct Color
{
public:
	double r;
	double g;
	double b;
	double a;

	Color() : r(1), g(1), b(1), a(1) {};
	Color(double red, double green, double blue, double alpha) : r(red), g(green), b(blue), a(alpha) {}

	Color operator *(const int num) {
		return (*this) * ((double)num);
	}
	Color operator *(const double num) {
		*this = Color(r * num, g * num, b * num, a);
		return *this;
	}
	Color operator *=(int num) { 
		*this = *this * num;
		return *this;
	}

	Color operator *=(double num) {
		*this = *this * num;
		return *this;
	}

	
};

enum class Terrain : unsigned int {
	OCEAN,
	LAND,
	COAST
};

struct CellDetail {
public:
	bool outer;
	
	unsigned int elevation;
	Terrain terrain;
	Color color;

private:
	Cell* unionCell;

	/*Cell* getUnionCell() {
		return unionCell;
	}*/
public: 
	CellDetail() : unionCell(nullptr), outer(false), elevation(0), terrain(Terrain::OCEAN), color(Color(0.2, 0, 0.6, 1)) {};
	CellDetail(Cell* c) : CellDetail() { unionCell = c; };
	Cell* findUnionCell();
	void setUnionCell(Cell* target);
};