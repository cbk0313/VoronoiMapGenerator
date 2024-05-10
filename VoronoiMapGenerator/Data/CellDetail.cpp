#pragma once
#include "CellDetail.h"
#include "../Cell.h"
#include "../Edge.h"


bool CellDetail::IsEdge() {
	return b_edge;
}

bool CellDetail::IsFlat() {
	return b_flat;
}

bool CellDetail::IsPeak() {
	return unionfind.UnionFindCell(Terrain::PEAK)->GetDetail().b_peak;
}

bool CellDetail::GetPeak() {
	return b_peak;
}


void CellDetail::SetFlat(bool b) {
	b_flat = b;
}

void CellDetail::SetPeak(bool b) {
	b_peak = b;
}

bool CellDetail::IsHighestPeak() {
	return unionfind.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().b_highest_peak;
}

void CellDetail::SetHighestPeak(bool b) {
	b_highest_peak = b;
}

bool CellDetail::GetHighestPeak() {
	return b_highest_peak;
}

Terrain CellDetail::GetTerrain() {
	return terrain;
}

Color& CellDetail::GetColor() {
	return color;
}

int CellDetail::GetElevation() {
	if (b_flat) {
		return elevation;
	}
	else {
		return elevation;
	}
}
void CellDetail::SetElevation(int num) {
	elevation = num;
}
void CellDetail::AddElevation(int num) {
	elevation += num;
}

UnionFind<Cell, TERRAIN_CNT>& CellDetail::GetUnionFind() {
	return unionfind;
}

void CellDetail::SetTerrain(Terrain t) {
	terrain = t;
	switch (t)
	{
	case Terrain::OCEAN:
		if (b_edge)
			color = Color(0.1, 0, 0.3, 1);
		else
			color = Color(0.2, 0, 0.6, 1);
		break;
	case Terrain::LAND:
		//color = Color(0.6, 0.4, 0, 1);
		color = Color(0, 0, 0);
		break;
	case Terrain::LAKE:
		color = Color(0.2, 0.4, 0.6);
		break;
	case Terrain::COAST:
		color *= 2;
		break;
	default:
		break;
	}
}

void CellDetail::SetEdge(bool b) {
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

void CellDetail::Reset(bool reset_edge, bool reset_terrain, bool reset_elev) {
	if (reset_edge) {
		b_edge = false;
	}
	b_peak = false;
	b_highest_peak = false;
	if (reset_elev) {
		elevation = 0;
	}
	if (reset_terrain) {
		SetTerrain(Terrain::OCEAN);
	}
	unionfind.Reset(cell);
}



bool CellDetail::CheckSurroundRising() {
	//int aroundUpper = 0;
	CellDetail& cd = cell->GetDetail();
	for (HalfEdge* he : cell->halfEdges) {
		Edge* e = he->edge;
		Cell* targetCell = (e->lSite->cell == cell && e->rSite) ? e->rSite->cell : e->lSite->cell;
		//if (targetCell->GetDetail().GetElevation() < c->GetDetail().GetElevation()) {
		CellDetail& tcd = targetCell->GetDetail();

		if (cd.GetTerrain() == Terrain::LAND && tcd.GetTerrain() == Terrain::LAND) {
			if (cd.GetElevation() < tcd.GetElevation()) {
				return true;
			}
		}
	}
	return false;
}
//
//
//Cell* CellDetail::UnionFindCell() {
//	if (unionCell == unionCell->GetDetail().unionCell) {
//		return unionCell;
//	}
//	else {
//		return unionCell = unionCell->GetDetail().UnionFindCell();
//	}
//}
//
//void CellDetail::SetUnionCell(Cell* target) {
//	auto vim_d = UnionFindCell()->detail;
//	target = target->GetDetail().UnionFindCell();
//	if (vim_d.b_edge) target->GetDetail().unionCell = vim_d.unionCell;
//	else {
//		UnionFindCell()->GetDetail().unionCell = target;
//	}
//	
//}
