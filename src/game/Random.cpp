#include "Random.hpp"

RandomGenerator::RandomGenerator(uint32_t _seed)
	: seed(_seed) {

}

uint32_t RandomGenerator::get_seed() {
	return seed;
}

float RandomGenerator::random() {
	return static_cast<float>(get_next()) / std::numeric_limits<uint32_t>::max();
}

Prbs::Prbs(uint32_t _seed, uint8_t _length, std::vector<uint8_t> _taps)
	: RandomGenerator(_seed)
	, length(_length)
	, taps(_taps)
	, lfsr(_seed) {
	// Max length is 31
	if (length >= 32) {
		throw std::runtime_error("PRBS length cannot exceed 31 (was " + std::to_string(length) + ")!");
	}
	if (taps.size() == 0) {
		throw std::runtime_error("Cannot have an empty list of taps!");
	}
}

uint32_t Prbs::get_next() {
	uint32_t new_bit = lfsr >> taps.at(0);
	for (uint8_t i = 1; i < taps.size(); i++) {
		new_bit ^= lfsr >> taps.at(i);
	}
	new_bit &= 0x1;
	
	uint32_t mask = (1 << length) - 1;
	lfsr = ((lfsr << 1) | new_bit) & mask;

	return lfsr;
}

XorShift::XorShift(uint32_t _seed)
	: RandomGenerator(_seed)
	, state(_seed) {
	
}

