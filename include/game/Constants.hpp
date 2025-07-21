#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Colour.hpp"
#include "Maths.hpp"

namespace WINDOW {
	constexpr Framework::vec2 SIZE = Framework::vec2{ 1024, 768 };
	constexpr Framework::vec2 SIZE_HALF = SIZE / 2;

	const std::string TITLE = "Minecart Madness";

	constexpr bool LIMIT_FPS = true;
	constexpr float TARGET_FPS = 120.0f;
	constexpr float TARGET_DT = 1.0f / TARGET_FPS;

	constexpr float MAX_DT = 0.05f;
}

namespace STRINGS {
	const std::string TITLE = "Minecart Madness";

	namespace TERRAIN_GENERATION {
		const std::string ALL_OPTIONS = "all_options";
		const std::string VALID_OPTIONS = "valid_options";

		const std::string UP = "up";
		const std::string DOWN = "down";
		const std::string LEFT = "left";
		const std::string RIGHT = "right";

		const std::string TERRAIN_TILES = "terrain_options";
		const std::string RAIL_TILES = "rail_options";
	}
}

namespace PATHS {
	constexpr uint8_t DEPTH = 4;

	namespace IMAGES {
		const std::string LOCATION = "assets/images/";

		const std::string MAIN_SPRITESHEET = "spritesheet.png";
		const std::string BUTTON_SPRITESHEET = "buttons.png";
		const std::string FONT_SPRITESHEET = "font.png";
	}

	namespace SAVE_DATA {
		
	}

	namespace LEVEL_DATA {
		const std::string LOCATION = "assets/levels/";

		const std::string TERRAIN_GENERATION_DATA = "terrain_generation.json";
	}
}

namespace GRAPHICS_OBJECTS {
	// Putting an enum in its own namespace is a bit hacky, but allows automatic casting, without needing enum class and all the manual casting.
	namespace IMAGES {
		enum IMAGES {
			MAIN_SPRITESHEET,
			BUTTON_SPRITESHEET,
			FONT_SPRITESHEET,
			STANDARD_BUTTON_UNSELECTED,
			STANDARD_BUTTON_HOVERED,
			STANDARD_BUTTON_SELECTED,

			TOTAL_IMAGES
		};
	}

	namespace BUTTON_IMAGE_GROUPS {
		enum BUTTON_IMAGE_GROUPS {
			STANDARD,

			TOTAL_BUTTON_IMAGE_GROUPS
		};
	}

	namespace SPRITESHEETS {
		enum SPRITESHEETS {
			MAIN_SPRITESHEET,
			BUTTON_SPRITESHEET,
			FONT_SPRITESHEET,

			TOTAL_SPRITESHEETS
		};
	}

	namespace FONTS {
		enum FONTS {
			MAIN_FONT,

			TOTAL_FONTS
		};
	}

	namespace TRANSITIONS {
		enum TRANSITIONS {
			FADE_TRANSITION,
			TOTAL_TRANSITIONS
		};
	}
}

namespace FONTS {
	namespace SIZE {
		constexpr uint8_t MAIN_FONT = 16;
	}

	namespace SCALE {
		constexpr uint8_t MAIN_FONT = 4;
	}

	namespace SPACING {
		constexpr uint8_t MAIN_FONT = 1;
	}
}

namespace COLOURS {
	const Framework::Colour BLACK { 0x00, 0x00, 0x00 };
	const Framework::Colour DARK_GREY { 0x38, 0x47, 0x66 };
	const Framework::Colour GREY { 0x6B, 0x7A, 0x99 };
	const Framework::Colour BLUE { 0xA7, 0xC7, 0xE7 };
	const Framework::Colour WHITE { 0xFF, 0xFF, 0xFF };
}

namespace TIMINGS {
	constexpr float INTRO_OPEN_TIME = 0.0f;// 4.0f;
}

namespace TRANSITIONS {
	constexpr float FADE_TIME = 1.0f;
}

namespace SPRITES {
	constexpr uint8_t SIZE = 8;
	constexpr uint8_t SIZE_HALF = SIZE / 2;
	constexpr uint8_t SCALE = 4;
	constexpr uint8_t UI_SCALE = 4;
	
	constexpr uint8_t LOGO_SCALE = SCALE * 2;

	namespace INDEX {
		constexpr uint32_t WHEEL = 137;

		constexpr uint32_t NONE = 255;
	}

	namespace RECT {
		const Framework::Rect LOGO = Framework::Rect(0, 12, 4, 4) * SPRITES::SIZE;
		const Framework::Rect MINECART = Framework::Rect(7, 8, 2, 1) * SPRITES::SIZE;
	}
}

namespace CURVES {
	namespace BEZIER {
		
	}
}

namespace BUTTONS {
	// NONE = 255, but this is defined as BUTTON_NONE_SELECTED in Button.hpp

	namespace TITLE {
		enum TITLE {
			PLAY,
			QUIT,

			TOTAL
		};
	}

	namespace PAUSED {
		enum PAUSED {
			RESUME,
			EXIT,

			TOTAL
		};
	}
}

namespace GAME {
	constexpr uint32_t CHUNK_TILE_HEIGHT = static_cast<uint32_t>(WINDOW::SIZE.y) / (SPRITES::SIZE * SPRITES::SCALE);
	constexpr uint32_t CHUNK_TILE_WIDTH = 8;

	constexpr uint32_t CHUNK_HEIGHT = CHUNK_TILE_HEIGHT * SPRITES::SIZE;
	constexpr uint32_t CHUNK_WIDTH = CHUNK_TILE_WIDTH * SPRITES::SIZE;


	constexpr uint32_t MAX_COLLAPSE_ATTEMPTS = 1000;
	constexpr uint32_t MAX_RESTART_ATTEMPTS = 10;

	namespace PLAYER {
		constexpr Framework::vec2 STARTING_POSITION = cmul(Framework::vec2{ 0.33f, 0.48f }, WINDOW::SIZE) / SPRITES::SCALE;

		constexpr uint8_t HEALTH = 3;

		constexpr float ACCELERATION = 4.0f;
		constexpr float DRAG_COEFFICIENT = 0.0002f;
		constexpr float BRAKING_DRAG_COEFFICIENT = 0.005f;
		constexpr float GRAVITY = 320.0f;

		// Speed of rotation in degrees per second
		constexpr float FALL_ROTATE_SPEED = 90.0f;
	}
}
