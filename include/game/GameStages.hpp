#pragma once

#include <optional>

#include "Constants.hpp"

#include "BaseStage.hpp"
#include "MenuStages.hpp"
#include "Animation.hpp"

#include "Player.hpp"
#include "Level.hpp"
#include "Hud.hpp"

class GameStage : public Framework::BaseStage {
public:
	void start();

	bool update(float dt);
	void render();

private:
	// Use std::unique_ptr so that we can delay construction without needing default ctors
	std::optional<Player> player;
	std::optional<Level> level;
	std::optional<Hud> hud;

	bool _first_time = true;
};

class PausedStage : public Framework::BaseStage {
public:
	PausedStage(BaseStage* background_stage);

	void start();

	bool update(float dt);
	void render();

private:
	BaseStage* _background_stage;
};
