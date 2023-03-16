#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Helper.h"

namespace Gfx {
	void DrawFPScounter(float fps) {
		std::string fps_counter = "FPS: " + std::to_string(fps);
		SDL_Color textColor = { 255, 255, 255, 0 };
		SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, fps_counter.c_str(), textColor);
		SDL_Texture* text = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		int text_width = textSurface->w;
		int text_height = textSurface->h;
		SDL_FreeSurface(textSurface);
		SDL_Rect renderQuad = { 20, 20, text_width, text_height };
		SDL_RenderCopy(gRenderer, text, NULL, &renderQuad);
		SDL_DestroyTexture(text);
	}

	void DrawCoordinates(SDL_Point closestPixel) {
		std::string material_disp = "X: " + std::to_string(closestPixel.x) + " Y: " + std::to_string(closestPixel.y);
		SDL_Color textColor = { 255, 255, 255, 0 };
		SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, material_disp.c_str(), textColor);
		SDL_Texture* text = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		int text_width = textSurface->w;
		int text_height = textSurface->h;
		SDL_FreeSurface(textSurface);
		SDL_Rect renderQuad = { SCREEN_WIDTH - 150, SCREEN_HEIGHT - 40, text_width, text_height };
		SDL_RenderCopy(gRenderer, text, NULL, &renderQuad);
		SDL_DestroyTexture(text);
	}

	SDL_Texture* loadTexture(std::string path)
	{
		//The final texture
		SDL_Texture* newTexture = NULL;

		//Load image at specified path
		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
		if (loadedSurface == NULL)
		{
			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		}
		else
		{
			//Create texture from surface pixels
			newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
			if (newTexture == NULL)
			{
				printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			}

			//Get rid of old loaded surface
			SDL_FreeSurface(loadedSurface);
		}

		return newTexture;
	}
}