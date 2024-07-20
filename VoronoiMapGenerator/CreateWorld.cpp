#include "VoronoiDiagramGenerator.h"
#include "Vector2.h"
#include "FastNoise/FastNoiseLite.h"
#include "Epsilon.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <tuple>
#include <unordered_map>
#include "Data/Buffer.h"
#include "Data/Triangle.h"
#include <Windows.h>

using std::cout;
using std::cin;
using std::endl;

using DistRadius = std::pair<double, double>; // fisrt: distance, second: radius
using PointDist = std::pair<Point2, double>; // first: point, second: radius


void VoronoiDiagramGenerator::CreateWorld(bool trans_edge, bool create_tri) {
	if (diagram) {
		setting.Srand();
		CreateLand();
		//CreateTestLand();
		SetupOcean();
		RemoveLake();
		CreateLake();
		CellVector buf(diagram->GetCellUnique());
		SetupElevation(buf);
		SetupCoast(buf);
		SetupPeak(buf);
		SetupLandUnion();
		SetupIsland();
		SetupMoisture();
		SetupBiome();
		CreateRiver();
		SetupEdgePos(trans_edge);
		SetupColor();
		if (create_tri) {
			CreateTriangle();
		}
	}
}

DistRadius VoronoiDiagramGenerator::GetMinDist(std::vector<PointDist>& points, Point2& c_p, double radius) {
	double min_dist = radius;
	double min_r = radius;
	for (PointDist& p : points) {
		double dist = p.first.DistanceTo(c_p);
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
	noise.SetSeed(setting.GetSeed());
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	FastNoiseLite island_noise;
	island_noise.SetSeed(setting.GetSeed());
	island_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	FastNoiseLite flat_noise;
	flat_noise.SetSeed(setting.GetSeed());
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

	double continent_range = setting.GetRadius() * 0.5;
	double island_range = setting.GetRadius() * 0.2;

	double island_step = setting.GetIslandRadiusMax() - setting.GetIslandRadiusMin();



	std::vector<PointDist> islands;

	for (unsigned int i = 0; i < setting.GetIslandCount(); i++) {
		Point2 p = Point2((setting.GetRandomRound() - 0.5) * 2 * island_range, (setting.GetRandomRound() - 0.5) * 2 * island_range);
		double length = Point2::Distance(Point2(0, 0), p);
		Point2 dir = Point2((p.x / length) * continent_range + center.x, (p.y / length) * continent_range + center.y);
		islands.push_back(std::make_pair(Point2(p.x + dir.x, p.y + dir.y), (setting.GetIslandRadiusMax() - (setting.GetRandom() * island_step))));
	}


	for (Cell* c : diagram->cells) {

		double scale = 3000;
		double island_scale = 1000;

		double dist = Point2::Distance(c->site.p, center);
		DistRadius p_dist = GetMinDist(islands, c->site.p, setting.GetRadius());

		double dist_scale = (1 - pow(dist / setting.GetRadius(), 1));
		double island_dist_scale = (1 - pow(p_dist.first / p_dist.second, 2)) * 3;

		double value = noise.GetNoise(round(c->site.p.x / scale), round(c->site.p.y / scale));
		double island_value = (1 + (island_noise.GetNoise(round(c->site.p.x / island_scale), round(c->site.p.y / island_scale))));

		bool IS_GROUND;
		switch (setting.GetMapType())
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

		// Set Land
		if (IS_GROUND || (pow(island_value, 2) > 1 && island_dist_scale > 1)) { 
			c->GetDetail().SetTerrain(Terrain::LAND);

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
	noise.SetSeed(setting.GetSeed());
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

	double lakeScale = std::clamp<double>(setting.GetLakeScale(), 0, 1);
	double lakeSize = std::clamp<double>(setting.GetLakeSize(), 0, 1);
	lakeSize = 2.3 - lakeSize * 2;
	lakeSize /= 100;
	lakeScale /= 2;

	lakeSize = round(lakeSize * 100) / 100;
	lakeSize = round(lakeScale * 100) / 100;

	FastNoiseLite lake_noise;
	lake_noise.SetSeed(setting.GetSeed());
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
	double lake_range = setting.GetRadius() * 1;
	double lake_step = setting.GetLakeRadiusMax() - setting.GetLakeRadiusMin();
	for (unsigned int i = 0; i < setting.GetLakeCount(); i++) {
		Point2 p = Point2((setting.GetRandom() - 0.5) * 2 * lake_range + center.x, (setting.GetRandom() - 0.5) * 2 * lake_range + center.y);
		lakes.push_back(std::make_pair(p, (setting.GetLakeRadiusMax() - (setting.GetRandom() * lake_step))));
	}

	std::vector<Cell*> lakeCells;
	for (Cell* c : diagram->cells) {

		double lake_scale = 500;
		DistRadius lake_dist = GetMinDist(lakes, c->site.p, setting.GetRadius());
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

			//auto unique = cd.UnionFindCell(Terrain::OCEAN)->GetUnique();
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

	CellQueue landQueue(diagram->GetCellUnique()); // BFS
	CellQueue lakeQueue(diagram->GetCellUnique());

	CellQueue lakeBuf(diagram->GetCellUnique(), false);

	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;

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
				lakeQueue.push(oceanCell);
			}
		}
	}


	while (!landQueue.empty() || !lakeQueue.empty()) {

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
			else if (IS_GROUND(cd.GetTerrain()) && tcd.GetTerrain() == Terrain::LAKE) {
				lakeBuf.push(targetCell);
			}
		}
	}


	while (!lakeBuf.empty()) {
		Cell* c = lakeBuf.GetValue();
		lakeBuf.pop();
		CellDetail& cd = c->GetDetail();

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			CellDetail& tcd = targetCell->GetDetail();
			if (IS_GROUND(tcd.GetTerrain())) {
				CellDetail& ud = cd.UnionFindCellDetail(Terrain::OCEAN);
				if (ud.GetElevation() == 0 || ud.GetElevation() > tcd.GetElevation()) {
					ud.SetElevation(tcd.GetElevation());
					cd.SetElevation(tcd.GetElevation());
				}
			}

		}
	}
}


