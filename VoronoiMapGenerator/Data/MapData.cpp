#pragma once
#include "MapData.h"
#include "../Cell.h"


bool CellDetail::IsEdge() {
	return b_edge;
}
bool CellDetail::IsFlat() {
	return b_flat;
}
bool CellDetail::IsPeak() {
	return unionfind.UnionFindCell(Terrain::PEAK)->detail.b_peak;
}


Terrain CellDetail::GetTerrain() {
	return terrain;
}

Color CellDetail::GetColor() {
	return color;
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
