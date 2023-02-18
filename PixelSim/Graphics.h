#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <iostream>
#include <algorithm>

#include "Helper.h"

namespace Gfx {
	void LoadPixels(int closestPixel) {
		const int screenWidthPxl = SCREEN_WIDTH / PIXEL_SIZE;
		const int screenHeightPxl = SCREEN_HEIGHT / PIXEL_SIZE;
		for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT / PIXEL_SIZE; i++) {

			SDL_Rect newSquare = SDL_Rect{};
			newSquare.x = (i%screenWidthPxl)*PIXEL_SIZE;
			newSquare.y = (int)floor(i/screenWidthPxl) * PIXEL_SIZE;
			newSquare.w = PIXEL_SIZE;
			newSquare.h = PIXEL_SIZE;

			SDL_Color targetCol = Pxl::Pixels[i].Color;

			int newColOff;
			if (Gui::inGui) {
				newColOff = -25;
			}
			else { 
				if (i == closestPixel) {
					newColOff = 50;
				}
				else {
					newColOff = 0;
				}
			}
			Pxl::Pixels[i].Color = targetCol;
			SDL_SetRenderDrawColor(gRenderer, clampInt(newColOff + targetCol.r, 0, 255), clampInt(newColOff + targetCol.g, 0, 255), clampInt(newColOff + targetCol.b, 0, 255), clampInt(targetCol.a, 0, 255));
			SDL_RenderFillRect(gRenderer, &newSquare);
		}
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

	namespace Gui {

		bool inGui = false;

		bool mouseOff = true;

		void UpdateGuis() {

		}
	}
}