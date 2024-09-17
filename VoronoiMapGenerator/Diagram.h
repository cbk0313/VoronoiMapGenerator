#ifndef _DIAGRAM_H_
#define _DIAGRAM_H_

#include "MemoryPool/C-11/MemoryPool.h"
//#include "../src/MemoryPool/C-98/MemoryPool.h" //You will need to use this version instead of the one above if your compiler doesn't handle C++11's noexcept operator
#include "Edge.h"
#include "Cell.h"
#include "Data/River.h"
#include "Data/Setting.h"

template<typename T>
struct UnionArray;


struct BoundingBox;
class Diagram;

class Diagram {

	friend class VoronoiDiagramGenerator;
	friend class RiverEdge;
	friend class RiverLine;
	//UDiagram(GenerateSetting& setting) : river_lines(RiverLines(setting)) {}
	Diagram() 
		: ADDED_COUNT(0)
		, max_elevation(1)
		, min_elevation(-1)
		, max_moisture(0)
		, mImageFlag(0)
		, CellUnique(0)
	{}
	GenerateSetting mSetting;

	std::vector<Cell*> cells;
	std::vector<Edge*> edges;
	std::vector<Vertex*> vertices;
	UnionArray<std::vector<Cell*>> oceanUnion;
	UnionArray<IslandUnion> islandUnion;
	RiverLines river_lines;
	RiverCrossingMap river_cross;
	std::vector<Triangle> triangles;

	std::vector<RiverEdge*> RIVER_EDGES;
	std::queue<RiverEdge*> RIVER_DELETE_QUEUE;

	std::vector<RiverLine*> RIVER_LINE_ARR;
	unsigned int ADDED_COUNT;


	int max_elevation;
	int min_elevation;
	unsigned int max_moisture;

	int mImageFlag;

public:
	unsigned int CellUnique;
	void Initialize(GenerateSetting& setting);
	unsigned int GetCellUnique() { return CellUnique; }
	std::vector<Cell*>& GetCells() { return cells; }
	std::vector<Edge*>& GetEdges() { return edges; }
	UnionArray<std::vector<Cell*>>& GetOceanUnion() { return oceanUnion; }
	UnionArray<IslandUnion>& GetIslandUnion() { return islandUnion; }
	RiverLines& GetRiverLines() { return river_lines; }
	RiverCrossingMap& GetRiverCrossing() { return river_cross; }
	std::vector<Triangle>& GetTriangles() { return triangles; }

	int GetMaxElevation() { return max_elevation; }
	unsigned int GetMaxMoisture() { return max_moisture; }
	int GetMinElevation() { return min_elevation; }

	int GetImageFlag() { return mImageFlag; }

	void SetMaxElevation(int new_max_elevation) { max_elevation = new_max_elevation; }
	void SetMaxMoisture(unsigned int new_max_moisture) { max_moisture = new_max_moisture; }
	void SetMinElevation(int new_min_elevation) { min_elevation = new_min_elevation; }

	~Diagram();
	void printDiagram();

	void RiverEdgeClear();
	void RiverLineClear();
	void RiverLineClearJunk();

	const GenerateSetting& GetSetting() const;
private:

	//std::vector<Cell*> tmpCells;
	//std::vector<Edge*> tmpEdges;
	//std::vector<Point2*> tmpVertices;

	MemoryPool<Cell> cellPool;
	MemoryPool<Edge> edgePool;
	MemoryPool<HalfEdge> halfEdgePool;
	MemoryPool<Vertex> vertexPool;

	Vertex* createVertex(double x, double y);
	Cell* createCell(Point2 site, Diagram* diagram);
	Edge* createEdge(Site* lSite, Site* rSite, Vertex* vertA, Vertex* vertB);
	Edge* createBorderEdge(Site* lSite, Vertex* vertA, Vertex* vertB);

	bool connectEdge(Edge* edge, BoundingBox bbox);
	bool clipEdge(Edge* edge, BoundingBox bbox);
	void clipEdges(BoundingBox bbox);
	void closeCells(BoundingBox bbox);
	void finalize();

	template <typename T>
	void freeVecMemory(T& t) {
		T tmp;
		t.swap(tmp);
	}
};

#endif