
#ifndef _CELL_H_
#define _CELL_H_

#include "Point2.h"
#include "Data/CellDetail.h"
#include <vector>

struct cellBoundingBox {
	double xmin;
	double ymin;
	double width;
	double height;

	cellBoundingBox(double _xmin, double _ymin, double _xmax, double _ymax) :
		xmin(_xmin), ymin(_ymin), width(_xmax - _xmin), height(_ymax - _ymin) {};
};

struct Cell;
struct Site {
	Point2 p;
	Cell* cell;

	Site() {};
	Site(Point2 _p, Cell* _cell) : p(_p), cell(_cell) {};
};

struct HalfEdge;
struct Cell {

private:
	static unsigned int s_unique;
	unsigned int unique;
	CellDetail detail;
public:
	Site site;
	std::vector<HalfEdge*> halfEdges;
	bool closeMe;



	Cell() : closeMe(false), detail(CellDetail(this)) { unique = s_unique++; };
	Cell(Point2 _site) : site(_site, this), closeMe(false), detail(CellDetail(this)) { unique = s_unique++; };

	unsigned int GetUnique() { return unique; };
	CellDetail& GetDetail() { return detail; };
	//bool IsInQueue

	std::vector<Cell*> getNeighbors();
	cellBoundingBox getBoundingBox();

	// Return whether a point is inside, on, or outside the cell:
	//   -1: point is outside the perimeter of the cell
	//    0: point is on the perimeter of the cell
	//    1: point is inside the perimeter of the cell
	int pointIntersection(double x, double y);

	static bool edgesCCW(HalfEdge* a, HalfEdge* b);
	static void ResetUnique() { s_unique = 0; };
	static unsigned int GetMaxUnique() { return s_unique; };
	static unsigned int GetCellCnt() { return s_unique + 1; };
};



#endif