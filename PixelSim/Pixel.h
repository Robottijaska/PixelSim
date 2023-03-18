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

		int ID = 0;
		std::string Name;
		SDL_Color Color = SDL_Color{ 50,50,50,255 };
		float AtomMass = 0.0f;

		//non-type related

		bool Updated = false;
		int Moles = 0;
		int Temperature = 0;
		int colOff = 0;
	};

	Pixel Pixels[SCREEN_WIDTH / PIXEL_SIZE][SCREEN_HEIGHT / PIXEL_SIZE];
	const int PIXELGRID_WIDTH = SCREEN_WIDTH / PIXEL_SIZE;
	const int PIXELGRID_HEIGHT = SCREEN_HEIGHT / PIXEL_SIZE;

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
			types::VACUUM,
			"Vacuum",
			SDL_Color{ 50,50,50 },
			0.0f
		},

		//OXYGEN
		Pixel{
			types::OXYGEN,
			"Oxygen",
			SDL_Color{ 170,170,200 },
			16.0f
		},

		//SAND
		Pixel{
			types::SAND,
			"Sand",
			SDL_Color{ 200,200,170 },
			60.1f
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
	}

	//swaps two pixels from p1 to p2
	bool SwapPixels(SDL_Point p1, SDL_Point p2) {
		if ( !(p2.y >= PIXELGRID_HEIGHT || p2.y < 0 || p2.x >= PIXELGRID_WIDTH || p2.x < 0) ) {
			if (Pixels[p1.x][p1.y].ID != Pixels[p2.x][p2.y].ID) {
				Pixel tempPixel = Pixels[p1.x][p1.y];

				Pixels[p1.x][p1.y] = Pixels[p2.x][p2.y];
				Pixels[p2.x][p2.y] = tempPixel;

				return true;
			}
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
			updateTimer--;
			if (updateTimer == 0) {

				for (int x = 0; x < PIXELGRID_WIDTH; x++) {
					for (int y = 0; y < PIXELGRID_HEIGHT; y++) {

						Pixel currentPixel = Pixels[x][y];

						if (!currentPixel.Updated && currentPixel.ID != types::VACUUM) {
							Pixels[x][y + 1].Updated = SwapPixels(SDL_Point{ x,y }, SDL_Point{ x,y + 1 });
						}
					}
				}

				for (int x = 0; x < PIXELGRID_WIDTH; x++) {
					for (int y = 0; y < PIXELGRID_HEIGHT; y++) {
						Pixels[x][y].Updated = false;
					}
				}
				//end
				updateTimer = 1;

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