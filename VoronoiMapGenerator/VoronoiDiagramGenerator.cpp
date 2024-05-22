#include "VoronoiDiagramGenerator.h"
#include "Vector2.h"
#include "FastNoise/FastNoiseLite.h"
#include "Epsilon.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include "Data/Buffer.h"
#include "Data/Triangle.h"
#include <Windows.h>

using std::cout;
using std::cin;
using std::endl;

using DistRadius = std::pair<double, double>; // fisrt: distance, second: radius
using PointDist = std::pair<Point2, double>; // first: point, second: radius


Diagram* VoronoiDiagramGenerator::GetDiagram() {
	return diagram;
}

void VoronoiDiagramGenerator::printBeachLine() {
	treeNode<BeachSection>* section = beachLine->getFirst(beachLine->getRoot());

	while (section) {
		cout << section->data.site->p << endl;
		section = section->next;
	}
	if (section) cout << section->data.site->p << endl;
	cout << endl << endl;
}

bool pointComparator(Point2* a, Point2* b) {
	double r = b->y - a->y;
	if (r < 0) return true;
	else if (r == 0) {
		if (b->x - a->x < 0) return true;
		else return false;
	}
	else return false;
}

void VoronoiDiagramGenerator::compute(std::vector<Point2>& sites, BoundingBox bbox, bool reset) {
	if (reset && diagram) {
		delete diagram;
		diagram = nullptr;
	}
	//siteEventQueue = new std::vector<Point2*>();
	std::vector<Point2*> siteEventQueue = std::vector<Point2*>();
	siteEventQueue.reserve(sites.size());
	boundingBox = bbox;

	for (size_t i = 0; i < sites.size(); ++i) {
		//sanitize sites by quantizing to integer multiple of epsilon
		sites[i].x = round(sites[i].x / EPSILON) * EPSILON;
		sites[i].y = round(sites[i].y / EPSILON) * EPSILON;

		siteEventQueue.push_back(&(sites[i]));
	}

	diagram = new Diagram();
	circleEventQueue = new CircleEventQueue();
	beachLine = new RBTree<BeachSection>();

	std::sort(siteEventQueue.begin(), siteEventQueue.end(), pointComparator);
	// Initialize site event queue


	// process queue
	Point2* site = siteEventQueue.empty() ? nullptr : siteEventQueue.back();
	if (!siteEventQueue.empty()) siteEventQueue.pop_back();
	treeNode<CircleEvent>* circle;

	// main loop
	for (;;) {
		// figure out whether to handle a site or circle event
		// for this we find out if there is a site event and if it is
		// 'earlier' than the circle event
		circle = circleEventQueue->firstEvent;

		// add beach section
		if (site && (!circle || site->y < circle->data.y || (site->y == circle->data.y && site->x < circle->data.x))) {


			// first create cell for new site
			Cell* cell = diagram->createCell(*site);
			// then create a beachsection for that site
			addBeachSection(&cell->site);
			site = siteEventQueue.empty() ? nullptr : siteEventQueue.back();
			if (!siteEventQueue.empty()) siteEventQueue.pop_back();
		}

		// remove beach section
		else if (circle)
			removeBeachSection(circle->data.beachSection);

		// all done, quit
		else
			break;
	}

	// wrapping-up:
	//   connect dangling edges to bounding box
	//   cut edges as per bounding box
	//   discard edges completely outside bounding box
	//   discard edges which are point-like

	diagram->clipEdges(boundingBox);

	//   add missing edges in order to close open cells
	diagram->closeCells(boundingBox);

	//diagram->finalize();

	delete circleEventQueue;
	circleEventQueue = nullptr;

	//delete siteEventQueue;
	//siteEventQueue = nullptr;

	delete beachLine;
	beachLine = nullptr;
}

bool halfEdgesCW(HalfEdge* e1, HalfEdge* e2) {
	return e1->angle < e2->angle;
}

void  VoronoiDiagramGenerator::relax() {
	std::vector<Point2> sites;
	std::vector<Point2> verts;
	std::vector<Vector2> vectors;
	//replace each site with its cell's centroid:
	//    subdivide the cell into adjacent triangles
	//    find those triangles' centroids (by averaging corners) 
	//    and areas (by computing vector cross product magnitude)
	//    combine the triangles' centroids through weighted average
	//	  to get the whole cell's centroid
	for (Cell* c : diagram->cells) {
		size_t edgeCount = c->halfEdges.size();
		verts.resize(edgeCount);
		vectors.resize(edgeCount);

		for (size_t i = 0; i < edgeCount; ++i) {
			verts[i] = c->halfEdges[i]->startPoint()->point;
			vectors[i] = c->halfEdges[i]->startPoint()->point - verts[0];
		}

		Point2 centroid(0.0, 0.0);
		double totalArea = 0.0;
		for (size_t i = 1; i < edgeCount - 1; ++i) {
			double area = (vectors[i + 1].x * vectors[i].y - vectors[i + 1].y * vectors[i].x) / 2;
			totalArea += area;
			centroid.x += area * (verts[0].x + verts[i].x + verts[i + 1].x) / 3;
			centroid.y += area * (verts[0].y + verts[i].y + verts[i + 1].y) / 3;
		}
		centroid.x /= totalArea;
		centroid.y /= totalArea;
		sites.push_back(centroid);
	}

	//then recompute the diagram using the cells' centroids
	delete diagram;
	diagram = nullptr;

	compute(sites, boundingBox, false);

}


void  VoronoiDiagramGenerator::relaxLoop(int num) {
	for (int i = 0; i < num; i++) {
		relax();
	}
}



