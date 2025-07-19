#pragma once

#include <memory>

#include "Animation.hpp"
#include "Font.hpp"
#include "GraphicsObjects.hpp"

#include "Constants.hpp"
#include "Player.hpp"

class Hud {
public:
	Hud(Framework::GraphicsObjects* _graphics_objects);

	void update(float dt);
	void render(const Player& player);

private:
	Framework::GraphicsObjects* graphics_objects;

	std::unique_ptr<Framework::AnimationInterface> heart;
	Framework::Text fps_text;
};
