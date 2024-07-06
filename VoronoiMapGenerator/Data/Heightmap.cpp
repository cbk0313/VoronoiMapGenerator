#pragma once
#include "Heightmap.h"


Heightmap::Heightmap(unsigned int w, unsigned int h, bool grayscale) : is_grayscale(grayscale), width(w), height(h) {
	if (grayscale) {
		data = new uint16_t[w * h * 3];
	}
	else {
		data = new uint16_t[w * h];
	}
}

Heightmap::~Heightmap() {
	Dispose();
}

void Heightmap::Dispose() {
	delete[] data;
}

uint16_t& Heightmap::operator[](size_t index) {
	return data[index];
}

uint16_t& Heightmap::Get(unsigned int x, unsigned int y) {
	if (is_grayscale) {
		unsigned int pos = x + width * (height - y - 1);
		return data[pos];
	}
	else {
		unsigned int pos = x * 3 + width * 3 * (height - y - 1);
		return data[pos];
	}
}

uint16_t* Heightmap::GetArray() {
	return data;
}