void VoronoiDiagramGenerator::CreateWorld() {
	if (diagram) {
		srand(setting.seed);
		CreateLand();
		//CreateTestLand();
		SetupOcean();
		RemoveLake();
		CreateLake();
		CellVector buf(Cell::GetCellCnt());
		SetupElevation(buf);
		SetupCoast(buf);
		SetupPeak(buf);
		SetupLandUnion();
		SetupIsland();
		SetupMoisture();
		SetupBiome();
		CreateRiver();
		SetupColor();
		SetupEdgePos();

	}
}

DistRadius VoronoiDiagramGenerator::GetMinDist(std::vector<PointDist>& points, Point2& c_p, double radius) {
	double min_dist = radius;
	double min_r = radius;
	for (PointDist& p : points) {
		double dist = CalcDistance(p.first, c_p);
		if (min_dist - min_r > dist - p.second) {
			min_dist = dist;
			min_r = p.second;
		}
	}

	return std::make_pair(min_dist, min_r);
}


void VoronoiDiagramGenerator::SetupOcean() {
	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;

			//if ((l_cell->GetDetail().GetTerrain() == Terrain::OCEAN || l_cell->GetDetail().GetTerrain() == Terrain::LAKE) &&
			//	(r_cell->GetDetail().GetTerrain() == Terrain::OCEAN || r_cell->GetDetail().GetTerrain() == Terrain::LAKE )) { // Set 'union' between connected seas
			if (IS_WATER(l_cell->GetDetail().GetTerrain()) && IS_WATER(r_cell->GetDetail().GetTerrain())) { // Set 'union' between connected seas
				if (l_cell->GetDetail().IsEdge()) {
					r_cell->GetDetail().SetUnionCell(Terrain::OCEAN, l_cell);
				}
				else {
					l_cell->GetDetail().SetUnionCell(Terrain::OCEAN, r_cell);
				}
			}
		}
	}

	//for (Cell* c : diagram->cells) {
	//	if (c->GetDetail().GetTerrain() == Terrain::OCEAN && c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetDetail().IsEdge()) {
	//		//c->GetDetail().SetElevation(OCEAN_ELEVATION);
	//	}
	//}
	has_created_ocean = true;
}

