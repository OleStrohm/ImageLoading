//
// Created by Ole on 10.09.2017.
//

#include "bitarray.h"
#include <string>

BitArray::BitArray()
		: localSize(0), totalSize(0) {
	current = new std::bitset<BITSET_SIZE>();
	bitsets.push_back(current);
}

BitArray::BitArray(const int& value, unsigned int length)
		: localSize(0), totalSize(0) {
	current = new std::bitset<BITSET_SIZE>();
	bitsets.push_back(current);
	
	if (length > sizeof(int) * 8)
		length = sizeof(int) * 8;
	
	for (unsigned int i = 0; i < length; i++) {
		pushBack((bool) ((value >> i) & 0x1));
	}
}

BitArray::BitArray(std::string& bits)
		: localSize(0), totalSize(0) {
	current = new std::bitset<BITSET_SIZE>();
	bitsets.push_back(current);
	
	pushBack(bits);
}

BitArray::BitArray(const bool* const bits, const unsigned int& size)
		: localSize(0), totalSize(0) {
	current = new std::bitset<BITSET_SIZE>();
	bitsets.push_back(current);
	
	for (int i = 0; i < size; i++)
		pushBack(bits[i]);
}

BitArray::~BitArray() {
	for (auto bitset : bitsets)
		delete bitset;
}

void BitArray::pushBack(const bool& one) {
	if (localSize == BITSET_SIZE) {
		current = new std::bitset<BITSET_SIZE>();
		localSize = 0;
	}
	(*current)[localSize] = one;
	localSize++;
	updateSize();
}

void BitArray::pushBack(const unsigned char& byte) {
	for (unsigned int i = 0; i < sizeof(byte) * 8; i++) {
		pushBack((bool) ((byte >> i) & 1));
	}
}

void BitArray::pushBack(const std::string& bits) {
	for (char c : bits) {
		if (c == '0')
			pushBack((bool) 0);
		else if (c == '1')
			pushBack((bool) 1);
	}
}

void BitArray::setBit(const unsigned int& pos, bool one) {
	if (pos >= getSize())
		return;
	bitsets[pos / BITSET_SIZE]->set(pos % BITSET_SIZE, one);
}

bool BitArray::getBit(const unsigned int& position) const {
	if (position >= getSize())
		return false;
	return (*(bitsets[position / BITSET_SIZE]))[position % BITSET_SIZE];
}

void BitArray::addBit(const unsigned int& position) {
	if (position >= getSize())
		return;
	if (getBit(position)) {
		setBit(position, false);
		addBit(position + 1);
	} else {
		setBit(position, true);
	}
}

void BitArray::add(const unsigned int& value) {
	for (unsigned int i = 0; i < sizeof(int) * 8; i++) {
		if ((value >> i) & 0x1)
			addBit(i);
	}
}

void BitArray::add(const BitArray& value) {
	const unsigned int count = (value.getSize() < getSize() ? value.getSize() : getSize());
	
	for (unsigned int i = 0; i < count; i++)
		if (value.getBit(i))
			addBit(i);
}

unsigned char BitArray::read(const unsigned int& position, const unsigned int& length, const bool& reverse) {
	unsigned char result = 0;
	for(unsigned int i = 0; i < length; i++) {
		if(getBit(position + (reverse ? length - 1 - i : i)))
			result |= 1 << i;
	}
	return result;
}

void BitArray::updateSize() {
	totalSize = (unsigned int) (BITSET_SIZE * (bitsets.size() - 1) + localSize);
}

const std::string BitArray::toString() const {
	std::string out;
	for (unsigned int i = 0; i < getSize(); i++) {
		out = (getBit(i) ? "1" : "0") + out;
	}
	return out;
}

std::ostream& operator<<(std::ostream& stream, const BitArray& array) {
	return stream << array.toString();
}

std::ostream& operator<<(std::ostream& stream, const BitArray* const array) {
	return stream << array->toString();
}
