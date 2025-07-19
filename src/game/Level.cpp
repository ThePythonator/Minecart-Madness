#include "Level.hpp"

Level::Level(Framework::GraphicsObjects* _graphics_objects, uint32_t _seed)
	: graphics_objects(_graphics_objects)
	, seed(_seed)
	, random(_seed)
	, wfc(WaveFunctionCollapse::create_from_file(
		// Add 1 tile to the left side to allow chunks to be stitched together
		GAME::CHUNK_TILE_WIDTH + 2, GAME::CHUNK_TILE_HEIGHT,
		graphics_objects->base_path + PATHS::LEVEL_DATA::LOCATION + PATHS::LEVEL_DATA::TERRAIN_GENERATION_DATA
	)) {
	next_chunk_id = 0;
}

void Level::update(float dt, const Framework::vec2& player_position, Framework::InputHandler* input) {
	// Check which chunks need to be loaded
	float left_edge = player_position.x - GAME::PLAYER_POSITION;
	float right_edge = left_edge + WINDOW::SIZE.x / SPRITES::SCALE;
	uint32_t leftmost_chunk_id = static_cast<uint32_t>(left_edge / GAME::CHUNK_WIDTH);
	uint32_t rightmost_chunk_id = static_cast<uint32_t>(right_edge / GAME::CHUNK_WIDTH) + 1;

	scroll = left_edge;

	//printf("left: %f, right: %f\n", left_edge, right_edge);
	//printf("l: %d, r: %u\n", leftmost_chunk_id, rightmost_chunk_id);

	// TODO: Now check whether chunk ids in the left to right range (inclusive) are actually loaded
	// and load them if not
	// TODO: maybe rework this slightly
	if (input->just_down(Framework::KeyHandler::Key::SPACE) || input->is_down(Framework::KeyHandler::Key::RETURN)) {
		//generate_next_tile();
	}

	//while (next_chunk_id <= rightmost_chunk_id) {
	if (next_chunk_id <= rightmost_chunk_id && chunk_loader_status == std::future_status::ready) {
		std::cout << "New chunk___________" << std::endl;
		//generate_next_chunk();
		chunk_loader_thread = std::async(std::launch::async, [this]() { generate_next_chunk(); });
	}
	
	chunk_loader_status = chunk_loader_thread.wait_for(std::chrono::milliseconds(1));

	// Also free up any chunks which are below the leftmost_chunk_id
	// item == [key, value]
	std::erase_if(chunks, [leftmost_chunk_id](const auto& item) { return item.first < leftmost_chunk_id; });
}

void Level::render() {
	for (const auto& [chunk_id, chunk] : chunks) {
		Framework::vec2 chunk_pos = Framework::Vec(chunk_id * GAME::CHUNK_TILE_WIDTH, 0);
		graphics_objects->graphics.render_rect({ chunk_pos * SPRITES::SCALE * SPRITES::SIZE - Framework::vec2{scroll * SPRITES::SCALE, 0}, {GAME::CHUNK_WIDTH * SPRITES::SCALE, GAME::CHUNK_HEIGHT * SPRITES::SCALE}}, COLOURS::WHITE);
		Framework::vec2 tile_pos = chunk_pos;
		for (const ChunkColumn& column : chunk.chunk_grid) {
			tile_pos.y = 0;
			for (uint32_t tile_id : column) {
				if (tile_id != SPRITES::INDEX::NONE) {
					Framework::vec2 pos = tile_pos * SPRITES::SIZE;
					pos.x -= scroll;
					graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(tile_id, pos);
				}
				tile_pos.y++;
			}
			tile_pos.x++;
		}
	}
}

uint32_t Level::get_seed() {
	return seed;
}

bool Level::touching_rail(Framework::Rect rect) {
	std::vector<uint32_t> rail_tile_ids = wfc.get_option_collections().rail;

	for (auto [chunk_id, x, y] : get_overlapping_tile_coords(rect)) {
		// Check if each tile is a rail
		if (!chunks.contains(chunk_id)) continue; // Chunk not generated?

		// TODO: check x and y - are these not necessarily valid? (y must be valid)
		uint32_t tile_id = chunks.at(chunk_id).chunk_grid[x][y];

		if (std::find(rail_tile_ids.begin(), rail_tile_ids.end(), tile_id) != rail_tile_ids.end()) {
			// tile_id was a rail tile
			// Check rect collision
			//Framework::Rect rail_rect(chunk_id * GAME::CHUNK_WIDTH + x * SPRITES::SIZE, y * SPRITES::SIZE + 6, SPRITES::SIZE, 2);
			//if (tile_id == 187 || tile_id == 188) {
			//	rail_rect.position.y = y * SPRITES::SIZE;
			//	rail_rect.size.y = SPRITES::SIZE;
			//}
			//if (Framework::colliding(rect, rail_rect)) {
			//	// Take into account slopes
			//	if (tile_id == 187) {
			//		// rect_position.x + 5 is the midpoint of the minecart
			//		float scale = (rect.position.x + 5 - rail_rect.position.x) / SPRITES::SIZE;
			//		return rail_rect.position.y + (1 - scale) * SPRITES::SIZE;
			//	}
			//	else if (tile_id == 188) {
			//		float scale = (rect.position.x + 5 - rail_rect.position.x) / SPRITES::SIZE;
			//		return rail_rect.position.y + scale * SPRITES::SIZE;
			//	}

			//	return rail_rect.position.y;
			//}
			return true;
		}
	}
	return false;
}

