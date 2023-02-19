#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <algorithm>

#include "Helper.h"

namespace Gui {

	bool inGui = false;

	bool mouseOff = true;

	void UpdateGuis() {

	}
}

namespace Gfx {
	void LoadPixels(SDL_Point closestPixel) {
		const int screenWidthPxl = SCREEN_WIDTH / PIXEL_SIZE;
		const int screenHeightPxl = SCREEN_HEIGHT / PIXEL_SIZE;
		if (Gui::inGui) {
			//makes all pixels darker
			for (int i = 0; i < SCREEN_WIDTH / PIXEL_SIZE; i++) {
				for (int j = 0; j < SCREEN_HEIGHT / PIXEL_SIZE; j++) {
					SDL_Rect newSquare = SDL_Rect{ i * PIXEL_SIZE, j * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
					SDL_Color targetCol = Pxl::Pixels[i][j].Color;
					SDL_SetRenderDrawColor(gRenderer, clampInt(targetCol.r - 25, 0, 254), clampInt(targetCol.g - 25, 0, 254), clampInt(targetCol.b - 25, 0, 254), targetCol.a);
					SDL_RenderFillRect(gRenderer, &newSquare);
				}
			}
		}
		else {

			for (int i = 0; i < SCREEN_WIDTH / PIXEL_SIZE; i++) {
				for (int j = 0; j < SCREEN_HEIGHT / PIXEL_SIZE; j++) {
					SDL_Rect newSquare = SDL_Rect{ i * PIXEL_SIZE, j * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
					SDL_Color targetCol = Pxl::Pixels[i][j].Color;
					SDL_SetRenderDrawColor(gRenderer, targetCol.r, targetCol.g, targetCol.b, targetCol.a);
					SDL_RenderFillRect(gRenderer, &newSquare);
				}
			}

			//closest pixel to mouse is brighter
			SDL_Rect newSquare = SDL_Rect{ closestPixel.x, closestPixel.y, PIXEL_SIZE, PIXEL_SIZE };
			SDL_Color targetCol = Pxl::Pixels[newSquare.x][newSquare.y].Color;
			newSquare.x *= PIXEL_SIZE;
			newSquare.y *= PIXEL_SIZE;
			SDL_SetRenderDrawColor(gRenderer, clampInt(targetCol.r + 50, 0, 254), clampInt(targetCol.g + 50, 0, 254), clampInt(targetCol.b + 50, 0, 254), targetCol.a);
			SDL_RenderFillRect(gRenderer, &newSquare);
		}
	}

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