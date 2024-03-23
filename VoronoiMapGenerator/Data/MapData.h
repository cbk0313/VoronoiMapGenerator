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
	Color(double red, double green, double blue, double alpha = 1) : r(red), g(green), b(blue), a(alpha) {}

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

	Color operator +(const Color c) {
		*this = Color(r + c.r, g + c.g, b + c.b, a);
		return *this;
	}

	Color operator +=(Color c) {
		*this = *this + c;
		return *this;
	}

	
};

enum class Terrain : unsigned int {
	OCEAN,
	LAND,
	COAST,
	LAKE
};

struct CellDetail {
public:
	bool is_edge;
	bool is_flat;
	bool is_peak;
private:
	Cell* unionCell;
public:
	unsigned int elevation;
	Terrain terrain;
	Color color;
private:
public:
	/*Cell* getUnionCell() {
		return unionCell;
	}*/
public: 
	CellDetail() : is_edge(false), is_flat(false), is_peak(true), unionCell(nullptr), elevation(0), terrain(Terrain::OCEAN), color(Color(0.2, 0, 0.6, 1)) {};
	CellDetail(Cell* c) : CellDetail() { unionCell = c; };
	Cell* unionFindCell();
	void setUnionCell(Cell* target);
};