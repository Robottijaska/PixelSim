#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <iostream>

#include "Helper.h"

namespace Pxl {
	namespace types {
		enum PixelTypesID {
			VACUUM,
			WATER,
			SAND,
		};

		enum Phases {
			SOLID_STATIC,
			SOLID_POWDER,
			LIQUID,
			GAS
		};
	}

	struct Pixel {
	public:
		//type related

		int ID = 0;
		std::string Name;
		SDL_Color Color = SDL_Color{ 50,50,50,255 };
		float AtomMass = 0.0f;

		//non-type related

		int Phase = types::GAS;
		bool Updated = false;
		int Moles = 1;
		int Temperature = 0;
		int colOff = 0;

		float GetMass() {
			return Moles * AtomMass;
		}
	};

	Pixel Pixels[SCREEN_WIDTH / PIXEL_SIZE][SCREEN_HEIGHT / PIXEL_SIZE];
	const int PIXELGRID_WIDTH = SCREEN_WIDTH / PIXEL_SIZE;
	const int PIXELGRID_HEIGHT = SCREEN_HEIGHT / PIXEL_SIZE;

	//---PIXEL TYPES---
	Pixel PixelTypes[3] = {
		//VACUUM
		Pixel{
			types::VACUUM,
			"Vacuum",
			SDL_Color{ 50,50,50 },
			0.0f
		},

		//WATER
		Pixel{
			types::WATER,
			"Water",
			SDL_Color{ 170,170,200 },
			16.0f,
			types::LIQUID
		},

		//SAND
		Pixel{
			types::SAND,
			"Sand",
			SDL_Color{ 200,200,170 },
			60.1f,
			types::SOLID_POWDER
		},
	};

	Pixel GetPixelType(int id) {
		return PixelTypes[id];
	}

	void SetPixel(SDL_Point location, Pixel pixel) {
		Pixels[location.x][location.y].ID = pixel.ID;
		Pixels[location.x][location.y].Name = pixel.Name;
		Pixels[location.x][location.y].Color = pixel.Color;
		Pixels[location.x][location.y].AtomMass = pixel.AtomMass;
		Pixels[location.x][location.y].Phase = pixel.Phase;
	}

	//swaps two pixels from p1 to p2
	bool SwapPixels(SDL_Point p1, SDL_Point p2) {
		if (Pixels[p1.x][p1.y].GetMass() > Pixels[p2.x][p2.y].GetMass()) {
			Pixel tempPixel = Pixels[p1.x][p1.y];

			SetPixel(SDL_Point{ p1.x , p1.y }, Pixels[p2.x][p2.y]);
			SetPixel(SDL_Point{ p2.x , p2.y }, tempPixel);

			return true;
		}
		return false;
	}

	Pxl::Pixel selectedPixel = Pxl::GetPixelType(Pxl::types::VACUUM);

	//slows down pixels
	unsigned int updateTimer = 1;

	void UpdatePixels(bool isPaused, SDL_Point mPosition, SDL_Point mPositionOld, Uint32 mButton, bool mouseClick, int pxlState) {
		//placing pixels
		if (mButton)
		{
			if (pxlState == 2) {
				if (mButton) {
					float dir = (float)atan2(mPositionOld.y - mPosition.y, mPositionOld.x - mPosition.x);
					Vector2 pxlTurtle = Vector2{ (double)mPosition.x,(double)mPosition.y };
					for (int i = 0; findGLen(mPosition, SDL_Point{ (int)pxlTurtle.x, (int)pxlTurtle.y }) <= findGLen(mPosition, mPositionOld); i++) {
						if (!(pxlTurtle.y >= SCREEN_HEIGHT - 1 || pxlTurtle.x >= SCREEN_WIDTH - 1)) {
							switch (mButton) {
							case SDL_BUTTON_LEFT:
								SetPixel(findClosestPxl(SDL_Point{ (int)pxlTurtle.x, (int)pxlTurtle.y }), selectedPixel);
								break;
							case SDL_BUTTON_X1:
								SetPixel(findClosestPxl(SDL_Point{ (int)pxlTurtle.x, (int)pxlTurtle.y }), GetPixelType(types::VACUUM));
								break;
							default:
								break;
							}
						}
						pxlTurtle.x += PIXEL_SIZE * cos(dir);
						pxlTurtle.y += PIXEL_SIZE * sin(dir);
					}
				}
			}
		}

		//pixel updating loop
		if (!isPaused) {
			//gravity updates
			for (int x = 0; x != PIXELGRID_WIDTH; x++) {
				for (int y = 0; y != PIXELGRID_HEIGHT; y++) {
					Pixel currentPixel = Pixels[x][y];
					if (currentPixel.ID != types::VACUUM && !currentPixel.Updated) {
						if (currentPixel.Phase != types::SOLID_STATIC) {
							if (y + 1 < PIXELGRID_HEIGHT && SwapPixels(SDL_Point{ x,y }, SDL_Point{ x,y + 1 })) { Pixels[x][y+1].Updated = true; }
							else if ( (y + 1 < PIXELGRID_HEIGHT && x + 1 < PIXELGRID_WIDTH) && SwapPixels(SDL_Point{ x,y }, SDL_Point{ x + 1,y + 1 })) { Pixels[x + 1][y + 1].Updated = true; }
							else if ( (y + 1 < PIXELGRID_HEIGHT && x - 1 > -1) && SwapPixels(SDL_Point{ x,y }, SDL_Point{ x - 1,y + 1 })) { Pixels[x - 1][y + 1].Updated = true; }
							else if (currentPixel.Phase != types::SOLID_POWDER) {
								if (x + 1 < PIXELGRID_WIDTH && SwapPixels(SDL_Point{ x,y }, SDL_Point{ x + 1,y})) { Pixels[x + 1][y].Updated = true; }
								else if (x - 1 > -1 && SwapPixels(SDL_Point{ x,y }, SDL_Point{ x - 1,y })) { Pixels[x - 1][y].Updated = true; }
							}
						}
					}
				}
			}
			for (int x = 0; x < PIXELGRID_WIDTH; x++) {
				for (int y = 0; y < PIXELGRID_HEIGHT; y++) {
					Pixels[x][y].Updated = false;
				}
			}
		}
	}

	void LoadPixels(SDL_Point closestPixel, int pxlState) {
		const int screenWidthPxl = SCREEN_WIDTH / PIXEL_SIZE;
		const int screenHeightPxl = SCREEN_HEIGHT / PIXEL_SIZE;
		if (pxlState == 0) {
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
			if (pxlState == 2) {
				//closest pixel to mouse is brighter
				SDL_Rect newSquare = SDL_Rect{ closestPixel.x, closestPixel.y, PIXEL_SIZE, PIXEL_SIZE };
				SDL_Color targetCol = Pxl::Pixels[newSquare.x][newSquare.y].Color;
				newSquare.x *= PIXEL_SIZE;
				newSquare.y *= PIXEL_SIZE;
				SDL_SetRenderDrawColor(gRenderer, clampInt(targetCol.r + 50, 0, 254), clampInt(targetCol.g + 50, 0, 254), clampInt(targetCol.b + 50, 0, 254), targetCol.a);
				SDL_RenderFillRect(gRenderer, &newSquare);
			}
		}
	}
}