void VoronoiDiagramGenerator::CreateLand() {


	Point2 center = Point2((boundingBox.xR - boundingBox.xL) / 2, (boundingBox.yB - boundingBox.yT) / 2); // Center of the circular island.

	FastNoiseLite noise;
	noise.SetSeed(setting.seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	FastNoiseLite island_noise;
	island_noise.SetSeed(setting.seed);
	island_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	FastNoiseLite flat_noise;
	flat_noise.SetSeed(setting.seed);
	flat_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	flat_noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
	flat_noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
	flat_noise.SetDomainWarpAmp(100);

	/*

		double scale = 600;

		double x = round(c->site.p.x / scale), y = round(c->site.p.y / scale);

		noise.DomainWarp(x, y);
		double value = noise.GetNoise(x, y);
		*/

		//noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	double continent_range = setting.radius * 0.5;
	double island_range = setting.radius * 0.2;

	double island_step = setting.island_radius_max - setting.island_radius_min;



	std::vector<PointDist> islands;

	for (unsigned int i = 0; i < setting.island_cnt; i++) {
		Point2 p = Point2((GetRandom() - 0.5) * 2 * island_range, (GetRandom() - 0.5) * 2 * island_range);
		double length = CalcDistance(Point2(0, 0), p);
		Point2 dir = Point2((p.x / length) * continent_range + center.x, (p.y / length) * continent_range + center.y);
		islands.push_back(std::make_pair(Point2(p.x + dir.x, p.y + dir.y), (setting.island_radius_max - (GetRandom() * island_step))));
	}


	for (Cell* c : diagram->cells) {

		double scale = 3000;
		double island_scale = 1000;

		double dist = CalcDistance(c->site.p, center);
		DistRadius p_dist = GetMinDist(islands, c->site.p, setting.radius);
		//std::cout << p_dist << endl;

		double dist_scale = (1 - pow(dist / setting.radius, 1));
		double island_dist_scale = (1 - pow(p_dist.first / p_dist.second, 2)) * 3;

		double value = noise.GetNoise(round(c->site.p.x / scale), round(c->site.p.y / scale));
		double island_value = (1 + (island_noise.GetNoise(round(c->site.p.x / island_scale), round(c->site.p.y / island_scale))));

		bool IS_GROUND;
		switch (setting.type)
		{
		case MapType::CONTINENT:
			IS_GROUND = (pow(1 + value, 2) * dist_scale * 3 > 1);
			break;
		case MapType::ISLAND:
			IS_GROUND = (pow(value, 1) * dist_scale * 50) > 1 && (dist_scale > 0);
			break;
		default:
			IS_GROUND = false;
			break;
		}


		//std::cout << lake_value << "\n";
		if (IS_GROUND || (pow(island_value, 2) > 1 && island_dist_scale > 1)) { // Set Land
			//if (pow(lake_value, 2) > 0.4 - lakeScale) { // Set Land
			c->GetDetail().SetTerrain(Terrain::LAND);
			//c->GetDetail().SetElevation(0);

			double flat_scale = 600;
			double flat_x = round(c->site.p.x / flat_scale), flat_y = round(c->site.p.y / flat_scale);

			flat_noise.DomainWarp(flat_x, flat_y);
			double flat_value = flat_noise.GetNoise(flat_x, flat_y);
			if (flat_value > 0.7) {
				c->GetDetail().SetFlat(true);
			}

		}
		else {
			c->GetDetail().SetHighestPeak(false);
			c->GetDetail().SetElevation(OCEAN_ELEVATION);
			for (HalfEdge* he : c->halfEdges) {
				if (!he->edge->rSite) { // Check whether it is the outermost border.
					c->GetDetail().SetEdge(true);
					break;
				}
			}
		}

	}
}


void VoronoiDiagramGenerator::CreateTestLand() {

	FastNoiseLite noise;
	noise.SetSeed(setting.seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
	noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
	noise.SetDomainWarpAmp(100);

	//noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	for (Cell* c : diagram->cells) {

		double scale = 600;

		double x = round(c->site.p.x / scale), y = round(c->site.p.y / scale);

		noise.DomainWarp(x, y);
		double value = noise.GetNoise(x, y);

		//std::cout << lake_value << "\n";
		if (value > 0.7) { // Set Land
			//if (pow(lake_value, 2) > 0.4 - lakeScale) { // Set Land
			c->GetDetail().SetTerrain(Terrain::LAND);
		}
		else {
			c->GetDetail().SetHighestPeak(false);
			for (HalfEdge* he : c->halfEdges) {
				if (!he->edge->rSite) { // Check whether it is the outermost border.
					c->GetDetail().SetEdge(true);
					break;
				}
			}
		}
	}
}



void VoronoiDiagramGenerator::RemoveLake() {
	for (Cell* c : diagram->cells) { // remove lake
		CellDetail& cd = c->GetDetail();
		Terrain ct = cd.GetTerrain();
		if (ct == Terrain::OCEAN) {
			auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
			if (!cd.IsEdge()) {
				cd.SetTerrain(Terrain::LAND);
				c->GetDetail().SetElevation(0);
				cd.GetUnionFind().Reset(c);
			}
		}
	}
}

void VoronoiDiagramGenerator::CreateLake() {

	Point2 center = Point2((boundingBox.xR - boundingBox.xL) / 2, (boundingBox.yB - boundingBox.yT) / 2);

	double lakeScale = std::clamp<double>(setting.lake_scale, 0, 1);
	double lakeSize = std::clamp<double>(setting.lake_size, 0, 1);
	lakeSize = 2.3 - lakeSize * 2;
	lakeSize /= 100;
	lakeScale /= 2;

	FastNoiseLite lake_noise;
	lake_noise.SetSeed(setting.seed);
	lake_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	lake_noise.SetFrequency((float)lakeSize);
	lake_noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
	lake_noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
	lake_noise.SetDomainWarpAmp(200);
	/*
	lake_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	lake_noise.SetFractalGain(lakeSize);
	lake_noise.SetFractalOctaves(6);
	lake_noise.SetFractalLacunarity(2);*/
	//lake_noise.SetFrequency(0.2);

	std::vector<PointDist> lakes;
	double lake_range = setting.radius * 1;
	double lake_step = setting.lake_radius_max - setting.lake_radius_min;
	for (unsigned int i = 0; i < setting.lake_cnt; i++) {
		Point2 p = Point2((GetRandom() - 0.5) * 2 * lake_range + center.x, (GetRandom() - 0.5) * 2 * lake_range + center.y);
		lakes.push_back(std::make_pair(p, (setting.lake_radius_max - (GetRandom() * lake_step))));
	}

	std::vector<Cell*> lakeCells;
	for (Cell* c : diagram->cells) {

		double lake_scale = 500;
		DistRadius lake_dist = GetMinDist(lakes, c->site.p, setting.radius);
		double lake_dist_scale = (1 - (lake_dist.first / lake_dist.second));
		double x = (c->site.p.x / lake_scale), y = (c->site.p.y / lake_scale);
		lake_noise.DomainWarp(x, y);
		double lake_value = (1 + (lake_noise.GetNoise(x, y)));
		if ((pow(lake_value, 2) > 0.4 - lakeScale) && lake_dist_scale > 0) {
			lakeCells.push_back(c);
		}

	}



	for (Cell* c : lakeCells) {
		for (HalfEdge* he : c->halfEdges) {
			if (!he->edge->lSite || !he->edge->rSite || he->edge->lSite->cell->GetDetail().GetTerrain() == Terrain::OCEAN || he->edge->rSite->cell->GetDetail().GetTerrain() == Terrain::OCEAN) {
				goto LAKE_LOOP_POINT;
			}
		}
		c->GetDetail().SetTerrain(Terrain::LAKE);
	LAKE_LOOP_POINT:;
	}




	if (has_created_ocean) {
		for (Cell* c : diagram->cells) { // reset cell unionfind and other
			if (c->GetDetail().GetTerrain() == Terrain::OCEAN) c->GetDetail().Reset(false, false, false);
			else c->GetDetail().Reset(false, false);
			//c->GetDetail().Reset(false, false);
		}
		SetupOcean();
	}


	for (Cell* c : diagram->cells) { // Set Lake
		CellDetail& cd = c->GetDetail();
		Terrain ct = cd.GetTerrain();
		//if (ct == Terrain::OCEAN || ct == Terrain::LAKE) {
		if (IS_WATER(ct)) {

			auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
			if (!cd.IsEdge()) {
				//std::cout << "isEdge(): " << cd.IsEdge() << "\n";
				//cd.SetTerrain(Terrain::LAKE);
			}
			else {
				auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
				diagram->oceanUnion.insert(unique)->push_back(c);
			}

		}/*
		else if (ct == Terrain::COAST) {
			auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
			diagram->oceanUnion.insert(unique)->push_back(c);
		}*/
	}
}

void VoronoiDiagramGenerator::SetupElevation(CellVector& coastBuffer) {

	//std::queue<Cell*> landQueue; // BFS
	//std::queue<Cell*> lakeQueue;
	
	CellQueue landQueue(Cell::GetCellCnt()); // BFS
	CellQueue lakeQueue(Cell::GetCellCnt());

	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;
			//auto& l_detail = l_cell->GetDetail();
			//auto& r_detail = r_cell->GetDetail();

			Cell* landCell, * oceanCell;
			(l_cell->GetDetail().GetTerrain() == Terrain::OCEAN || l_cell->GetDetail().GetTerrain() == Terrain::LAKE) ? (landCell = r_cell, oceanCell = l_cell) : (landCell = l_cell, oceanCell = r_cell);

			if (landCell->GetDetail().GetTerrain() == Terrain::LAND && oceanCell->GetDetail().GetTerrain() == Terrain::OCEAN) {
				CellDetail& cd = oceanCell->GetDetail();
				if (cd.IsEdge()) { // check if it's lake or not
					coastBuffer.push_back(oceanCell);
					landQueue.push(oceanCell);
				}

			}

			else if (landCell->GetDetail().GetTerrain() == Terrain::LAND && oceanCell->GetDetail().GetTerrain() == Terrain::LAKE) {
				//landCell->GetDetail().GetColor() = Color(1, 0, 0);
				lakeQueue.push(oceanCell);
			}
		}
	}


	while (!landQueue.empty() || !lakeQueue.empty()) {
		//Cell* c = landQueue.front();

		bool is_lq = landQueue.empty();
		Cell* c;
		if (is_lq) {
			c = lakeQueue.GetValue();
			lakeQueue.pop();
		}
		else {
			c = landQueue.GetValue();
			landQueue.pop();
		}

		CellDetail& cd = c->GetDetail();
		unsigned int land_cnt = 0, low_cnt = 0;
		int cur_elev = IS_LAND(cd.GetTerrain()) ? cd.GetElevation() : 0;

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			//if (targetCell->GetDetail().GetElevation() < c->GetDetail().GetElevation()) {
			CellDetail& tcd = targetCell->GetDetail();
			if (cd.GetTerrain() == Terrain::LAKE && tcd.GetTerrain() == Terrain::LAND) {
				if (tcd.GetElevation() == 0) {
					tcd.SetElevation(cd.UnionFindCell(Terrain::OCEAN)->GetDetail().GetElevation());
					lakeQueue.push(targetCell);
					continue;
				}

			}
			else if (tcd.GetTerrain() == Terrain::LAND) {
				land_cnt++;
				//if (tcd.GetElevation() == 0) {
				if (cur_elev + 1 < tcd.GetElevation()) {

					tcd.SetElevation(0);
				}
				if (tcd.GetElevation() == 0) {
					if (tcd.IsFlat() && cd.GetElevation() > LAND_MIN_ELEVATION) {
						tcd.SetElevation(cur_elev);
					}
					else {
						tcd.SetElevation(cur_elev + 1);
						//cd.SetHighestPeak(false);
						//cd.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().SetHighestPeak(false);
					}


					if (is_lq) {
						lakeQueue.push(targetCell);
					}
					else {
						landQueue.push(targetCell);
					}


				}
			}
			else if (tcd.GetTerrain() == Terrain::LAKE) {
				//low_cnt++;
				if (!is_lq) {
					auto t_union = tcd.UnionFindCell(Terrain::OCEAN);
					if (t_union->GetDetail().GetElevation() == 0 || t_union->GetDetail().GetElevation() > cd.GetElevation()) {
						t_union->GetDetail().SetElevation(cd.GetElevation());
					}
				}
			}
		}
	}
}


