#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <iostream>

#include "Helper.h"

namespace Pxl {
	struct Pixel {
	public:
		//type related

		SDL_Color Color = SDL_Color{ 50,50,50,255 };
		float AtomMass = 0.0f;

		//non-type related

		int Moles = 0;
		int Temperature = 0;
		int colOff = 0;
	};

	Pixel Pixels[SCREEN_WIDTH * SCREEN_HEIGHT / PIXEL_SIZE];

	namespace types {
		enum PixelTypesID {
			VACUUM,
			OXYGEN,
			SAND,
		};
	}

	//---PIXEL TYPES---
	Pixel PixelTypes[3] = {
		//VACUUM
		Pixel{
			SDL_Color{ 50,50,50 },
			0.0f
		},
		
		//OXYGEN
		Pixel{
			SDL_Color{ 170,170,200 },
			16.0f
		},

		//SAND
		Pixel{
			SDL_Color{ 200,200,170 },
			60.1f
		},
	};

	Pixel GetPixelType(int id) {
		return PixelTypes[id];
	}

	SDL_Point PixelsGetCoordinates(int input) {
		SDL_Point output = SDL_Point();
		output.x = (input % screenWidthPxl) * PIXEL_SIZE;
		output.y = (int)floor(input / static_cast<float>(screenWidthPxl))* PIXEL_SIZE;
		return output;
	}

	int GetClosestPixel(SDL_Point input) {
		input.x = floor(input.x / (PIXEL_SIZE));
		input.y = floor(input.y / (PIXEL_SIZE));
		return input.x + input.y * screenWidthPxl;
	}

	void AddPixel(int location, Pixel pixel) {
		Pixels[location].Color = pixel.Color;
		Pixels[location].AtomMass = pixel.AtomMass;
	}

	void UpdatePixels(SDL_Event* Event, int closestPixel) {
		if (mouseDown && SDL_BUTTON_LMASK)
		{
			SDL_GetMouseState(&mPosition.x, &mPosition.y);
			clampInt(&mPosition.x, 0, SCREEN_WIDTH);
			clampInt(&mPosition.y, 0, SCREEN_HEIGHT);
			AddPixel(closestPixel, GetPixelType(types::OXYGEN));
		}
	}
}