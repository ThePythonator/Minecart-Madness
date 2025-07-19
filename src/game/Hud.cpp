#include "Hud.hpp"

Hud::Hud(Framework::GraphicsObjects* _graphics_objects)
	: graphics_objects(_graphics_objects) {

	// Create animation
	Framework::Animation heart_animation = {
		{196, 3.00f, Framework::SpriteTransform::NONE},
		{197, 0.05f, Framework::SpriteTransform::NONE},
		{198, 0.05f, Framework::SpriteTransform::NONE},
		{199, 0.05f, Framework::SpriteTransform::NONE},
		{200, 0.05f, Framework::SpriteTransform::NONE},
		{201, 0.05f, Framework::SpriteTransform::NONE}
	};
	// Create animation handler
	heart = std::make_unique<Framework::AnimationHandler>(graphics_objects->spritesheets[GRAPHICS_OBJECTS::SPRITESHEETS::MAIN_SPRITESHEET], heart_animation);


	// Create fps text
	fps_text = Framework::Text(&graphics_objects->fonts[GRAPHICS_OBJECTS::FONTS::MAIN_FONT], "", COLOURS::BLACK, 2);

}

void Hud::update(float dt) {
	heart->update(dt);

	uint32_t fps = static_cast<uint32_t>(1.0f / dt);
	fps_text.set_text(std::to_string(fps));
}

void Hud::render(const Player& player) {
	for (uint8_t i = 0; i < player.get_health(); i++) {
		heart->render(Framework::Vec(4 + i * SPRITES::SIZE, 4));
	}

	//fps_text.render({ 2, 0 }, Framework::Font::AnchorPosition::TOP_LEFT);
}
