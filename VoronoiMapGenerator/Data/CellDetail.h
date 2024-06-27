#pragma once


#include <vector>
#include <unordered_map>
#include <iostream>

#include "Color.h"
#include "UnionFind.h"

struct Cell;

enum class Terrain : std::size_t {
	OCEAN,
	LAND,
	COAST,
	LAKE,
	PEAK,
	HIGHEST_PEAK,
	FLAT,

	COUNT // it's mean enum count
};

#define TERRAIN_CNT static_cast<int>(Terrain::COUNT)

#define IS_GROUND(x) (x == Terrain::LAND || x == Terrain::PEAK || x == Terrain::HIGHEST_PEAK)
#define IS_LAND(x) (IS_GROUND(x) || x == Terrain::LAKE )
#define IS_WATER(x) (x == Terrain::OCEAN || x == Terrain::LAKE || x == Terrain::COAST)
#define IS_OCEAN(x) (x == Terrain::OCEAN || x == Terrain::COAST)

#define VERTEX_DECREASE_COLOR Color(0.2, 0.2, 0.2)

#define COAST_ELEVATION -1
#define OCEAN_ELEVATION -2
#define LAND_MIN_ELEVATION 1

//typedef UnionFind<Cell, TERRAIN_CNT> UF;

struct CellDetail {

	//friend class VoronoiDiagramGenerator;
	friend struct UnionFind<Cell, TERRAIN_CNT>;
private:

	bool b_edge;
	bool b_flat;
	bool b_peak;
	bool b_highest_peak;

	//Cell* unionCell;

	int elevation;
	unsigned int moisture;
	//unsigned int localMoisture;
	//unsigned int areaMoisture;
	unsigned int biome;

	Cell* cell;

	Terrain terrain;
	Color color;
	UnionFind<Cell, TERRAIN_CNT> unionfind;

public:
	Terrain GetTerrain();
	Color& GetColor();
	bool IsEdge();
	bool GetEdge();

	bool IsFlat();
	void SetFlat(bool b);

	bool IsPeak();
	bool GetPeak();
	void SetPeak(bool b);

	bool IsHighestPeak();
	void SetHighestPeak(bool b);
	bool GetHighestPeak();

	int GetElevation();
	void SetElevation(int num);
	void AddElevation(int num);

	unsigned int GetMoisture();
	void SetMoisture(unsigned int num);
	void AddMoisture(unsigned int num);
	//unsigned int GetAreaMoisture();
	//unsigned int GetLocalMoisture();
	//void SetAreaMoisture(unsigned int num);
	//void AddAreaMoisture(unsigned int num);
	//void SetLocalMoisture(unsigned int num);
	//void AddLocalMoisture(unsigned int num);

	unsigned int GetBiome();
	void SetBiome(unsigned int num);
	void AddBiome(unsigned int num);

	UnionFind<Cell, TERRAIN_CNT>& GetUnionFind();
	Cell* UnionFindCell(Terrain t);
	CellDetail& UnionFindCellDetail(Terrain t);
	void SetUnionCell(Terrain t, Cell* c);
	/*Cell* getUnionCell() {
		return unionCell;
	}*/
	void SetTerrain(Terrain t);
	void SetEdge(bool b);
	void Reset(bool reset_edge = true, bool reset_terrain = true, bool reset_elev = true);

	CellDetail() : b_edge(false), b_flat(false), b_peak(false), b_highest_peak(false), cell(nullptr), unionfind(UnionFind<Cell, TERRAIN_CNT>(nullptr)), elevation(0), moisture(0), biome(0) { SetTerrain(Terrain::OCEAN); };
	CellDetail(Cell* c) : CellDetail() {
		cell = c;
		unionfind = UnionFind<Cell, TERRAIN_CNT>(c);
	};
	bool CheckSurroundRising();
	//Cell* UnionFindCell();
	//void SetUnionCell(Cell* target);
};


struct IslandUnion {
	std::vector<Cell*> land;
	UnionArray<std::vector<Cell*>> highestPeakUnion;
	UnionArray<std::vector<Cell*>> peakUnion;
	UnionArray<std::vector<Cell*>> lakeUnion;
};