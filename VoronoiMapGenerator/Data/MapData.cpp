#pragma once
#include "MapData.h"
#include "../Cell.h"


bool CellDetail::isEdge() {
	return b_edge;
}
bool CellDetail::isFlat() {
	return b_flat;
}
bool CellDetail::isPeak() {
	return unionfind.unionFindCell(static_cast<int>(Terrain::PEAK))->detail.b_peak;
}


Terrain CellDetail::getTerrain() {
	return terrain;
}

Color CellDetail::getColor() {
	return color;
}


//
//
//Cell* CellDetail::unionFindCell() {
//	if (unionCell == unionCell->detail.unionCell) {
//		return unionCell;
//	}
//	else {
//		return unionCell = unionCell->detail.unionFindCell();
//	}
//}
//
//void CellDetail::setUnionCell(Cell* target) {
//	auto vim_d = unionFindCell()->detail;
//	target = target->detail.unionFindCell();
//	if (vim_d.b_edge) target->detail.unionCell = vim_d.unionCell;
//	else {
//		unionFindCell()->detail.unionCell = target;
//	}
//	
//}
