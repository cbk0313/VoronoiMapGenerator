#ifndef _VORONOI_DIAGRAM_GENERATOR_H_
#define _VORONOI_DIAGRAM_GENERATOR_H_

#include "RBTree.h"
#include "CircleEventQueue.h"
#include "BeachLine.h"
#include "Diagram.h"
#include <vector>

struct BoundingBox {
	double xL;
	double xR;
	double yB;
	double yT;

	BoundingBox() {};
	BoundingBox(double xmin, double xmax, double ymin, double ymax) :
		xL(xmin), xR(xmax), yB(ymin), yT(ymax) {};
};

class VoronoiDiagramGenerator {
public:
	VoronoiDiagramGenerator() : circleEventQueue(nullptr), siteEventQueue(nullptr), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* compute(std::vector<Point2>& sites, BoundingBox bbox);
	Diagram* relax(Diagram* diagram);

	Diagram* relaxLoop(int num, Diagram* diagram);

	void SetLand(int seed, double radius, Diagram* diagram);
private:
	//Diagram* diagram;
	CircleEventQueue* circleEventQueue;
	std::vector<Point2*>* siteEventQueue;
	BoundingBox	boundingBox;

	void printBeachLine();

	//BeachLine
	RBTree<BeachSection>* beachLine;
	treeNode<BeachSection>* addBeachSection(Site* site, Diagram* diagram);
	inline void detachBeachSection(treeNode<BeachSection>* section);
	void removeBeachSection(treeNode<BeachSection>* section, Diagram* diagram);
	double leftBreakpoint(treeNode<BeachSection>* section, double directrix);
	double rightBreakpoint(treeNode<BeachSection>* section, double directrix);

	inline double calcDistance(Point2& a, Point2& b);
};

inline double VoronoiDiagramGenerator::calcDistance(Point2& a, Point2& b) {
	double x = (b.x - a.x);
	double y = (b.y - a.y);
	return sqrt((x * x) + (y * y));
}


inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

#endif