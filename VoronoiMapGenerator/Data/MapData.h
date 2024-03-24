#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>


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

enum class Terrain : std::size_t {
	OCEAN,
	LAND,
	COAST,
	LAKE,
	PEAK,

	COUNT // it's mean enum count
};

#define TERRAIN_CNT static_cast<int>(Terrain::COUNT)

#define IS_LAND(x) (x == Terrain::LAND || x == Terrain::PEAK)
#define IS_OCEAN(x) (x == Terrain::LAKE || x == Terrain::COAST)

template<typename T, std::size_t N>
struct UnionFind {

private:
	T* cell[N];
public:
	T* unionFindCell(std::size_t t);
	void setUnionCell(std::size_t t, T* target);

	UnionFind(T* c) {
		for (int i = 0; i < N; i++) {
			cell[i] = c;
		}
	}

	T*& operator[](std::size_t index) {
		return cell[index];
	}

	const T*& operator[](std::size_t index) const {
		return cell[index];
	}
};


struct CellDetail {
public:
	bool is_edge;
	bool is_flat;
	bool is_peak;
private:
	//Cell* unionCell;

public:
	unsigned int elevation;
	Terrain terrain;
	Color color;
	UnionFind<Cell, TERRAIN_CNT> unionfind;

	/*Cell* getUnionCell() {
		return unionCell;
	}*/
public: 
	CellDetail() : is_edge(false), is_flat(false), is_peak(true), unionfind(UnionFind<Cell, TERRAIN_CNT>(nullptr)), elevation(0), terrain(Terrain::OCEAN), color(Color(0.2, 0, 0.6, 1)) {};
	CellDetail(Cell* c) : CellDetail() { unionfind = UnionFind<Cell, TERRAIN_CNT>(c); };

	//Cell* unionFindCell();
	//void setUnionCell(Cell* target);
};


template<typename T>
struct UnionArray {
	
	std::unordered_map<unsigned int, T> unions;

	UnionArray() {}
	T* findUnion(unsigned int uniuqe) {
		auto item = unions.find(uniuqe);
		if (item == unions.end()) {
			return nullptr;
		}
		return &item->second;
	}
	T* insert(unsigned int uniuqe) {
		T* uni = findUnion(uniuqe);
		if (uni == nullptr) {
			unions.insert(std::make_pair(uniuqe, T()));
			uni = findUnion(uniuqe);
		}
		return uni;
	}
};

struct IslandUnion {
	std::vector<Cell*> land;
	UnionArray<std::vector<Cell*>> peakUnion;
	UnionArray<std::vector<Cell*>> lakeUnion;
};