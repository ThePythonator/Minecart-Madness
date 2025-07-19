#pragma once

#include <array>
#include <chrono>
#include <cmath>
#include <future>
#include <map>

#include "GraphicsObjects.hpp"
#include "Maths.hpp"

#include "Constants.hpp"
#include "Random.hpp"

class Level {
public:
	enum class RailDirection {
		NONE,
		UP,
		DOWN
	};

	Level(Framework::GraphicsObjects* _graphics_objects, uint32_t _seed);

	void update(float dt, const Framework::vec2& player_pos, Framework::InputHandler* input);
	void render();

	uint32_t get_seed();

	bool touching_rail(Framework::Rect rect);
	float rail_height_at(float x);
	std::vector<std::pair<uint8_t, RailDirection>> get_rail_heights(uint32_t chunk_id);

private:
	void generate_next_chunk();

	std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> get_overlapping_tile_coords(Framework::Rect rect);

	Framework::GraphicsObjects* graphics_objects;

	typedef std::array<uint32_t, GAME::CHUNK_TILE_HEIGHT> ChunkColumn;
	typedef std::array<ChunkColumn, GAME::CHUNK_TILE_WIDTH> ChunkGrid;

	struct Chunk {
		ChunkGrid chunk_grid;
		std::vector<std::pair<uint8_t, RailDirection>> rail_heights;
	};

	std::map<uint32_t, Chunk> chunks; // Key of map determines chunk ID
	uint32_t next_chunk_id;
	
	uint32_t seed;
	XorShift random;
	WaveFunctionCollapse wfc;

	float scroll = 0.0f;

	std::future_status chunk_loader_status = std::future_status::ready;
	std::future<void> chunk_loader_thread;
};