#include "Player.hpp"

// TODO
Player::Player(Framework::GraphicsObjects* _graphics_objects)
	: graphics_objects(_graphics_objects) {

	position = { GAME::PLAYER_POSITION, 32 };
}

void Player::update(float dt, Framework::InputHandler* input, Level& level) {
	if (input->is_down(Framework::KeyHandler::Key::RIGHT)) {
		velocity.x += GAME::PLAYER::ACCELERATION * dt;
	}

	if (on_rail) {
		velocity.y = 0;
	}
	else {
		velocity.y += GAME::PLAYER::GRAVITY * dt;
	}

	velocity.x -= velocity.x * velocity.x * GAME::PLAYER::DRAG_COEFFICIENT * dt;
	position += velocity * dt;

	// Handle collisions
	on_rail = false;

	// Get height of rail at x = position.x + 5 (centre)
	float rail_height = level.rail_height_at(position.x + 5) - 4 - 1; // 4 is player height, 1 is wheel height
	if (rail_height <= position.y) {
		on_rail = true;
		position.y = rail_height;
	}

	//if (level.touching_rail(Framework::Rect(position, { 10, 5 }))) {
		//on_rail = true;
		//position.y = 
	//}
}

void Player::render() {
	// TODO: rotate minecart if on slope

	// Render minecart
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].rect(SPRITES::RECT::MINECART, { GAME::PLAYER_POSITION - 3, position.y - 4});

	// Render wheels
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER_POSITION - 3 + 1, position.y - 3 + 3 });
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER_POSITION - 3 + 7, position.y - 3 + 3 });
}

uint8_t Player::get_health() const {
	return health;
}

Framework::vec2 Player::get_position() const {
	return position;
}

