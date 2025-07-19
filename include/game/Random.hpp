#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "File.hpp"

#include "Constants.hpp"

class RandomGenerator {
public:
	RandomGenerator(uint32_t _seed);

	virtual uint32_t get_next() = 0;
	uint32_t get_seed();
	float random();

	template <typename T>
	T choice(const std::vector<T>& v) {
		if (v.size() == 0) {
			throw std::runtime_error("Cannot randomly select item from empty list!");
		}
		uint32_t index = static_cast<uint32_t>(random() * v.size());
		return v.at(index);
	}

	template <typename T>
	T choice(const std::vector<T>& v, const std::vector<uint32_t>& w) {
		uint32_t sum = 0;
		for (uint32_t a : w) sum += a;
		uint32_t index = static_cast<uint32_t>(random() * sum);
		uint32_t i = 0;
		//std::cout << "sum: " << sum << ", index: " << index;
		//std::cout << "i: " << i << ", w: " << w.at(i) << ", index: " << index << std::endl;
		while (index >= w.at(i)) {
			index -= w.at(i);
			i++;
			//std::cout << "i: " << i << ", w: " << w.at(i) << ", index: " << index << std::endl;
		}
		//std::cout << ",i: " << i << std::endl;
		//std::cout << "v.at(i): " << v.at(i) << std::endl;
		//return choice(v);
		return v.at(i);
	}

protected:
	uint32_t seed;
};

class Prbs : public RandomGenerator {
public:
	Prbs(uint32_t _seed, uint8_t _length, std::vector<uint8_t> _taps);

	uint32_t get_next();

private:
	const std::vector<uint8_t> taps;
	uint32_t lfsr;
	const uint8_t length;
};

class XorShift : public RandomGenerator {
public:
	XorShift(uint32_t _seed);

	uint32_t get_next();
	uint32_t get_state() { return state; }

private:
	uint32_t state;
};

class WaveFunctionCollapse {
public:
	struct ValidOptions {
		std::vector<uint32_t> up, down, left, right;
	};

	struct OptionCollections {
		std::vector<uint32_t> all, terrain, rail;
	};

	WaveFunctionCollapse(uint8_t _width, uint8_t _height, OptionCollections _options, std::map<uint32_t, uint32_t> _relative_frequencies, std::map<uint32_t, ValidOptions> _valid_options_lookup);

	static WaveFunctionCollapse create_from_file(uint8_t _width, uint8_t _height, std::string filepath);

	void reset();

	void set_cell(uint8_t x, uint8_t y, uint32_t value);
	void set_cell(uint8_t x, uint8_t y, std::vector<uint32_t> values);
	std::optional<uint32_t> get_cell(uint8_t x, uint8_t y);
	std::vector<uint32_t> get_cell_options(uint8_t x, uint8_t y);
	bool is_cell_collapsed(uint8_t x, uint8_t y);

	bool collapse_single_cell(RandomGenerator& random);
	bool collapse(RandomGenerator& random);

	OptionCollections get_option_collections();

private:
	void update_options();
	void update_surrounding_options(uint8_t x, uint8_t y);

	void reduce_options(uint8_t x, uint8_t y, std::vector<uint32_t> valid_options);

	void backtrack();
	void restart();

	const std::vector<std::pair<int, int>> directions = {
		{  0, -1 }, // up
		{  0,  1 }, // down
		{ -1,  0 }, // left
		{  1,  0 }, // right
	};

	struct Cell {
		bool collapsed;
		std::vector<uint32_t> options;
	};

	struct Decision {
		std::vector<Cell> previous_state;
		uint8_t x, y;
		uint32_t chosen_value;
	};

	const OptionCollections options;

	const std::map<uint32_t, uint32_t> relative_frequencies;
	const std::map<uint32_t, ValidOptions> valid_options_lookup;

	uint8_t width, height;
	std::vector<Cell> cells;

	//std::vector<Cell> initial_state;
	std::stack<Decision> history;
};