void VoronoiDiagramGenerator::SetupPeak(CellVector& coastBuffer) {

	//std::queue<Cell*> landQueue; // BFS
	//std::queue<Cell*> lakeQueue;
	std::queue<int> test;

	CellQueue landQueue(Cell::GetCellCnt(), false); // BFS
	CellQueue UnionQueue(Cell::GetCellCnt(), false); // BFS
	for (Cell* c : coastBuffer.GetBuffer()) {
		landQueue.push(c);
		UnionQueue.push(c);
	}

	int peak_sens = -2;

	while (!UnionQueue.empty()) {
		//Cell* c = landQueue.front();

		Cell* c = UnionQueue.GetValue();
		UnionQueue.pop();

		CellDetail& cd = c->GetDetail();
		int under_cell_cnt = 0;
		max_elevation = std::max<int>(max_elevation, cd.GetElevation());

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			//if (targetCell->GetDetail().GetElevation() < c->GetDetail().GetElevation()) {
			CellDetail& tcd = targetCell->GetDetail();

			if (cd.GetTerrain() == Terrain::LAND) {
				if (tcd.GetTerrain() == Terrain::LAND) {
					if (tcd.GetElevation() < cd.GetElevation()) {
						under_cell_cnt++;
					}
					else if (tcd.GetElevation() == cd.GetElevation()) {
						under_cell_cnt++;
						tcd.SetUnionCell(Terrain::PEAK, c);
						tcd.SetUnionCell(Terrain::HIGHEST_PEAK, c);
					}

				}
				else {
					under_cell_cnt++;
				}
			}

			if (tcd.GetTerrain() == Terrain::LAND) {
				UnionQueue.push(targetCell);
			}

		}

		if (under_cell_cnt == c->halfEdges.size()) {
			cd.SetHighestPeak(true);
		}
		if (under_cell_cnt >= c->halfEdges.size() + peak_sens) {
			cd.SetPeak(true);
		}
	}

	while (!landQueue.empty()) {
		//Cell* c = landQueue.front();

		Cell* c = landQueue.GetValue();
		landQueue.pop();

		CellDetail& cd = c->GetDetail();
		unsigned int land_cnt = 0, low_cnt = 0;
		size_t cell_cnt = c->halfEdges.size();
		int under_cell_cnt = 0;

		for (HalfEdge* he : c->halfEdges) {

			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			//if (targetCell->GetDetail().GetElevation() < c->GetDetail().GetElevation()) {
			CellDetail& tcd = targetCell->GetDetail();


			if (cd.GetTerrain() == Terrain::LAND) {
				if (tcd.GetTerrain() == Terrain::LAND) {
					if (tcd.GetElevation() < cd.GetElevation()) {
						under_cell_cnt++;
					}
					else if (tcd.GetElevation() == cd.GetElevation()) {
						under_cell_cnt++;
					}
				}
				else {
					under_cell_cnt++;
				}
			}

			if (tcd.GetTerrain() == Terrain::LAND) {
				landQueue.push(targetCell);
			}

		}

		if (cell_cnt != under_cell_cnt) {
			//cd.SetPeak(false);
			cd.UnionFindCellDetail(Terrain::HIGHEST_PEAK).SetHighestPeak(false);
			//cd.SetPeak(true);
			//cd.UnionFindCell(Terrain::PEAK)->GetDetail().SetPeak(true);
		}
		else {
			//cd.SetPeak(false);
			//cd.UnionFindCell(Terrain::PEAK)->GetDetail().SetPeak(false);
			//cd.UnionFindCell(Terrain::PEAK)->GetDetail().SetPeak(false);
			//cd.SetPeak(false);
		}

		if (under_cell_cnt < c->halfEdges.size() + peak_sens) {
			//cd.SetHighestPeak(true);
			cd.UnionFindCellDetail(Terrain::PEAK).SetPeak(false);
		}

	}
}


