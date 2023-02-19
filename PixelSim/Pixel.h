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

	Pixel Pixels[SCREEN_WIDTH / PIXEL_SIZE][SCREEN_HEIGHT / PIXEL_SIZE];

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

	void SetPixel(SDL_Point location, Pixel pixel) {
		Pixels[location.x][location.y].Color = pixel.Color;
		Pixels[location.x][location.y].AtomMass = pixel.AtomMass;
	}

	void UpdatePixels(SDL_Point mPosition, Uint32 mButton, SDL_Point closestPixel) {
		if (mButton)
		{
			clampInt(&mPosition.x, 0, SCREEN_WIDTH);
			clampInt(&mPosition.y, 0, SCREEN_HEIGHT);

			switch (mButton) {
				case SDL_BUTTON_LEFT:
					SetPixel(closestPixel, GetPixelType(types::OXYGEN));
					break;
				case SDL_BUTTON_X1:
					SetPixel(closestPixel, GetPixelType(types::VACUUM));
					break;
				default:
					break;
			}
		}
	}
}