void VoronoiDiagramGenerator::SetupPeak(CellVector& coastBuffer) {

	CellQueue landQueue(diagram->GetCellUnique(), false); 
	CellQueue UnionQueue(diagram->GetCellUnique(), false);
	for (Cell* c : coastBuffer.GetBuffer()) {
		landQueue.push(c);
		UnionQueue.push(c);
	}

	while (!UnionQueue.empty()) {
		Cell* c = UnionQueue.GetValue();
		UnionQueue.pop();

		CellDetail& cd = c->GetDetail();
		int under_cell_cnt = 0;
		max_elevation = std::max<int>(max_elevation, cd.GetElevation());
		double avg_elev = cd.GetElevation();

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			CellDetail& tcd = targetCell->GetDetail();
			if (cd.IsFlat()) avg_elev += 999999999;

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

					avg_elev += tcd.GetElevation();
				}
				else {
					under_cell_cnt++;
					avg_elev += 999999999;
				}
			}
			else {
				avg_elev += 999999999;
			}

			if (tcd.GetTerrain() == Terrain::LAND) {
				UnionQueue.push(targetCell);
			}

		}

		if (IS_LAND(cd.GetTerrain())) {
			avg_elev /= (c->halfEdges.size() + 1);
			if (avg_elev < cd.GetElevation()) {
				cd.SetPeak(true);
			}
			if (!cd.IsFlat() && under_cell_cnt > c->halfEdges.size() - 2) {
				cd.SetPeak(true);
				//p_q.push(std::make_pair(c, cd.GetElevation()));
			}
		}
		if (under_cell_cnt == c->halfEdges.size()) {
			cd.SetHighestPeak(true);
			//p_q.push(std::make_pair(c, cd.GetElevation()));
		}


	}

	while (!landQueue.empty()) {
		Cell* c = landQueue.GetValue();
		landQueue.pop();

		CellDetail& cd = c->GetDetail();
		unsigned int land_cnt = 0, low_cnt = 0;
		size_t cell_cnt = c->halfEdges.size();
		int under_cell_cnt = 0;
		double avg_elev = cd.GetElevation();
		for (HalfEdge* he : c->halfEdges) {

			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			CellDetail& tcd = targetCell->GetDetail();
			avg_elev += tcd.GetElevation();

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
			cd.UnionFindCellDetail(Terrain::HIGHEST_PEAK).SetHighestPeak(false);
		}
		else {
			//
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
			}

			if (cd.GetPeak() || is_highest_peak) {
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
			cd.SetElevation(cd.UnionFindCellDetail(Terrain::OCEAN).GetElevation());
			
		}
	}
}