void VoronoiDiagramGenerator::SetupCoast(CellVector& coastBuffer) {
	for (Cell* c : coastBuffer.GetBuffer()) {
		CellDetail& cd = c->GetDetail();
		cd.SetTerrain(Terrain::COAST);
		cd.SetElevation(COAST_ELEVATION);
	}
}

void VoronoiDiagramGenerator::SetupLandUnion() {
	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;
			CellDetail& ld = l_cell->GetDetail();
			CellDetail& rd = r_cell->GetDetail();
			Terrain l_t = ld.GetTerrain();
			Terrain r_t = rd.GetTerrain();
			//if ((l_t == Terrain::LAND || l_t == Terrain::LAKE) && (r_t == Terrain::LAND || r_t == Terrain::LAKE)) { // land ¶¥ °øÀ¯
			if (IS_LAND(l_t) && IS_LAND(r_t)) { // land ¶¥ °øÀ¯
				ld.SetUnionCell(Terrain::LAND, r_cell);
			}

			if (ld.IsFlat() && rd.IsFlat()) {
				ld.SetUnionCell(Terrain::FLAT, r_cell);
			}
		}
	}
}

void VoronoiDiagramGenerator::SetupIsland() {

	Cell* lastPeak = nullptr;
	for (Cell* c : diagram->cells) {
		CellDetail& cd = c->GetDetail();

		Terrain ct = cd.GetTerrain();
		if (ct == Terrain::LAND) {
			auto arr = diagram->islandUnion.insert(cd.UnionFindCell(Terrain::LAND)->GetUnique());
			arr->land.push_back(c);

			bool is_highest_peak = cd.UnionFindCellDetail(Terrain::HIGHEST_PEAK).GetHighestPeak();
			if (is_highest_peak) {
				auto unique = cd.UnionFindCell(Terrain::HIGHEST_PEAK)->GetUnique();
				cd.SetTerrain(Terrain::HIGHEST_PEAK);
				arr->highestPeakUnion.insert(unique)->push_back(c);
				/*if (c->GetUnique() > unique) {
					std::cout << "cell: " << c->GetUnique() << "\n";
					std::cout << "uni: " << unique << "\n";
				}*/
				//cd.GetColor() = Color(0.3, 0.3, 0.3);
				//cd.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().GetColor() = Color(0, 0, 0);
			}

			if (cd.IsPeak() || is_highest_peak) {
				if (lastPeak == nullptr) lastPeak = c;
				auto unique = c->GetDetail().UnionFindCell(Terrain::PEAK)->GetUnique();
				arr->peakUnion.insert(unique)->push_back(c);
			}
		}
		else if (ct == Terrain::LAKE) {
			auto arr = diagram->islandUnion.insert(cd.UnionFindCell(Terrain::LAND)->GetUnique());
			arr->land.push_back(c);
			auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
			arr->lakeUnion.insert(unique)->push_back(c);
			cd.SetElevation(cd.UnionFindCellDetail(Terrain::LAND).GetElevation());
		}
		/*if (cd.b_peak) {
			c->GetDetail().GetColor() = Color(0, 1, 0);
		}*/
	}
}


