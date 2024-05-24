#ifndef _DIAGRAM_H_
#define _DIAGRAM_H_

#include "MemoryPool/C-11/MemoryPool.h"
//#include "../src/MemoryPool/C-98/MemoryPool.h" //You will need to use this version instead of the one above if your compiler doesn't handle C++11's noexcept operator
#include "Edge.h"
#include "Cell.h"

template<typename T>
struct UnionArray;


struct BoundingBox;
class Diagram {
public:
	std::vector<Cell*> cells;
	std::vector<Edge*> edges;
	std::vector<Vertex*> vertices;
	UnionArray<std::vector<Cell*>> oceanUnion;
	UnionArray<IslandUnion> islandUnion;
	std::vector<Edge*> river_edges;


	~Diagram();
	void printDiagram();
private:
	friend class VoronoiDiagramGenerator;

	//std::vector<Cell*> tmpCells;
	//std::vector<Edge*> tmpEdges;
	//std::vector<Point2*> tmpVertices;

	MemoryPool<Cell> cellPool;
	MemoryPool<Edge> edgePool;
	MemoryPool<HalfEdge> halfEdgePool;
	MemoryPool<Vertex> vertexPool;

	Vertex* createVertex(double x, double y);
	Cell* createCell(Point2 site);
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