float Level::rail_height_at(float x) {
	uint32_t chunk_id = x / GAME::CHUNK_WIDTH;
	x -= chunk_id * GAME::CHUNK_WIDTH;
	uint32_t tile_x = x / SPRITES::SIZE;

	// Often because chunk is not loaded
	if (!chunks.contains(chunk_id)) return GAME::CHUNK_HEIGHT + SPRITES::SIZE; // Enough to disappear off screen

	// Offset tile_x by 1 because rail_heights is 2 wider than the chunk, one at each end
	auto [height, direction] = get_rail_heights(chunk_id).at(tile_x + 1);
	height *= SPRITES::SIZE;

	float scale = (x - tile_x * SPRITES::SIZE) / SPRITES::SIZE;

	// TODO: move to constants
	switch (direction) {
	case RailDirection::NONE:
		return height + 6;
	case RailDirection::UP:
		return height + 6 + SPRITES::SIZE - scale * SPRITES::SIZE;
	case RailDirection::DOWN:
		return height + 6 - SPRITES::SIZE + scale * SPRITES::SIZE;
	}
}

std::vector<std::pair<uint8_t, Level::RailDirection>> Level::get_rail_heights(uint32_t chunk_id) {
	return chunks.at(chunk_id).rail_heights;
}

std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> Level::get_overlapping_tile_coords(Framework::Rect rect) {
	// Find which chunk(s) the rect is in
	uint32_t left_chunk_id = rect.topleft().x / GAME::CHUNK_WIDTH;
	uint32_t right_chunk_id = rect.topright().x / GAME::CHUNK_WIDTH;

	// (chunk_id, x, y)
	std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> coord_list;

	// Check both chunks
	for (uint32_t chunk_id = left_chunk_id; chunk_id < right_chunk_id + 1; chunk_id++) {
		// Scaled pixel coords within the chunk
		Framework::vec2 chunk_coords = { rect.position.x - chunk_id * GAME::CHUNK_WIDTH, rect.position.y };
		// Tile coords within the chunk
		Framework::vec2 tile_coords = chunk_coords / SPRITES::SIZE;
		// Width and height of rect, rounded up
		float tile_width = ceil(rect.size.x / SPRITES::SIZE);
		float tile_height = ceil(rect.size.y / SPRITES::SIZE); // TODO: maybe add 1?
		// Find tile locations the rect is overlapping
		for (uint32_t x = 0; x < tile_width; x++) {
			for (uint32_t y = 0; y < tile_height; y++) {
				uint32_t tile_x = tile_coords.x + x;
				uint32_t tile_y = tile_coords.y + y;

				// Don't allow invalid x or y coordinates
				if (tile_x < 0 || tile_x >= GAME::CHUNK_TILE_WIDTH) continue;
				if (tile_y < 0 || tile_y >= GAME::CHUNK_TILE_HEIGHT) continue;

				coord_list.emplace_back(chunk_id, tile_x, tile_y);
			}
		}
	}
	return coord_list;
}

