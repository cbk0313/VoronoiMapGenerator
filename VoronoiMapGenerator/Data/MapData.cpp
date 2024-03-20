#pragma once
#include "MapData.h"
#include "../Cell.h"


Cell* CellDetail::findUnionCell() {
	if (unionCell == unionCell->detail.unionCell) {
		return unionCell;
	}
	else {
		return unionCell = unionCell->detail.findUnionCell();
	}
}

void CellDetail::setUnionCell(Cell* target) {
	auto vim_d = findUnionCell()->detail;
	target = target->detail.findUnionCell();
	if (vim_d.outer) target->detail.unionCell = vim_d.unionCell;
	else {
		findUnionCell()->detail.unionCell = target;
	}
	
}