void VoronoiDiagramGenerator::SetupMoisture() {

	using Container = std::pair<Cell*, int>;
	auto u_f = [](ReturnType<Container> c) { return c.first->GetUnique(); };
	auto v_f = [](auto buffer) -> decltype(auto) { return buffer->front(); };
	auto p_u_f = [](ReturnType<Cell*> c) { return c->GetUnique(); };
	auto p_v_f = [](auto buffer) -> decltype(auto) { return buffer->top(); };


	for (auto item : diagram->islandUnion.unions) {
		//CellPriorityQueue p_q(diagram->GetCellUnique(), false);
		UniBuf<std::priority_queue, Cell*, decltype(p_u_f), decltype(p_v_f), Cell*, std::vector<Cell*>, River::RiverPriorityComp> p_q(p_u_f, p_v_f, diagram->GetCellUnique(), false);

		auto island = item.second;
		for (auto highestPeak_union : island.highestPeakUnion.unions) {

			for (auto highestPeak : highestPeak_union.second) {
				p_q.push(highestPeak);
			}
		}


		while (!p_q.empty()) {
			Cell* c = p_q.GetValue();
			p_q.pop();

			CellDetail& cd = c->GetDetail();
			max_moisture = std::max<unsigned int>(max_moisture, cd.GetMoisture());
			for (HalfEdge* he : c->halfEdges) {
				Edge* e = he->edge;
				Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
				CellDetail& tcd = targetCell->GetDetail();

				if (IS_LAND(tcd.GetTerrain()) && cd.GetElevation() >= tcd.GetElevation()) {
					if (!p_q.IsCalculating(targetCell->GetUnique())) {

						bool nonPeakBonus = false;
						if (cd.GetElevation() > tcd.GetElevation()) {

							if (tcd.GetMoisture() == 0) {
								tcd.SetMoisture(cd.GetMoisture());
							}
							else {
								if (tcd.GetMoisture() < cd.GetMoisture()) {
									tcd.SetMoisture(cd.GetMoisture());
									p_q.SetCalculating(targetCell->GetUnique(), false);
								}
							}
							tcd.AddMoisture(1 + nonPeakBonus);
							max_moisture = std::max<unsigned int>(max_moisture, tcd.GetMoisture());
						}
						else if (cd.GetElevation() == tcd.GetElevation()) {

							if (tcd.GetMoisture() == 0) {
								tcd.SetMoisture(cd.GetMoisture() + 1 + nonPeakBonus);
							}
							else if (tcd.GetMoisture() < cd.GetMoisture()) {
								tcd.SetMoisture(cd.GetMoisture() + 1 + nonPeakBonus);
								p_q.SetCalculating(targetCell->GetUnique(), false);
							}
							else {
								tcd.AddMoisture(1 + nonPeakBonus);
								max_moisture = std::max<unsigned int>(max_moisture, tcd.GetMoisture());
							}
						}
					}

					bool is_first = p_q.push(targetCell);
				}

			}
		}

	}

	UniBuf<std::queue, Container, decltype(u_f), decltype(v_f), Container> lake_q(u_f, v_f, diagram->GetCellUnique(), false);
	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		for (auto highestPeak_union : island.lakeUnion.unions) {
			int power = (int)highestPeak_union.second.size() * 2;
			for (auto highestPeak : highestPeak_union.second) {
				auto new_v = std::make_pair(highestPeak, power);
				lake_q.push(new_v);
			}
		}
	}

	while (!lake_q.empty()) {
		auto value = lake_q.GetValue();
		lake_q.pop();
		Cell* c = value.first;
		CellDetail& cd = c->GetDetail();
		unsigned int pow = (unsigned int)value.second;

		if (pow <= 0) continue;

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			CellDetail& tcd = targetCell->GetDetail();

			if (IS_GROUND(tcd.GetTerrain()) && cd.GetElevation() >= tcd.GetElevation()) {
				if (tcd.GetMoisture() < pow) {
					auto new_v = std::make_pair(targetCell, pow);
					if (lake_q.push(new_v)) {
						tcd.SetMoisture(pow);
					}
				}

			}

		}

	}
}


