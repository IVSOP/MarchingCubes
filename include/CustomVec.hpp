#ifndef CUSTOMVEC_H
#define CUSTOMVEC_H







// !!!!!!!!!!!!! SEGFAULT GENERATOR AHEAD!!!!!!!!!!!! READ EVERYTHING CAREFULLY !!!!!!!!!!!





#include "stdlib.hpp"
#include "types.hpp"
#include <vector>

// specialized structure, since a vector was too slow. I want to be able to memcpy lots of data, not build/emplace one by one (compiler usually does it when I don't want to)
// it is meant to be very unsafe, minimal and fast. no constructors for the objects are used, and they are never empty-initialized
// getting a pointer and adding things will absolutely destroy invalidate and eat that pointer
template <typename T>
class CustomVec {
public:
	CustomVec() = delete; // for max performance you have to decide on an initial size > 0
	
	// !!!!!cap > 0 otherwise when doubling size it gets stuck at 0
	// didnt know a better way to check this at the time
	CustomVec(std::size_t cap)
	: _sp(0), _capacity(cap)
	{
		_data = reinterpret_cast<T *>(std::malloc(sizeof(T) * cap));
	}

	~CustomVec() {
		free(_data);
	}

	// copies data from the vector into here, reserving more space if needed
	constexpr void add(const std::vector<T> &vec) {
		std::size_t og_sp = _sp;
		_sp += vec.size();
		if (og_sp == _sp) {
			return;
		}
		while (_sp > _capacity) {
			grow();
		}
		// how to force this into memcpy aligned???
		std::memcpy(reinterpret_cast<void *>(_data + og_sp), reinterpret_cast<const void *>(vec.data()), sizeof(T) * (_sp - og_sp));
		// std::copy(vec.begin(), vec.end(), _data + og_sp); // like 1 fps worse (because I dont have LTO?????)
	}

	void clear() {
		_sp = 0;
	}

	// cursed
	template <typename... Args>
	void emplace_back(Args&&... args) {
		if (_sp == _capacity) {
			grow();
		}
		// bad, calls operator=
		// _data[_sp] = T(std::forward<Args>(args)...);
		// WHAT THE FUCK??????
		new (_data + _sp) T(std::forward<Args>(args)...);
		_sp ++;
	}

	constexpr T &operator[](std::size_t index) const {
        return _data[index];
    }

	constexpr T &operator[](std::size_t index) {
        return _data[index];
    }

	constexpr T * data() const { return _data; }
	constexpr std::size_t size() const { return _sp; }
	constexpr std::size_t capacity() const { return _capacity; }

	// a bit cursed but makes it clear that the pointer is returned and not the object
	// compiler will optimize this
	constexpr T *getBackPointer() const { return &_data[_sp - 1]; }

	// makes sure at least len elements can be written with no realloc
	constexpr void reserve(size_t len) { 
		while (_sp + len > _capacity) grow();
	}

	// copy bytes into here, no questions asked, does not even resize if needed, only use if sizeof(T) == 1, will make this better in the future TODO
	// !!!!!!!!!!!!! actually just straight up assumes that T is a byte
	constexpr void copy_bytes(const void *buff, size_t len) {
		std::memcpy(reinterpret_cast<uint8_t *>(_data) + _sp, buff, len);
		_sp += len;
	}

	// also assumes T is a byte TODO
	// write bytes at [offset], no questions asked, again
	constexpr void write_bytes_at(const void *buff, size_t len, size_t offset) {
		std::memcpy(reinterpret_cast<uint8_t *>(_data) + offset, buff, len);
	}

// private:
	std::size_t _sp; // stack pointer (num elements)
	std::size_t _capacity; // capacity of underlying data structure. for max speed, starts at > 0

	T * _data;

	void grow() {
		_capacity *= 2;
		_data = reinterpret_cast<T *>(std::realloc(reinterpret_cast<void *>(_data), sizeof(T) * _capacity));
	}
};

#endif
