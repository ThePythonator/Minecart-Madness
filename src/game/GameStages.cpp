#include "GameStages.hpp"

// GameStage

void GameStage::start() {
	// Set transition
	set_transition(graphics_objects->transition_ptrs[GRAPHICS_OBJECTS::TRANSITIONS::FADE_TRANSITION].get());

	if (_first_time) {
		_first_time = false;

		// Start transition
		transition->open();

		// Create Player and Hud instances
		player = Player(graphics_objects);
		level.emplace(graphics_objects, 0); // TODO: change seed, e.g. to level number? or randomly generated
		hud = Hud(graphics_objects);
	}
}

bool GameStage::update(float dt) {
	transition->update(dt);

	level->update(dt, player->get_position(), input);
	player->update(dt, input, level.value());
	hud->update(dt);

	if (input->just_down(Framework::KeyHandler::Key::ESCAPE) || input->just_down(Framework::KeyHandler::Key::P)) {
		finish(new PausedStage(this), false);
	}

	return true;
}

void GameStage::render() {
	graphics_objects->graphics.fill(COLOURS::BLUE);

	//graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET].sprite(0, Framework::Vec(128, 64));

	level->render();
	player->render();
	hud->render(player.value());

	transition->render();
}

// PausedStage

PausedStage::PausedStage(BaseStage* background_stage) : BaseStage() {
	// Save the background stage so we can still render it, and then go back to it when done
	_background_stage = background_stage;
}

void PausedStage::start() {
	// Create buttons
	buttons.clear();
	buttons.emplace_back(
		Framework::Rect(WINDOW::SIZE_HALF - Framework::Vec(128, 32), Framework::Vec(256, 64)),
		graphics_objects->button_image_groups[GRAPHICS_OBJECTS::BUTTON_IMAGE_GROUPS::STANDARD],
		Framework::Text(&graphics_objects->fonts[GRAPHICS_OBJECTS::FONTS::MAIN_FONT], "Resume", COLOURS::BLACK, 4.0f),
		BUTTONS::PAUSED::RESUME
	);
	buttons.emplace_back(
		Framework::Rect(WINDOW::SIZE_HALF - Framework::Vec(128, -64), Framework::Vec(256, 64)),
		graphics_objects->button_image_groups[GRAPHICS_OBJECTS::BUTTON_IMAGE_GROUPS::STANDARD],
		Framework::Text(&graphics_objects->fonts[GRAPHICS_OBJECTS::FONTS::MAIN_FONT], "Exit", COLOURS::BLACK, 4.0f),
		BUTTONS::PAUSED::EXIT
	);

	// Set transition
	set_transition(graphics_objects->transition_ptrs[GRAPHICS_OBJECTS::TRANSITIONS::FADE_TRANSITION].get());
}

bool PausedStage::update(float dt) {
	transition->update(dt);

	if (input->just_down(Framework::KeyHandler::Key::ESCAPE) || input->just_down(Framework::KeyHandler::Key::P)) {
		// Return to game (exit pause)
		finish(_background_stage);
		//transition->close();
	}

	// Update buttons
	for (Framework::Button& button : buttons) {
		button.update(input);

		if (button.pressed() && transition->is_open()) {
			button_selected = button.get_id();
			if (button_selected == BUTTONS::PAUSED::RESUME) {
				// Return to game (exit pause)
				finish(_background_stage);
				//transition->close();
			}
			else {
				transition->close();
			}
		}
	}

	if (transition->is_closed()) {
		if (button_selected == BUTTONS::PAUSED::EXIT) {
			finish(new TitleStage());
		}
	}

	return true;
}

void PausedStage::render() {
	// Render background stage
	_background_stage->render();

	// Render pause menu
	graphics_objects->graphics.fill(COLOURS::BLACK, 0x7f);

	for (const Framework::Button& button : buttons) {
		button.render();
	}

	transition->render();
}