using Container_CC = std::tuple<Cell*, std::vector<Cell*>>;
struct LakeComp {
	bool operator() (Container_CC& A, Container_CC& B) {
		CellDetail& ad = get<0>(A)->GetDetail();
		CellDetail& bd = get<0>(B)->GetDetail();
		if (get<1>(A).size() < get<1>(B).size()) {
			return true;
		}
		else {
			if (get<1>(A).size() == get<1>(B).size()) {
				if (ad.GetMoisture() < bd.GetMoisture()) {
					return true;
				}
			}
			return false;
		}
	}

};


void VoronoiDiagramGenerator::CreateRiver() {
	diagram->river_lines.Clear();
	diagram->river_cross.Clear();
	diagram->RiverEdgeClear();
	diagram->RiverLineClear();

	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		using Container = std::pair<Cell*, Cell*>;
		auto u_f = [](ReturnType<Container> c) { return c.first->GetUnique(); };
		auto v_f = [](auto buffer) -> decltype(auto) { return buffer->front(); };

		for (auto lake_union : island.lakeUnion.unions) {
			//UniBuf<std::queue, Container, decltype(u_f), decltype(v_f), Container> buf(u_f, v_f, diagram->GetCellUnique(), false);

			auto p_u_f = [](ReturnType<Container> c) { return c.first->GetUnique(); };
			auto p_v_f = [](auto buffer) -> decltype(auto) { return const_cast<Container&>(buffer->top()); };
			UniBuf<std::priority_queue, Container, decltype(p_u_f), decltype(p_v_f), Container, std::vector<Container>, River::RiverPriorityComp> buf(p_u_f, p_v_f, diagram->GetCellUnique(), false);
			Cell* first_c = lake_union.second[0];

			for (auto lake : lake_union.second) {
				Container temp = std::make_pair(lake, lake);
				buf.push(std::make_pair(lake, lake));
				auto river_pos = std::make_pair(lake->GetUnique(), lake->GetUnique());
				diagram->river_lines.GetRiverEdges()[river_pos] = RiverEdge::CreateStartPoint(diagram, &diagram->river_lines, lake);

			}

			while (!buf.empty()) {
				Container value = buf.GetValue();
				buf.pop();
				Cell* c = value.first;
				Cell* pre_c = value.second;

				CellDetail& cd = c->GetDetail();

				RiverEdge* pre_e = pre_c == nullptr ?
					diagram->river_lines.GetRiverEdge(RiverEdge::GetPos(c, c)) :
					diagram->river_lines.GetRiverEdge(RiverEdge::GetPos(pre_c, c));

				int next_dist = pre_e->IsStart() ? 1 : pre_e->GetDistance() + 1;

				Cell* owner = pre_e->GetOnwer();

				//if (pre_e->GetOwnerEdge()->GetOceanConnect() > lake_union.second.size()) continue;
				auto uni_e = diagram->river_lines.GetRiverEdge(RiverEdge::GetPos(first_c, first_c));
				int cell_cnt = 0;
				if (diagram->river_lines.GetOceanConnect(pre_e->GetOnwer()) < lake_union.second.size()) {
					for (HalfEdge* he : c->halfEdges) {
						Edge* e = he->edge;
						Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
						CellDetail& tcd = targetCell->GetDetail();

						if (IS_GROUND(tcd.GetTerrain()) && !buf.IsCalculating(targetCell->GetUnique())) {
							if (tcd.GetElevation() < cd.GetElevation() ||
								(IS_WATER(cd.GetTerrain()) ||
									((tcd.IsFlat() || cd.IsFlat()) && tcd.GetElevation() == cd.GetElevation() /*&& tcd.GetMoisture() >= cd.GetMoisture()*/))) {
								auto river_pos = RiverEdge::GetPos(c, targetCell);
								//umap[RiverEdge::GetPos(c, targetCell)]

								auto iter = diagram->river_lines.GetRiverOutEdges().find(targetCell->GetUnique());
								// No rivers overlap with the new river
								if (iter == diagram->river_lines.GetRiverOutEdges().end()) {
									auto new_e = RiverEdge::Create(diagram, &diagram->river_lines, c, targetCell, owner, pre_e, nullptr, next_dist);
									diagram->river_lines.GetRiverEdges()[river_pos] = new_e;

									buf.push(std::make_pair(targetCell, c));
									cell_cnt++;

								}
								else {

									bool check1 = false;
									bool check2 = false;
									for (auto check_e : iter->second) {
										check1 = check1 || diagram->river_lines.CheckRiverEdgeLinked(pre_e->GetOnwer(), check_e->GetOnwer());
										check2 = check2 || pre_e->GetOnwer() == check_e->GetOnwer();
									}
									
									if (!check1 && !check2) {
										auto new_e = RiverEdge::Create(diagram, &diagram->river_lines, c, targetCell, owner, pre_e, nullptr, next_dist);
										//new_e->SetRiverEnd(true);
										new_e->SetRiverEnd(true);
										diagram->river_lines.GetRiverEdges()[river_pos] = new_e;
										auto find_v = iter->second;
										for (auto find_e : find_v) {
											new_e->AddNext(find_e);
											find_e->SetDistAndNextAll(next_dist + 1, owner);
											diagram->river_lines.AddLinkRiverEdge(pre_e->GetOnwer(), find_e->GetOnwer());
										}

									}


								}

							}
						}
						else if (IS_WATER(tcd.GetTerrain()) && tcd.UnionFindCell(Terrain::OCEAN) != owner->GetDetail().UnionFindCell(Terrain::OCEAN)) {
							if (IS_OCEAN(tcd.GetTerrain())) {
								if (!buf.IsCalculating(targetCell->GetUnique())) {
									buf.SetCalculating(targetCell->GetUnique(), true);
								}
								else {
									continue;
								}

								diagram->river_lines.AddOceanConnect(pre_e->GetOnwer());
							}
							else {
								if (tcd.GetTerrain() == Terrain::LAKE) {

									// If the rivers are already connected to each other, the connection will not be made.
									if (buf.IsCalculating(targetCell->GetUnique()) || diagram->river_lines.CheckRiverLinked(pre_e->GetOnwer(), targetCell)) {
										continue;
									}

									buf.SetCalculating(targetCell->GetUnique(), true);
									diagram->river_lines.AddLinkRiverEdge(pre_e->GetOnwer(), targetCell);
									diagram->river_lines.AddLinkRiver(pre_e->GetOnwer(), targetCell);


								}
							}
							//auto owner_c = pre_e->GetOnwer();
							//auto owner_pos = RiverEdge::GetPos(owner_c, owner_c);
							//if (umap.find(owner_pos) != umap.end()) {
							auto onwer_e = pre_e->GetOwnerEdge();
							//if (onwer_e->GetDistance() == 0) {
							auto new_e = RiverEdge::Create(diagram, &diagram->river_lines, c, targetCell, first_c, pre_e, nullptr, next_dist);
							//new_e->SetRiverEnd(true);
							diagram->river_lines.GetRiverEdges()[RiverEdge::GetPos(c, targetCell)] = new_e;
							//pre_e->AddNext(new_e);
							//	onwer_e->SetDist(next_dist);
							cell_cnt++;
							//}
						//}


							break;
						}

					}
				}
				if (cell_cnt == 0) {
					pre_e->DeleteLine(/*buf.GetCalculating()*/);

				}


			}
		}

	}
	
	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		//break;
		
		for (auto lake_union : island.lakeUnion.unions) {


			for (auto lake : lake_union.second) {
				auto river_pos = RiverEdge::GetPos(lake, lake);
				using Temp = std::pair< RiverEdge*, int>;
				std::stack<Temp> buf;
				if (diagram->river_lines.GetRiverEdges().find(river_pos) != diagram->river_lines.GetRiverEdges().end()) {
					buf.push(std::make_pair(diagram->river_lines.GetRiverEdges()[river_pos], lake_union.second.size()));
				}

				while (!buf.empty()) {
					auto value = buf.top();
					buf.pop();
					RiverEdge* e = value.first;

					if (e == nullptr) continue;
					auto power = std::max<int>(e->GetPower(), value.second);
					e->SetPower(power);
					if (!e->IsStart()) {
						if (e->GetStart()->GetDetail().GetElevation() < e->GetEnd()->GetDetail().GetElevation()) {
							power++;
							e->SetPower(power);
						}
					}

					if (e->GetEnd()->GetDetail().GetTerrain() != Terrain::LAKE || e->IsStart()) {

						if (e->GetNexts().size() == 0) {
							auto iter = diagram->river_lines.GetRiverOutEdges().find(e->GetEnd()->GetUnique());
							if (iter != diagram->river_lines.GetRiverOutEdges().end()) {
								for (auto iter_e : iter->second) {
									if (!iter_e->IsStart() && iter_e->GetPower() + 1 > power) {
										int new_power = std::max<int>(power, iter_e->GetPower());
										buf.push(std::make_pair(iter_e, new_power));
									}
								}
							}
						}
						else if (e->GetRiverEnd()) {
							auto iter = diagram->river_lines.GetRiverOutEdges().find(e->GetEnd()->GetUnique());
							if (iter != diagram->river_lines.GetRiverOutEdges().end()) {
								for (auto iter_e : iter->second) {
									if (!iter_e->IsStart()) {
										int new_power = power + iter_e->GetPower();
										buf.push(std::make_pair(iter_e, new_power));
									}
								}
							}
						}
						else {
							if (e->GetNexts().size() == 1 || e->IsStart()) {
								for (auto next_e : e->GetNexts()) {
									if (!next_e->IsStart()) {
										int new_power = std::max<int>(power, next_e->GetPower());
										e->SetPower(new_power);
										for (auto edge : e->GetPrevs()) {
											edge->SetPower(std::max<int>(edge->GetPower(), new_power));
										}

										buf.push(std::make_pair(next_e, new_power));
									}
								}
							}
							else {
								power /= 2;
								e->SetPower(power);
								//if (power < 1) power = 1;
								for (auto next_e : e->GetNexts()) {
									if (!next_e->IsStart()) buf.push(std::make_pair(next_e, power));
								}
							}

						}
					}

				}

			}

		}
	}


	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;


		for (auto lake_union : island.lakeUnion.unions) {


			for (auto lake : lake_union.second) {


				auto river_pos = RiverEdge::GetPos(lake, lake);
				using Temp = std::pair< RiverEdge*, RiverLine*>;
				std::stack<Temp> buf;
				if (diagram->river_lines.GetRiverEdges().find(river_pos) != diagram->river_lines.GetRiverEdges().end()) {
					buf.push(std::make_pair(diagram->river_lines.GetRiverEdges()[river_pos], RiverLine::Create(diagram, setting)));
				}


				while (!buf.empty()) {
					auto value = buf.top();
					buf.pop();
					RiverEdge* e = value.first;
					RiverLine* c_arr = RiverLine::Create(*value.second);

					if (e == nullptr) continue;

					if (e->IsStart()) c_arr->AddPoint(RiverPoint(e->GetPower(), e->GetStart()));
					else c_arr->AddPoint(RiverPoint(e->GetPower(), e->GetEnd()));

					if (e->GetNexts().size() == 0 || e->GetRiverEnd()) {
						if (c_arr->GetPointArray().size() > 1) {
							diagram->river_lines.AddLine(&diagram->river_cross, c_arr);
						}
					}
					else {
						if (e->GetNexts().size() == 1 || e->IsStart()) {
							for (auto next_e : e->GetNexts()) {
								buf.push(std::make_pair(next_e, c_arr));
							}
						}
						else if (c_arr->GetPointArray().size() > 1) {
							diagram->river_lines.AddLine(&diagram->river_cross, c_arr);
							for (auto next_e : e->GetNexts()) {
								RiverLine* new_line = RiverLine::Create(diagram, setting);
								new_line->AddPoint(RiverPoint(next_e->GetPower(), next_e->GetStart()));
								buf.push(std::make_pair(next_e, new_line));
							}
						}

					}

				}

			}

		}
	}

	diagram->river_lines.AdjustPoint();
	SetupRiverTriangle(VertexColor(Color::lake));
	diagram->RiverLineClearJunk();
}

