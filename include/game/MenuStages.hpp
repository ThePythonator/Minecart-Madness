#pragma once

#include "BaseStage.hpp"
#include "Font.hpp"
#include "Timer.hpp"

#include "Constants.hpp"

#include "GameStages.hpp"

class IntroStage : public Framework::BaseStage {
public:
	void start();

	bool update(float dt);
	void render();

private:
	Framework::Timer intro_timer;
};

class TitleStage : public Framework::BaseStage {
public:
	void start();

	bool update(float dt);
	void render();

private:
	Framework::Text title_text;
	Framework::Timer _timer;
};
