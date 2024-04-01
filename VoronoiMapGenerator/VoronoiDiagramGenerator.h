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
	VoronoiDiagramGenerator() : circleEventQueue(nullptr), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* compute(std::vector<Point2>& sites, BoundingBox bbox);
	Diagram* relax(Diagram* diagram);

	Diagram* relaxLoop(int num, Diagram* diagram);

	void createWorld(int seed, double radius, double lakeScale, double lakeSize, Diagram* diagram);
private:
	//Diagram* diagram;
	CircleEventQueue* circleEventQueue;
	//std::vector<Point2*>* siteEventQueue;
	BoundingBox	boundingBox;

	void printBeachLine();

	//BeachLine
	RBTree<BeachSection>* beachLine;
	treeNode<BeachSection>* addBeachSection(Site* site, Diagram* diagram);
	inline void detachBeachSection(treeNode<BeachSection>* section);
	void removeBeachSection(treeNode<BeachSection>* section, Diagram* diagram);
	double leftBreakpoint(treeNode<BeachSection>* section, double directrix);
	double rightBreakpoint(treeNode<BeachSection>* section, double directrix);

	inline double calcDistance(const Point2& a, const Point2& b);
	inline double getRandom();

	void setupOcean(Diagram* diagram);

	void initWorld(int seed, double radius, double lakeScale, double lakeSize, unsigned int p_cnt, double p_max_r, double p_min_r, Diagram* diagram);
	void createRiver(Diagram* diagram);
	std::pair<double, double> getMinDist(std::vector<std::pair<Point2, double>>& points, Point2& center, double radius);
	
};

inline double VoronoiDiagramGenerator::calcDistance(const Point2& a, const Point2& b) {
	double x = (b.x - a.x);
	double y = (b.y - a.y);
	return sqrt((x * x) + (y * y));
}

inline double VoronoiDiagramGenerator::getRandom() {
	return rand() / ((double)RAND_MAX);
}


inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

#endif