void VoronoiDiagramGenerator::SetupRiverTriangle(VertexColor c) {
	diagram->river_lines.CreateTriagle(c);
	diagram->river_cross.CreateCrossingPointTriagle(c, setting.GetRiverRadius(), setting.GetRiverPowerScale(), 0.1);

}

void VoronoiDiagramGenerator::SetupBiome() {
}


void VoronoiDiagramGenerator::SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, VertexColor& elev_rate_c) {

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
					//v->color = Color(1, 0, 0);
				}
				//v->color = Color(1, 0, 0);
			}
		}
	}


}

void VoronoiDiagramGenerator::SetupColor(int flag) {
	image_flag = flag;

	if (has_set_color) {
		for (Vertex* v : diagram->vertices) {
			v->Reset();
		}
		for (Edge* e : diagram->edges) {
			e->color = VertexColor();
		}
	}
	has_set_color = true;

	double elev_rate = 1.0 / (double)max_elevation;
	double gray_rate = static_cast<uint16_t>((MAX_GRAY) / max_elevation);

	VertexColor elev_rate_c = VertexColor(Color(elev_rate / 2, elev_rate / 2, elev_rate / 2), gray_rate / 2);
	for (Cell* c : diagram->cells) {

		CellDetail& cd = c->GetDetail();
		if (IS_GROUND(cd.GetTerrain()) /*&& cd.GetElevation() != 0*/) {
			if (flag & ISLAND) {

				//if (false && (cd.IsPeak()) && !cd.IsFlat()) {
				//	double elev_scale = (double)(cd.GetElevation() + 1) * elev_rate;
				//	Color island_elev = Color(elev_scale, elev_scale, elev_scale);
				//	cd.GetColor() = island_elev;
				//	//cd.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().GetColor() = Color(0, 0, 0);
				//}
				//else {
				//	
				double elev_scale = (double)(cd.GetElevation()) * elev_rate;
				uint16_t gray_scale = static_cast<uint16_t>(((double)cd.GetElevation() / (double)max_elevation) * MAX_GRAY);
				VertexColor island_elev = VertexColor(Color(elev_scale, elev_scale, elev_scale), gray_scale);
				cd.GetColor() = island_elev;
				//}
			}
			else {
				cd.GetColor() = VertexColor(Color::black);
			}

		}
		else if (cd.GetTerrain() == Terrain::LAKE) {

			if (flag & LAKE) {
				if (flag == LAKE) {
					cd.GetColor() = VertexColor(Color::white);
				}
				else cd.GetColor() = VertexColor(Color::lake);
			}
			else {
				if (flag & ISLAND) {
					auto tud = cd.UnionFindCellDetail(Terrain::OCEAN);
					double elev_scale = (double)(tud.GetElevation()) * elev_rate;
					uint16_t gray_scale = static_cast<uint16_t>(((double)tud.GetElevation() / (double)max_elevation) * MAX_GRAY);
					double scale = 1;
					double color = elev_scale * scale;
					VertexColor island_elev = VertexColor(Color(color, color, color), gray_scale);

					cd.GetColor() = island_elev;
				}
				else {
					cd.GetColor() = VertexColor(Color::black);
				}
			}
		}
		else if (cd.GetTerrain() == Terrain::COAST) {
			if (flag & COAST) {
				cd.GetColor() = VertexColor(Color::coast);
			}
			else {
				if (flag & OCEAN) {
					if (cd.IsEdge()) {
						cd.GetColor() = VertexColor(Color::edgeOcean);
					}
					else {
						cd.GetColor() = VertexColor(Color::ocean);
					}

				}
				else {
					cd.GetColor() = VertexColor(Color::black);
				}
			}
		}
		else if (cd.GetTerrain() == Terrain::OCEAN) {
			if (flag & OCEAN) {
				if (cd.GetEdge()) {
					cd.GetColor() = VertexColor(Color::edgeOcean);
				}
				else {
					cd.GetColor() = VertexColor(Color::ocean);
				}
			}
			else {
				cd.GetColor() = VertexColor(Color::black);
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
			avg += he->edge->vertA->point.DistanceTo(he->edge->vertB->point);
		}
		avg /= c->halfEdges.size();

		for (auto he : c->halfEdges) {


			Edge* e = he->edge;

			Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;
			Cell* opposite_c = opposite_s->cell;

			CellDetail& tcd = opposite_s->cell->GetDetail();

			Vertex* vA = e->vertA;
			Vertex* vB = e->vertB;

			double dist = he->edge->vertA->point.DistanceTo(he->edge->vertB->point);
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
									e->color = VertexColor::MixColor(vA->color, vB->color);

								}
							}
							else {
								e->color = VertexColor::MixColor(vA->color, vB->color);
							}
						}
					}
				}
			}
			else {
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
								}
								else {
									e->color = VertexColor::MixColor(vA->color, vB->color);
								}
							}
						}
						else
						{
							e->color = VertexColor::MixColor(vA->color, vB->color);
						}
					}
					else {
						if (cd.GetElevation() > tcd.GetElevation()) {
							e->color = VertexColor::MixColor(vA->color, vB->color);
						}
						else if (cd.GetElevation() == tcd.GetElevation()) {
							e->color = cd.GetColor();
						}
						else {
							e->color = VertexColor::MixColor(vA->color, vB->color);
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

				CellDetail& tcd = opposite_s->cell->GetDetail();

				Vertex* vA = e->vertA;
				Vertex* vB = e->vertB;


				if ((vA->elev > COAST_ELEVATION && vB->elev <= COAST_ELEVATION) ||
					(vB->elev > COAST_ELEVATION && vA->elev <= COAST_ELEVATION)) {
					e->color = VertexColor::MixColor(vA->color, vB->color);
				}

			}
		}
	}
}


