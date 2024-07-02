#pragma once
#include "Heightmap.h"


Heightmap::Heightmap(unsigned int w, unsigned int h, bool grayscale) : is_grayscale(grayscale), width(w), height(h) {
	if (grayscale) {
		data = new unsigned char[w * h * 3];
	}
	else {
		data = new unsigned char[w * h];
	}
}

Heightmap::~Heightmap() {
	Dispose();
}

void Heightmap::Dispose() {
	delete[] data;
}

unsigned char& Heightmap::operator[](size_t index) {
	return data[index];
}

unsigned char& Heightmap::Get(unsigned int x, unsigned int y) {
	if (is_grayscale) {
		unsigned int pos = x + width * (height - y - 1);
		return data[pos];
	}
	else {
		unsigned int pos = x * 3 + width * 3 * (height - y - 1);
		return data[pos];
	}

}unsigned char* Heightmap::GetArray() {
	return data;
}

