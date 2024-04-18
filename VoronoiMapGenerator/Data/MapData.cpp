#pragma once
#include "MapData.h"
#include "../Cell.h"


bool CellDetail::IsEdge() {
	return b_edge;
}
bool CellDetail::IsPeak() {
	return b_peak;
}
bool CellDetail::IsHighestPeak() {
	return unionfind.UnionFindCell(Terrain::HIGHEST_PEAK)->detail.b_highest_peak;
}

void CellDetail::SetPeak(bool b) {
	b_peak = b;
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

unsigned int CellDetail::GetElevation() {
	return elevation;
}
void CellDetail::SetElevation(unsigned int num) {
	elevation = num;
}
void CellDetail::AddElevation(unsigned int num) {
	elevation += num;
}

UnionFind<Cell, TERRAIN_CNT>& CellDetail::GetUnionFind() {
	return unionfind;
}

//
//
//Cell* CellDetail::UnionFindCell() {
//	if (unionCell == unionCell->detail.unionCell) {
//		return unionCell;
//	}
//	else {
//		return unionCell = unionCell->detail.UnionFindCell();
//	}
//}
//
//void CellDetail::SetUnionCell(Cell* target) {
//	auto vim_d = UnionFindCell()->detail;
//	target = target->detail.UnionFindCell();
//	if (vim_d.b_edge) target->detail.unionCell = vim_d.unionCell;
//	else {
//		UnionFindCell()->detail.unionCell = target;
//	}
//	
//}