void VoronoiDiagramGenerator::SetupEdgePos(bool trans_edge) {
	for (Edge* e : diagram->edges) {
		if (e->rSite) {
			double dist = e->vertA->point.DistanceTo(e->vertB->point);

			Point2 norm = (e->vertA->point - e->vertB->point) / dist;
			norm = Point2(-norm.y, norm.x);
			double step = 3;
			double perp_step = 3;
		

			if (trans_edge) {
				double scale1 = (0.5 - ((1 / step) / 2)) + setting.GetRandom() / step;
				double perp_len = (dist / perp_step);
				double scale2 = (perp_len / 2) - setting.GetRandom() * perp_len;
				e->p = e->vertA->point * scale1 + e->vertB->point * (1 - scale1) + norm * scale2;
			}
			else {
				e->p = (e->vertA->point + e->vertB->point) / 2;
			}
		}
		else {
			e->p = (e->vertA->point + e->vertB->point) / 2;
		}

	}
}

void VoronoiDiagramGenerator::CreateTriangle() {
	diagram->triangles.clear();
	unsigned int cnt = 0;

	for (Cell* c : diagram->cells) {
		cnt += (unsigned int)c->halfEdges.size();
	}
	diagram->triangles.reserve(cnt);

	for (Cell* c : diagram->cells) {
		for (HalfEdge* hf : c->halfEdges) {
			Edge* e = hf->edge;
			Site* s = e->lSite->cell == c || e->rSite == nullptr ? e->lSite : e->rSite;
			//if (s == nullptr) continue;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;
			Point2 center = c->site.p;
			Point2 pA = e->vertA->point;
			Point2 pB = e->vertB->point;
			Point2 edge_mp = e->p;
			VertexColor colorA = e->vertA->color;
			VertexColor colorB = e->vertB->color;
			VertexColor edge_c = e->color;
			VertexColor center_c = s->cell->GetDetail().GetColor();

			//middle_c = (colorA + colorB) / 2;
			diagram->triangles.push_back(Triangle({ pA, center, edge_mp, colorA, center_c, edge_c }));
			diagram->triangles.push_back(Triangle({ pB, center, edge_mp, colorB, center_c, edge_c }));
		}
	}
}