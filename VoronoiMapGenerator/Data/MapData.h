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
	T* unionFindCell(Terrain t);
	void setUnionCell(Terrain t, T* target);

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


template<typename T, std::size_t N>
T* UnionFind<T, N>::unionFindCell(Terrain t) {
	unsigned int num = static_cast<unsigned int>(t);
	if (cell[num] == cell[num]->detail.unionfind[num]) {
		return cell[num];
	}
	else {
		return cell[num] = cell[num]->detail.unionfind.unionFindCell(t);
	}
}

template<typename T, std::size_t N>
void UnionFind<T, N>::setUnionCell(Terrain t, T* target) {
	unsigned int num = static_cast<unsigned int>(t);
	auto& vim_d = unionFindCell(t)->detail;
	target = target->detail.unionfind.unionFindCell(t);
	vim_d.unionfind[num] = target;
	//if (vim_d.b_edge) target->detail.unionfind[t] = vim_d.unionfind[t];
	//else {
	//	vim_d.unionfind[t] = target;
	//}
}

//typedef UnionFind<Cell, TERRAIN_CNT> UF;

struct CellDetail {

	friend class VoronoiDiagramGenerator;
	friend struct UnionFind<Cell, TERRAIN_CNT>;
private:

	bool b_edge;
	bool b_flat;
	bool b_peak;

	//Cell* unionCell;

	unsigned int elevation;
	unsigned int moisture;
	unsigned int biome;

	Cell* cell;

	Terrain terrain;
	Color color;
	UnionFind<Cell, TERRAIN_CNT> unionfind;

public:
	Terrain getTerrain();
	Color getColor();
	bool isEdge();
	bool isFlat();
	bool isPeak();
	/*Cell* getUnionCell() {
		return unionCell;
	}*/
	void setTerrain(Terrain t) {
		terrain = t;
		switch (t)
		{
		case Terrain::OCEAN:
			if(b_edge)
				color = Color(0.1, 0, 0.3, 1);
			else
				color = Color(0.2, 0, 0.6, 1);
			break;
		case Terrain::LAND:
			color = Color(0.6, 0.4, 0, 1);
			break;
		default:
			break;
		}
	}
private:
	void setEdge(bool b) {
		if (terrain == Terrain::OCEAN) {
			if (b) {
				color = Color(0.1, 0, 0.3, 1);
			}
			else {
				color = Color(0.2, 0, 0.6, 1);
			}
		}
		
		b_edge = b;
	}

public: 
	CellDetail() : b_edge(false), b_flat(false), b_peak(true), cell(nullptr), unionfind(UnionFind<Cell, TERRAIN_CNT>(nullptr)), elevation(0) { setTerrain(Terrain::OCEAN); };
	CellDetail(Cell* c) : CellDetail() {
		cell = c;
		unionfind = UnionFind<Cell, TERRAIN_CNT>(c);
	};

private:
	void reset(bool reset_edge = true, bool reset_terrain = true) {
		if (reset_edge) {
			b_edge = false;
		}
		b_flat = false;
		b_peak = true;
		elevation = 0;
		if (reset_terrain) {
			setTerrain(Terrain::OCEAN);
		}

		for (unsigned int i = 0, cnt = (unsigned int)Terrain::COUNT; i < cnt; i++) {
			unionfind.setUnionCell((Terrain)i, cell);
		}
		

	}
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