void VoronoiDiagramGenerator::SetupMoisture() {
	CellPriorityQueue p_q(Cell::GetCellCnt(), false);
	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		for (auto highestPeak_union : island.highestPeakUnion.unions) {

			for (auto highestPeak : highestPeak_union.second) {
				p_q.push(highestPeak);
			}
		}
	}
	auto u_f = [](std::pair<Cell*, int> c) { return c.first->GetUnique(); };
	auto v_f = [](auto buffer) { return buffer->front(); };

	UniBuf<std::queue, std::pair<Cell*, int>, decltype(u_f), decltype(v_f), std::pair<Cell*, int>> lake_q(u_f, v_f, Cell::GetCellCnt(), false);
	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		for (auto highestPeak_union : island.lakeUnion.unions) {
			int power = highestPeak_union.second.size() * 2;
			for (auto highestPeak : highestPeak_union.second) {
				lake_q.push(std::make_pair(highestPeak, power));
			}
		}
	}




	while (!p_q.empty()) {
		Cell* c = p_q.GetValue();
		p_q.pop();

		CellDetail& cd = c->GetDetail();
		//std::cout << " Elevation: " << cd.GetElevation() << ", Moisture: " << cd.GetMoisture() << ", AreaMoisture: " << cd.GetAreaMoisture() << ", LocalMoisture: " << cd.GetLocalMoisture() << "\n";
		max_moisture = std::max<unsigned int>(max_moisture, cd.GetMoisture());
		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			//if (targetCell->GetDetail().GetElevation() < c->GetDetail().GetElevation()) {
			CellDetail& tcd = targetCell->GetDetail();
			if (IS_LAND(tcd.GetTerrain()) && cd.GetElevation() >= tcd.GetElevation()) {
				//if (p_q.push(targetCell)) {
					
				//}

				bool is_first = p_q.push(targetCell);

				if (cd.GetElevation() > tcd.GetElevation()) {
					
					if (tcd.IsFlat()) {
						tcd.UnionFindCellDetail(Terrain::FLAT).AddAreaMoisture(1);
					}
					
					if (is_first) {
						tcd.SetLocalMoisture(max_elevation - tcd.GetElevation());
					}
					else {
						tcd.AddLocalMoisture(1);
					}
					
				}
				else {
					if(is_first) tcd.SetLocalMoisture(cd.GetLocalMoisture() + 1);
				}
			}

		}
	}

	while (!lake_q.empty()) {
		auto value = lake_q.GetValue();
		lake_q.pop();
		Cell* c = value.first;
		CellDetail& cd = c->GetDetail();
		int pow = value.second;

		if (pow <= 0) continue;

		//std::cout << pow << "\n";
		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			CellDetail& tcd = targetCell->GetDetail();

			if (IS_GROUND(tcd.GetTerrain()) && cd.GetElevation() >= tcd.GetElevation() ) {
				if (lake_q.push(std::make_pair(targetCell, pow - 1))) {
					tcd.AddLocalMoisture(pow - 1);
				}
			}

		}

	}
}

void VoronoiDiagramGenerator::SetupBiome() {
}


void VoronoiDiagramGenerator::CreateRiver() {
	//std::priority_queue<Cell*, std::vector<Cell*>, compare> p_q;
	
	///*for (auto item : diagram->islandUnion.unions) {
	//	auto island = item.second;
	//	for (auto lake_union : island.lakeUnion.unions) {

	//		for (auto lake : lake_union.second) {
	//			Point2& p = lake->site.p;
	//			p_q.push(lake);
	//		}
	//	}
	//}*/

}


void VoronoiDiagramGenerator::SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, Color& elev_rate_c) {
	
	CellDetail& cd = c->GetDetail();
	CellDetail& tcd = opposite_c->GetDetail();
	if (IS_LAND(cd.GetTerrain())) {
		if (cd.GetTerrain() == Terrain::LAKE) {
			if (tcd.GetTerrain() == Terrain::LAKE) {
				v->color = cd.GetColor();
				v->elev = cd.GetElevation();
			}
			return;
		}

		if ((tcd.GetTerrain() == Terrain::OCEAN || tcd.GetTerrain() == Terrain::COAST)) {
			if (v->elev == 0) {
				v->color = cd.GetColor();
			}
		}
		else {
			if (v->elev < cd.GetElevation()) {
				v->elev = cd.GetElevation();
				v->cells.clear();
				v->color = cd.GetColor() - elev_rate_c;
			}
			else if (v->elev == cd.GetElevation()) {
				if (!v->Find(opposite_c)) {
					v->cells.push_back(opposite_c);
				}
			}

			if (v->cells.size() >= 3) {
				v->color = cd.GetColor();
			}

		}
	}
	else {
		if (IS_LAND(tcd.GetTerrain())) {
			v->color = tcd.GetColor();
			v->elev = tcd.GetElevation();
		}
		else {

			if (v->elev <= 0) {

				if (v->elev >= cd.GetElevation()) {

					v->color = cd.GetColor();
					v->elev = cd.GetElevation();
				}
				else {
					//std::cout << cd.GetElevation() << "\n";
					//v->color = Color(1, 0, 0);
				}
				//v->color = Color(1, 0, 0);
			}
		}
	}


}