uint32_t XorShift::get_next() {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

WaveFunctionCollapse::WaveFunctionCollapse(uint8_t _width, uint8_t _height, OptionCollections _options, std::map<uint32_t, uint32_t> _relative_frequencies, std::map<uint32_t, ValidOptions> _valid_options_lookup)
	: width(_width), height(_height)
	, options(_options)
	, relative_frequencies(_relative_frequencies)
	, valid_options_lookup(_valid_options_lookup) {
	cells.resize(width * height, Cell{ false, options.all });
	update_options();
}

WaveFunctionCollapse WaveFunctionCollapse::create_from_file(uint8_t _width, uint8_t _height, std::string filepath) {
	Framework::JSONHandler::json data = Framework::JSONHandler::read(filepath);

	try {
		std::vector<uint32_t> _all_options;
		std::map<uint32_t, uint32_t> _relative_frequencies;
		for (auto& [key, value] : data.at(STRINGS::TERRAIN_GENERATION::ALL_OPTIONS).items()) {
			_all_options.emplace_back(std::stoul(key));
			_relative_frequencies.emplace(std::stoul(key), value);
		}
		
		std::map<uint32_t, WaveFunctionCollapse::ValidOptions> _valid_options_lookup;
		for (auto& [key, value] : data.at(STRINGS::TERRAIN_GENERATION::VALID_OPTIONS).items()) {
			WaveFunctionCollapse::ValidOptions valid_options;
			valid_options.up = value.at(STRINGS::TERRAIN_GENERATION::UP).get<std::vector<uint32_t>>();
			valid_options.down = value.at(STRINGS::TERRAIN_GENERATION::DOWN).get<std::vector<uint32_t>>();
			valid_options.left = value.at(STRINGS::TERRAIN_GENERATION::LEFT).get<std::vector<uint32_t>>();
			valid_options.right = value.at(STRINGS::TERRAIN_GENERATION::RIGHT).get<std::vector<uint32_t>>();

			_valid_options_lookup.emplace(std::stoul(key), valid_options);
		}

		std::vector<uint32_t> _terrain_options = data.at(STRINGS::TERRAIN_GENERATION::TERRAIN_TILES).get<std::vector<uint32_t>>();
		std::vector<uint32_t> _rail_options = data.at(STRINGS::TERRAIN_GENERATION::RAIL_TILES).get<std::vector<uint32_t>>();

		WaveFunctionCollapse::OptionCollections options{ _all_options, _terrain_options, _rail_options };
		return WaveFunctionCollapse(_width, _height, options, _relative_frequencies, _valid_options_lookup);
	}
	catch (const Framework::JSONHandler::type_error& error) {
		std::cerr << "Unable to parse JSON rule file for the WaveFunctionCollapse class!" << std::endl;
		throw std::runtime_error("Unable to parse JSON rule file for the WaveFunctionCollapse class!");
	}
}

void WaveFunctionCollapse::reset() {
	std::fill(cells.begin(), cells.end(), Cell{ false, options.all });
	while (history.size()) history.pop();
}

void WaveFunctionCollapse::set_cell(uint8_t x, uint8_t y, uint32_t value) {
	Cell& cell = cells.at(y * width + x);
	cell.collapsed = true;
	cell.options = { value };
	update_surrounding_options(x, y);
}

void WaveFunctionCollapse::set_cell(uint8_t x, uint8_t y, std::vector<uint32_t> values) {
	cells.at(y * width + x).options = values;
}

std::optional<uint32_t> WaveFunctionCollapse::get_cell(uint8_t x, uint8_t y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return {};
	const Cell& cell = cells.at(y * width + x);
	if (cell.options.size() != 1) return {};
	return cell.options.at(0);
}

std::vector<uint32_t> WaveFunctionCollapse::get_cell_options(uint8_t x, uint8_t y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return {};
	return cells.at(y * width + x).options;
}

bool WaveFunctionCollapse::is_cell_collapsed(uint8_t x, uint8_t y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return false; // Not sure if should be true or false
	return cells.at(y * width + x).collapsed;
}

bool WaveFunctionCollapse::collapse_single_cell(RandomGenerator& random) {
	// Find cell with lowest entropy
	std::vector<std::pair<uint32_t, uint32_t>> candidates;
	uint32_t min_entropy = std::numeric_limits<uint32_t>::max();

	for (uint8_t x = 0; x < width; x++) {
		for (uint8_t y = 0; y < height; y++) {
			std::vector<uint32_t> options = get_cell_options(x, y);

			if (options.size() > 0 && !is_cell_collapsed(x, y)) {
				uint32_t entropy = 0;

				for (uint32_t option : options) {
					entropy += option * relative_frequencies.at(option);
				}

				if (entropy == min_entropy) {
					candidates.push_back({ x, y });
				}
				else if (entropy < min_entropy) {
					min_entropy = entropy;
					candidates = { {x, y} };
				}
			}
			else if (options.size() == 0) {
				//std::cout << "Dead end! (" << (int)x << ", " << (int)y << ")" << std::endl;
				if (history.size() == 0) return false; // TEMP
				backtrack();

				// We don't want to carry on to the rest of the function:
				// everything needs to be checked again
				return false;
			}
		}
	}

	if (candidates.size() == 0) {
		// No more options, we're done
		std::cout << "Clean finish" << std::endl;
		return true;
	}
	else {
		// Randomly select cell to collapse
		auto [x, y] = random.choice(candidates);

		//std::cout << "# candidates: " << candidates.size() << std::endl;

		// Get options for that cell
		std::vector<uint32_t> options = get_cell_options(x, y);

		if (options.size() == 0) {
			while (history.size()) {
				Decision d = history.top();
				history.pop();

				std::cout << "x=" << d.x << ", y=" << d.y << ", v=" << d.chosen_value << std::endl;
			}
			std::cout << "Giving up!?" << std::endl;
			return true; // just give up
		}

		//std::cout << "Chose (" << x << ", " << y << ")" << std::endl;

		std::vector<uint32_t> weights;
		for (uint32_t option : options) {
			weights.push_back(relative_frequencies.at(option));
		}

		// Randomly select from those options
		//uint32_t final_value = random.choice(options);
		uint32_t final_value = random.choice(options, weights);

		// Update history
		history.emplace(cells, x, y, final_value);

		// Set that cell to the collapsed value
		set_cell(x, y, final_value);

		// Update adjacent cells
		update_surrounding_options(x, y);

		// Assume that there is still more to do
		return false;
	}
}

bool WaveFunctionCollapse::collapse(RandomGenerator& random) {
	uint32_t restart_attempts = 0;
	uint32_t collapse_attempts = 0;
	while (!collapse_single_cell(random)) {
		collapse_attempts++;
		if (collapse_attempts >= GAME::MAX_COLLAPSE_ATTEMPTS) {
			// Restart from beginning
			restart();
			collapse_attempts = 0;
			restart_attempts++;
			if (restart_attempts > GAME::MAX_RESTART_ATTEMPTS) {
				std::cerr << "Max restart attempts exceeded!" << std::endl;
				return false;
			}
		}
	}
	return true;
}

WaveFunctionCollapse::OptionCollections WaveFunctionCollapse::get_option_collections() {
	return options;
}

void WaveFunctionCollapse::update_options() {
	// Adjust all options based on collapsed items
	for (uint8_t x = 0; x < width; x++) {
		for (uint8_t y = 0; y < height; y++) {
			std::vector<uint32_t> options = get_cell_options(x, y);

			// 0 = up, 1 = down, 2 = left, 3 = right
			for (uint8_t i = 0; i < directions.size(); i++) {
				auto [dx, dy] = directions[i];
				if (auto value = get_cell(x + dx, y + dy)) {
					const ValidOptions& valid_options = valid_options_lookup.at(*value);

					// Up/down and left/right swapped because you need to look in the other direction
					std::vector<uint32_t> choices = std::vector<std::vector<uint32_t>>{
						valid_options.down,
						valid_options.up,
						valid_options.right,
						valid_options.left,
					}[i];

					// Remove any options which are not present in valid_options
					reduce_options(x, y, choices);
				}
			}
		}
	}
}

void WaveFunctionCollapse::update_surrounding_options(uint8_t x, uint8_t y) {
	if (auto value = get_cell(x, y)) {
		const ValidOptions& valid_options = valid_options_lookup.at(*value);

		bool p = false;
		if (p) {
			std::cout << "Value: " << *value << ", x: " << (int)x << ", y: " << (int)y << std::endl;

			std::cout << " Up before: ";
			for (auto& item : get_cell_options(x, y - 1)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << " Down before: ";
			for (auto& item : get_cell_options(x, y + 1)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << " Left before: ";
			for (auto& item : get_cell_options(x - 1, y)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << " Right before: ";
			for (auto& item : get_cell_options(x + 1, y)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;
		}

		std::vector<uint32_t> options;

		reduce_options(x, y - 1, valid_options.up);
		reduce_options(x, y + 1, valid_options.down);
		reduce_options(x - 1, y, valid_options.left);
		reduce_options(x + 1, y, valid_options.right);

		if (p) {
			std::cout << "  Up after: ";
			for (auto& item : get_cell_options(x, y - 1)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << "  Down after: ";
			for (auto& item : get_cell_options(x, y + 1)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << "  Left after: ";
			for (auto& item : get_cell_options(x - 1, y)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;

			std::cout << "  Right after: ";
			for (auto& item : get_cell_options(x + 1, y)) {
				std::cout << item << ", ";
			}
			std::cout << std::endl;
		}

		// TODO: maybe check four adjacent cells 
	}
}

void WaveFunctionCollapse::reduce_options(uint8_t x, uint8_t y, std::vector<uint32_t> valid_options) {
	if (x < 0 || x >= width || y < 0 || y >= height) return;
	std::vector<uint32_t> options = get_cell_options(x, y);

	bool not_collapsed = options.size() != 1;

	std::erase_if(options, [valid_options](uint32_t v) { return std::find(valid_options.begin(), valid_options.end(), v) == valid_options.end(); });

	//if (options.size() == 0) {
	//	backtrack(); // TODO: temp
	//}
	
	set_cell(x, y, options);

	// Check if this caused the cell to be collapsed
	if (not_collapsed && options.size() == 1) {
		update_surrounding_options(x, y);
	}
}

void WaveFunctionCollapse::backtrack() {
	// Undo last decision and remove the option from the available choices

	if (history.size() == 0) {
		// No way of restarting!
		std::cerr << "Error while generating level: cannot backtrack any further!" << std::endl;
		return;
	}

	// Get last decision made
	Decision last_decision = history.top();
	history.pop(); // Remove last decision from history

	// Undo state
	cells = last_decision.previous_state;

	std::vector<uint32_t> options = get_cell_options(last_decision.x, last_decision.y);

	std::erase_if(options, [last_decision](uint32_t v) { return v == last_decision.chosen_value; });

	/*if (options.size() == 0) {
		backtrack();
	}*/

	// Update available choices
	set_cell(last_decision.x, last_decision.y, options);

	// Update surrounding cells
	update_surrounding_options(last_decision.x, last_decision.y);
}

void WaveFunctionCollapse::restart() {
	/*if (history.size() == 0) {
		throw std::runtime_error("Cannot restart with no history!");
	}*/

	while (history.size()) {
		Decision d = history.top();
		history.pop();

		// Get original state
		if (!history.size()) {
			std::cout << "Restarting!" << std::endl;
			cells = d.previous_state;
		}
	}
}
