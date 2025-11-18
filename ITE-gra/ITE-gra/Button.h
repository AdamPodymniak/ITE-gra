#pragma once
#include <raylib.h>

class Button{
	public:
		Button(const char *imagePath, Vector2 imagePosition);
		~Button();
		void Draw();
	private: 
		Texture2D texture;
		Vector2 position;
};