void VoronoiDiagramGenerator::SetupColor(int flag) {
	if (has_set_color) {
		for (Vertex* v : diagram->vertices) {
			v->Reset();
		}
		for (Edge* e : diagram->edges) {
			e->color = Color();
		}
	}
	has_set_color = true;

	double elev_rate = 1.0 / (double)max_elevation;
	std::cout << "elev_rate: " << elev_rate << "\n";

	Color elev_rate_c = Color(elev_rate / 2, elev_rate / 2, elev_rate / 2);
	for (Cell* c : diagram->cells) {

		CellDetail& cd = c->GetDetail();
		/*if (IS_GROUND(cd.GetTerrain())) {
			std::cout << "land elev: " << cd.GetElevation() << "\n";
		}*/
		//if ((cd.GetTerrain() == Terrain::LAND || cd.GetTerrain() == Terrain::HIGHEST_PEAK) && cd.GetElevation() != 0) {
		
		if (IS_GROUND(cd.GetTerrain()) /*&& cd.GetElevation() != 0*/) {
			if (flag & ISLAND) {
				
				if (cd.IsHighestPeak() && false) {
					cd.GetColor() = Color(0.3, 0.3, 0.3);
					cd.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().GetColor() = Color(0, 0, 0);
				}
				else {
					
					double elev_scale = (double)(cd.GetElevation()) * elev_rate;
					double scale = 1;
					double color = elev_scale * scale;
					Color island_elev = Color(color, color, color);
					cd.GetColor() = island_elev;
				}
			}
			else {
				cd.GetColor() = Color::black;
			}
			
		}
		else if (cd.GetTerrain() == Terrain::LAKE) {
			//double elev_scale = (double)cd.UnionFindCell(Terrain::OCEAN)->GetDetail().GetElevation() / (max_elevation * 2);
			//elev_scale = pow(elev_scale, 1);
			//double scale = 1.2;
		/*	cd.GetColor().r += elev_scale * scale;
			cd.GetColor().g += elev_scale * scale;
			cd.GetColor().b += elev_scale * scale;*/
			if (flag & LAKE) {
				cd.GetColor() = Color::lake;
			}
			else {
				if (flag & ISLAND) {
					auto t_union = cd.UnionFindCell(Terrain::OCEAN);
					
					double elev_scale = (double)(t_union->GetDetail().GetElevation()) * elev_rate;
					double scale = 1;
					double color = elev_scale * scale;
					Color island_elev = Color(color, color, color);

					cd.GetColor() = island_elev;
				}
				else {
					cd.GetColor() = Color::black;
				}
			}
			
			//cd.GetColor().b += elev_scale * 0.7 * scale;
		}
		else if (cd.GetTerrain() == Terrain::COAST) {
			if (flag & COAST) {
				cd.GetColor() = Color::coast;
			}
			else {
				if (flag & OCEAN) {
					if (cd.IsEdge()) {
						cd.GetColor() = Color::edgeOcean;
					}
					else {
						cd.GetColor() = Color::ocean;
					}
					
				}
				else {
					cd.GetColor() = Color::black;
				}
			}
		}
		else if (cd.GetTerrain() == Terrain::OCEAN) {
			if (flag & OCEAN) {
				if (cd.GetEdge()) {
					cd.GetColor() = Color::edgeOcean;
				}
				else {
					cd.GetColor() = Color::ocean;
				}
			}
			else {
				cd.GetColor() = Color::black;
			}
		}
	}


	for (Cell* c : diagram->cells) {
		CellDetail& cd = c->GetDetail();


		for (auto he : c->halfEdges) {


			Edge* e = he->edge;

			Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;
			Cell* opposite_c = opposite_s->cell;

			CellDetail& cd = c->GetDetail();
			CellDetail& tcd = opposite_s->cell->GetDetail();

			Vertex* vA = e->vertA;
			Vertex* vB = e->vertB;


		

			SetupVertexColor(vA, c, opposite_c, elev_rate_c);
			SetupVertexColor(vB, c, opposite_c, elev_rate_c);

		}
	}


	for (Cell* c : diagram->cells) {
		CellDetail& cd = c->GetDetail();

		double avg = 0;

		for (HalfEdge* he : c->halfEdges) {
			avg += he->edge->vertA->point.distanceTo(he->edge->vertB->point);
		}
		avg /= c->halfEdges.size();

		for (auto he : c->halfEdges) {


			Edge* e = he->edge;

			Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;
			Cell* opposite_c = opposite_s->cell;

			CellDetail& cd = c->GetDetail();
			CellDetail& tcd = opposite_s->cell->GetDetail();

			Vertex* vA = e->vertA;
			Vertex* vB = e->vertB;
			//if ((tcd.GetTerrain() == Terrain::OCEAN || tcd.GetTerrain() == Terrain::COAST)) {

			double dist = he->edge->vertA->point.distanceTo(he->edge->vertB->point);
			if (IS_LAND(cd.GetTerrain())) {



				if (IS_OCEAN(tcd.GetTerrain())) {
					e->color = cd.GetColor();
				}
				else {
					if (avg / 2 > dist) {
						//auto elev = vA->elev < vB->elev ? vA->elev : vB->elev;
						vA->color = vA->elev < vB->elev ? vA->color : vB->color;
						vB->color = vA->color;
						e->color = vA->color;
						//vA->elev = elev;
						//vB->elev = elev;

					}
					else {
						if (cd.GetTerrain() == Terrain::LAKE && tcd.GetTerrain() == Terrain::LAKE) {
							e->color = cd.GetColor();
						}
						else {
							if (cd.GetElevation() == tcd.GetElevation()) {
								if (vA->elev == vB->elev) {
									e->color = cd.GetColor();
								}
								else {
									e->color = Color::MixColor(vA->color, vB->color);

								}
								//e->color = Color(1, 0, 0);
							}
							//else if(e->elev < cd.GetElevation()){
							else {
								//e->elev = cd.GetElevation();
								e->color = Color::MixColor(vA->color, vB->color);
								//e->color = Color(1, 0, 0);
							}
						}
					}
				}
			}
			else {
				//if(cd.GetElevation() <= COAST_ELEVATION) std::cout << cd.GetElevation() << "\n";

				if (IS_OCEAN(tcd.GetTerrain())) {
					if (avg / 5 > dist) {
						if (tcd.GetElevation() <= COAST_ELEVATION) {

							if (vA->elev > COAST_ELEVATION && vB->elev > COAST_ELEVATION) { // µÑ ´Ù ¶¥
								vA->color = vA->elev > vB->elev ? vA->color : vB->color;
								vB->color = vA->color;
								e->color = vA->color;
							}
							else {
								if (cd.GetElevation() == tcd.GetElevation()) {
									Vertex* tv = vA->elev > vB->elev ? vA : vB;
									Vertex* op_v = vA->elev < vB->elev ? vA : vB;
									tv->color = cd.GetColor();
									op_v->color = cd.GetColor();
									e->color = cd.GetColor();
									tv->elev = cd.GetElevation();
									//e->color = Color::MixColor(vA->color, vB->color);
									//e->color = vA->elev > vB->elev ? vA->color : vB->color;
								}
								else {
									e->color = Color::MixColor(vA->color, vB->color);
								}
							}
							//e->color = Color(1, 0, 0);
						}
						else
						{
							e->color = Color::MixColor(vA->color, vB->color);
						}
						//e->color = Color(1, 0, 0);
					}
					else {
						if (cd.GetElevation() > tcd.GetElevation()) {
							//e->color = cd.GetColor();
							e->color = Color::MixColor(vA->color, vB->color);
						}
						else if (cd.GetElevation() == tcd.GetElevation()) {
							e->color = cd.GetColor();
						}
						else {
							e->color = Color::MixColor(vA->color, vB->color);
						}

					}
				}
				//

			}
		}
	}
	for (Cell* c : diagram->cells) {
		CellDetail& cd = c->GetDetail();
		if (IS_LAND(cd.GetTerrain())) {
			for (auto he : c->halfEdges) {

				Edge* e = he->edge;

				Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
				Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;
				Cell* opposite_c = opposite_s->cell;

				CellDetail& cd = c->GetDetail();
				CellDetail& tcd = opposite_s->cell->GetDetail();

				Vertex* vA = e->vertA;
				Vertex* vB = e->vertB;


				if ((vA->elev > COAST_ELEVATION && vB->elev <= COAST_ELEVATION) ||
					(vB->elev > COAST_ELEVATION && vA->elev <= COAST_ELEVATION)) {
					e->color = Color::MixColor(vA->color, vB->color);
				}

			}
		}
	}
}


