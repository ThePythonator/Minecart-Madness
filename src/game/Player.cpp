#include "Player.hpp"

// TODO
Player::Player(Framework::GraphicsObjects* _graphics_objects)
	: graphics_objects(_graphics_objects) {

	position = GAME::PLAYER::STARTING_POSITION;

	health = GAME::PLAYER::HEALTH;
	on_rail = false;
	angle = 0.0f;
	wheels.left_y = wheels.right_y = position.y;
	start_delay.start();
}

void Player::update(float dt, Framework::InputHandler* input, Level& level) {
	start_delay.update(dt);
	if (start_delay.time() < 1.0f) return;

	bool speed_up = input->is_down(Framework::KeyHandler::Key::RIGHT);
	if (speed_up) {
		velocity.x += GAME::PLAYER::ACCELERATION * dt;
	}

	if (on_rail) {
		//velocity.y -= velocity.y * 0.5f * dt;
		if (velocity.y > 0.0f) velocity.y = 0;
	}
	else {
		velocity.y += GAME::PLAYER::GRAVITY * dt;
	}

	velocity.x -= velocity.x * velocity.x * (speed_up ? GAME::PLAYER::DRAG_COEFFICIENT : GAME::PLAYER::BRAKING_DRAG_COEFFICIENT) * dt;
	position += velocity * dt;

	angle += GAME::PLAYER::FALL_ROTATE_SPEED * dt;
	if (angle > 45.0f) angle = 45.0f;

	// Handle collisions
	on_rail = false;

	// Get height of rail at x = position.x + 5 (centre)
	float rail_height = level.rail_height_at(position.x + 5) - 4 - 1; // 4 is player height, 1 is wheel height
	//std::cout << "rail: " << rail_height << ", pos y: " << position.y << std::endl;
	if (rail_height <= position.y + 0.5f) {
	//if (position.y - 0.5f <= rail_height && rail_height <= position.y + 0.5f) {
		on_rail = true;
		velocity.y = (rail_height - position.y) / dt;
		position.y = rail_height;
	}

	wheels.left_y = level.rail_height_at(position.x + 5 - 3) - 4 - 1;
	wheels.right_y = level.rail_height_at(position.x + 5 + 3) - 4 - 1;
	//position.y = (wheels.left_y + wheels.right_y) / 2.0f - 1.0f;

	//if (level.touching_rail(Framework::Rect(position, { 10, 5 }))) {
		//on_rail = true;
		//position.y = 
	//}
}

void Player::render() {
	// Rotate minecart if on slope
	// Note: this could avoid visual flickering if the rail_height_at function returned the direction of the rail
	// TODO: clipping with rails occurs due to only using midpoint of minecart. Is there a better way?

	// TODO: idea: check rail height at both wheels. Set minecart height to avg of that, then rotate minecart as necessary to line up wheels?
	// Note that due to rotations the visual wheel locations will be slightly different, but shouldn't be too noticable

	if (on_rail) {
		angle = atan2(wheels.right_y - wheels.left_y, 6) * 180.0f * std::numbers::inv_pi;
	}
	// Otherwise, keep previous angle

	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].rect(SPRITES::RECT::MINECART, { GAME::PLAYER::STARTING_POSITION.x - 3, position.y - 4 }, SPRITES::SCALE, angle, { 8 * SPRITES::SCALE, 8 * SPRITES::SCALE });

	// Render wheels
	//graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER::STARTING_POSITION.x - 3 + 4, position.y - 3 + 3 }, SPRITES::SCALE, angle, { 4 * SPRITES::SCALE, 4 * SPRITES::SCALE });
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER::STARTING_POSITION.x - 3 + 1, position.y - 3 + 3 }, SPRITES::SCALE, angle, { 7 * SPRITES::SCALE, 4 * SPRITES::SCALE });
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER::STARTING_POSITION.x - 3 + 7, position.y - 3 + 3 }, SPRITES::SCALE, angle, { 1 * SPRITES::SCALE, 4 * SPRITES::SCALE });

	return;

	float dx = 6.0f;
	float dy = wheels.right_y - wheels.left_y;

	float cy = (wheels.left_y + wheels.right_y) / 2.0f;

	float h = sqrt(dx * dx + dy * dy);

	float nx = dx * dx / h;
	float ny = dy * dx / h;
	std::cout << "ly: " << wheels.left_y << std::endl;
	std::cout << "ry: " << wheels.right_y << std::endl;
	std::cout << "dy: " << dy << std::endl;
	std::cout << "cy: " << cy << std::endl;
	std::cout << "h: " << h << std::endl;
	std::cout << "nx: " << nx << std::endl;
	std::cout << "ny: " << ny << std::endl;
	float hx = nx / 2.0f;
	float hy = ny / 2.0f;
	float ly, ry;

	float lx = GAME::PLAYER::STARTING_POSITION.x + 4 - hx;
	float rx = GAME::PLAYER::STARTING_POSITION.x + 4 + hx;

	if (wheels.left_y < wheels.right_y) {
		ly = cy - hy;
		ry = cy + hy;
	}
	else {
		ly = cy - hy;
		ry = cy + hy;
	}

	float m_angle = atan2(ry - ly, rx - lx) * 180.0f * std::numbers::inv_pi;

	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].rect(SPRITES::RECT::MINECART, { GAME::PLAYER::STARTING_POSITION.x - 3, cy - 4 }, SPRITES::SCALE, m_angle, { 8 * SPRITES::SCALE, 8 * SPRITES::SCALE });

	//graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER::STARTING_POSITION.x - 3 + 1, wheels.left_y }, SPRITES::SCALE, angle, { 4 * SPRITES::SCALE, 4 * SPRITES::SCALE });
	//graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { GAME::PLAYER::STARTING_POSITION.x - 3 + 7, wheels.right_y }, SPRITES::SCALE, angle, { 4 * SPRITES::SCALE, 4 * SPRITES::SCALE });

	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { lx - 3, ly }, SPRITES::SCALE, m_angle, { 4 * SPRITES::SCALE, 4 * SPRITES::SCALE });
	graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(SPRITES::INDEX::WHEEL, { rx - 3, ry }, SPRITES::SCALE, m_angle, { 4 * SPRITES::SCALE, 4 * SPRITES::SCALE });

}

uint8_t Player::get_health() const {
	return health;
}

Framework::vec2 Player::get_position() const {
	return position;
}