void Level::generate_next_chunk() {
	Chunk last_chunk;
	if (next_chunk_id == 0) {
		for (uint8_t i = 0; i < 2; i++) {
			last_chunk.rail_heights.emplace_back(GAME::CHUNK_TILE_HEIGHT / 2, RailDirection::NONE); // TODO: maybe pick more intelligently
		}
	}
	else {
		last_chunk = chunks.at(next_chunk_id - 1);
	}

	Chunk chunk;

	std::cout << "Chunk " << next_chunk_id << " seed: " << random.get_state() << std::endl;

	// Reset grid stored within the wavefront solver
	wfc.reset();

	// NOTE: alternative idea: generate terrain, then fit rail to it on a second pass?

	WaveFunctionCollapse::OptionCollections options = wfc.get_option_collections();
	std::vector<uint32_t> non_rail = wfc.get_option_collections().all;
	std::erase_if(non_rail, [options](uint32_t o) { return std::find(options.rail.begin(), options.rail.end(), o) != options.rail.end(); });
	for (uint8_t x = 0; x < GAME::CHUNK_TILE_WIDTH + 2; x++) {
		// Don't allow rails on the very bottom line
		wfc.set_cell(x, GAME::CHUNK_TILE_HEIGHT - 1, non_rail);
	}

	for (uint8_t i = 0; i < 2; i++) {
		auto [height, direction] = last_chunk.rail_heights.at(i + last_chunk.rail_heights.size() - 2);
		chunk.rail_heights.emplace_back(height, direction);

		uint32_t index;
		switch (direction) {
		case RailDirection::NONE:
			index = 133;
			break;
		case RailDirection::UP:
			index = 187;
			height++;
			break;
		case RailDirection::DOWN:
			index = 188;
			break;
		}
		//std::cout << "Copying " << index << " to (" << (int)i << ", " << (int)height << ")" << std::endl;
		wfc.set_cell(i, height, index);
	}

	/*std::cout << "Last rail heights: ";
	for (auto [a,t] : last_chunk_data.rail_heights) {
		std::cout << (int)a << ", ";
	}
	std::cout << std::endl;

	std::cout << "New rail heights: ";
	for (auto [a, t] : rail_heights) {
		std::cout << (int)a << ", ";
	}
	std::cout << std::endl;*/

	// TODO: in middle of changing to use last_chunk_data.rail_heights
	for (uint8_t x = 2; x < GAME::CHUNK_TILE_WIDTH + 2; x++) {
		// Generate a path for the rail
		// Do this by selecting a direction to travel in each frame
		auto [previous_rail_height, previous_rail_direction] = chunk.rail_heights.at(chunk.rail_heights.size() - 1);
		uint32_t new_rail_height = previous_rail_height;
		float value = random.random();

		// TODO: make these constants?
		if (value < 0.25f && previous_rail_direction != RailDirection::DOWN && previous_rail_height > 12) {
			// Go up, but only if wasn't just going down
			new_rail_height--;
			previous_rail_direction = RailDirection::UP;

			// TODO: set tile options
			wfc.set_cell(x, new_rail_height + 1, 187); // TODO: change to list of options
		}
		else if (0.25f <= value && value < 0.5f && previous_rail_direction != RailDirection::UP && previous_rail_height < 20) {
			// Go down, but only if wasn't just going up
			new_rail_height++;
			previous_rail_direction = RailDirection::DOWN;

			// TODO: set tile options
			wfc.set_cell(x, new_rail_height, 188); // TODO: change to list of options
		}
		else {
			// Go straight
			// So do nothing
			previous_rail_direction = RailDirection::NONE;

			// TODO: instead of forcing these sprites, instead add these as options to wave function
			wfc.set_cell(x, new_rail_height, 133); // TODO: change to list of options
		}
		chunk.rail_heights.emplace_back(new_rail_height, previous_rail_direction);
	}

	// Copy last column of tiles from the previous chunk
	if (next_chunk_id > 0) {
		for (uint8_t y = 0; y < GAME::CHUNK_TILE_HEIGHT; y++) {
			if (last_chunk.chunk_grid[GAME::CHUNK_TILE_WIDTH - 1][y] < 256) // TEMP
			wfc.set_cell(0, y, last_chunk.chunk_grid[GAME::CHUNK_TILE_WIDTH - 1][y]);
			//std::cout << "y=" << (int)y << ": " << last_chunk[GAME::CHUNK_TILE_WIDTH - 1][y] << std::endl;

			/*std::cout << "y=" << (int)y << ": ";
			for (auto option : wfc.get_cell_options(1, y)) {
				std::cout << option << ", ";
			}
			std::cout << std::endl;*/
		}
	}

	// TODO: don't actually do this - need to incorporate generated terrain
	if (!wfc.collapse(random)) {
		// Rather hacky approach: just try again!
		// This could get stuck in an infinite loop if it is impossible to find a valid chunk
		//generate_next_chunk();
		//return;
	}

	// Copy data from wfc to chunk
	// Don't copy leftmost column, since that's used to stitch together the chunks
	for (uint8_t x = 0; x < GAME::CHUNK_TILE_WIDTH; x++) {
		for (uint8_t y = 0; y < GAME::CHUNK_TILE_HEIGHT; y++) {
			if (auto value = wfc.get_cell(x + 1, y)) {
				chunk.chunk_grid[x][y] = value.value();
			}
		}
	}

	// TEMP: put coins in incomplete cells
	for (uint8_t x = 0; x < GAME::CHUNK_TILE_WIDTH; x++) {
		for (uint8_t y = 0; y < GAME::CHUNK_TILE_HEIGHT; y++) {
			chunk.chunk_grid[x][y] = 144;
			if (auto value = wfc.get_cell(x + 1, y)) {
				chunk.chunk_grid[x][y] = value.value();
			}
		}
	}

	chunks.emplace(next_chunk_id, chunk);
	next_chunk_id++;
}
