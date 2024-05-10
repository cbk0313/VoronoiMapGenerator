#include "Buffer.h"
#include "..\Cell.h"
#include <algorithm>

CellBuffer::CellBuffer(size_t size, bool dedupl) : cell_cnt(size) {
	isCalculating = std::vector<bool>(cell_cnt, false);
	SetPopDeduplication(dedupl);
}

void CellBuffer::SetPopDeduplication(bool b){
	deduplication = b;
}

void CellBuffer::push(Cell* c){
	if (!isCalculating[c->GetUnique()]) {
		isCalculating[c->GetUnique()] = true;
		QueueBuffer::push(c);
	}
}

void CellBuffer::pop(){
	if (deduplication) isCalculating[buffer.front()->GetUnique()] = false;
	QueueBuffer::pop();
}

CellVector::CellVector(size_t size) : cell_cnt(size) {
	isCalculating = std::vector<bool>(cell_cnt, false);
}

void CellVector::push_back(Cell* c) {
	if (!isCalculating[c->GetUnique()]) {
		isCalculating[c->GetUnique()] = true;
		VectorBuffer::push_back(c);
	}
}