void VoronoiDiagramGenerator::SetupEdgePos() {
	for (Edge* e : diagram->edges) { 
		if (e->rSite) {
			double dist = e->vertA->point.distanceTo(e->vertB->point);

			//Point2 norm = (e->vertA->point - e->vertB->point).Normailize();
			Point2 norm = (e->vertA->point - e->vertB->point) / dist;
			norm = Point2(-norm.y, norm.x);
			double step = 3;
			double perp_step = 3;
			double scale1 = (0.5 - ((1 / step) / 2)) + GetRandom() / step;
			double perp_len = (dist / perp_step);
			double scale2 = (perp_len / 2) - GetRandom() * perp_len;

			e->p = e->vertA->point * scale1 + e->vertB->point * (1 - scale1) + norm * scale2;
		}
		else {
			e->p = (e->vertA->point + e->vertB->point) / 2;
		}
		
		//e->p = (e->vertA->point + e->vertB->point) / 2;
	}
}

void VoronoiDiagramGenerator::SaveAllImage(double dimension, double w, double h) {
	SetupColor(ALL_IMAGE);
	SaveImage("voronoi_map_all.bmp", dimension, w, h);

	SetupColor(ISLAND);
	SaveImage("voronoi_map_islnad.bmp", dimension, w, h);

	SetupColor(LAKE);
	SaveImage("voronoi_map_lake.bmp", dimension, w, h);

	SetupColor(ALL_IMAGE);
}

void VoronoiDiagramGenerator::SaveImage(const char* filename, double dimension, double w, double h) {
	char* pixel_data = new char[w * h * 3];

	for (Cell* c : diagram->cells) {
		//for (Edge* e : diagram->edges) {
		for (HalfEdge* hf : c->halfEdges) {
			Edge* e = hf->edge;

			Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;

			Point2 center = c->site.p / dimension;
			center.x *= w;
			center.y *= h;

			Point2 pA = e->vertA->point / dimension;
			pA.x *= w;
			pA.y *= h;

			Point2 pB = e->vertB->point / dimension;
			pB.x *= w;
			pB.y *= h;

			Point2 edge_mp = e->p / dimension;
			edge_mp.x *= w;
			edge_mp.y *= h;

			Color colorA = e->vertA->color;
			Color colorB = e->vertB->color;
			Color edge_c = e->color;

			Color center_c = s->cell->GetDetail().GetColor();

			//middle_c = (colorA + colorB) / 2;

			Triangle triA = Triangle({ &pA, &center, &edge_mp, &colorA, &center_c, &edge_c });
			Triangle triB = Triangle({ &pB, &center, &edge_mp, &colorB, &center_c, &edge_c });

			triA.draw(pixel_data, w, h);
			triB.draw(pixel_data, w, h);
		}
	}

	//char pixel_data[IMAGE_WIDTH * IMAGE_HEIGHT * 300];
	//glReadPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);


	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	FILE* out = nullptr;
	char buff[256];
	//const char* filename = "voronoi_map.bmp";
	fopen_s(&out, filename, "wb");
	char* data = pixel_data;
	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));
	bf.bfType = 'MB';
	bf.bfSize = sizeof(bf) + sizeof(bi) + w * h * 3;
	bf.bfOffBits = sizeof(bf) + sizeof(bi);
	bi.biSize = sizeof(bi);
	bi.biWidth = w;
	bi.biHeight = h;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biSizeImage = w * h * 3;
	fwrite(&bf, sizeof(bf), 1, out);
	fwrite(&bi, sizeof(bi), 1, out);
	fwrite(data, sizeof(unsigned char), w * h * 3, out);
	fclose(out);
	delete[] pixel_data;

	std::cout << "file saved: " << filename << "\n";

}