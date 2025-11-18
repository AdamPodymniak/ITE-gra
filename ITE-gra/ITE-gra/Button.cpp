#include "Button.h"

Button::Button(const char* imagePath, Vector2 imagePosition)
{
	texture = LoadTexture(imagePath);
	position = imagePosition;
}

Button::~Button() {
	UnloadTexture(texture);
}

void Button::Draw() {
	DrawTextureV(texture, position, WHITE);
}