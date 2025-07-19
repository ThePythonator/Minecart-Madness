#pragma once

#include "GraphicsObjects.hpp"

#include "Constants.hpp"

#include "Level.hpp"

class Player {
public:
	Player(Framework::GraphicsObjects* _graphics_objects);

	void update(float dt, Framework::InputHandler* input, Level& level);
	void render();

	uint8_t get_health() const;
	Framework::vec2 get_position() const;

private:
	Framework::vec2 position, velocity;
	uint8_t health = GAME::PLAYER::HEALTH;
	bool on_rail = false;

	Framework::GraphicsObjects* graphics_objects;
};
