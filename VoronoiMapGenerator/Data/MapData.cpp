#pragma once
#include "MapData.h"
#include "../Cell.h"


Cell* CellDetail::unionFindCell() {
	if (unionCell == unionCell->detail.unionCell) {
		return unionCell;
	}
	else {
		return unionCell = unionCell->detail.unionFindCell();
	}
}

void CellDetail::setUnionCell(Cell* target) {
	auto vim_d = unionFindCell()->detail;
	target = target->detail.unionFindCell();
	if (vim_d.is_edge) target->detail.unionCell = vim_d.unionCell;
	else {
		unionFindCell()->detail.unionCell = target;
	}
	
}
