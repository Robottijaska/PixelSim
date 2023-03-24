#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <iostream>


#include <algorithm>

#include "Helper.h"

namespace Pxl {

	const size_t PIXELGRID_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT / PIXEL_SIZE;
	const size_t PIXELGRID_WIDTH = SCREEN_WIDTH / PIXEL_SIZE;
	const size_t PIXELGRID_HEIGHT = SCREEN_HEIGHT / PIXEL_SIZE;

	namespace types {
		enum PixelTypesID {
			VACUUM,
			WATER,
			SAND,
			STONE,
		};

		enum Phases {
			SOLID_STATIC,
			SOLID_POWDER,
			LIQUID,
			GAS
		};
	}

	class Pixel {
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

	//---PIXEL TYPES---
	const Pixel PixelTypes[4] = {
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

		//Stone
		Pixel{
			types::STONE,
			"Stone",
			SDL_Color{ 100,100,130 },
			0.0f,
			types::SOLID_STATIC
		},
	};

	Pixel Pixels[PIXELGRID_SIZE];

	const Pixel& GetPixel(size_t pos){ return Pixels[pos]; }
	const Pixel& GetPixel(size_t x, size_t y) { return Pixels[x + y * PIXELGRID_WIDTH]; }

	bool InBounds(size_t x, size_t y) { return x < PIXELGRID_WIDTH && y < PIXELGRID_HEIGHT; }
	bool IsEmpty(size_t x, size_t y) { return InBounds(x, y) && GetPixel(x, y).ID == types::VACUUM; }

	size_t GetIndex(size_t x, size_t y) { return x + y * PIXELGRID_WIDTH; }

	bool IsMoveableTo(size_t xSrc, size_t ySrc, size_t x, size_t y) { 
		return GetPixel(x,y).Phase != types::SOLID_STATIC && InBounds(x, y) && Pixels[GetIndex(xSrc, ySrc)].GetMass() > Pixels[GetIndex(x, y)].GetMass(); 
	}

	size_t GetClosestPixel(size_t x, size_t y) { return GetIndex( (size_t)floor(x / PIXEL_SIZE), (size_t)floor(y / PIXEL_SIZE)); }

	void SetPixel(size_t x, size_t y, const Pixel& pixel) { Pixels[GetIndex(x, y)] = pixel; }
	void SetPixel(size_t index, const Pixel& pixel) { Pixels[index] = pixel; }

	std::vector<std::pair<size_t, size_t>> PixelChangesBuffer; // (1)ending pixel <- (2)starting pixel

	void TranslatePixel(size_t destination, size_t start) { 
		PixelChangesBuffer.emplace_back(destination, start); 
	}

	void CommitPixels() {
		//ensures that the queried movement has not already been requestedS
		for (size_t i = 0; i < PixelChangesBuffer.size(); i++) {
			if (Pixels[PixelChangesBuffer[i].first].GetMass() >= Pixels[PixelChangesBuffer[i].second].GetMass()) {
				PixelChangesBuffer[i] = PixelChangesBuffer.back(); PixelChangesBuffer.pop_back();
				i--;
			}
		}

		std::sort(PixelChangesBuffer.begin(), PixelChangesBuffer.end(),
			[](auto& a, auto& b) { return a.first < b.first; }
		);

		// pick random source for each destination

		size_t iprev = 0;

		PixelChangesBuffer.emplace_back(-1, -1); // to catch final move

		for (size_t i = 0; i < PixelChangesBuffer.size() - 1; i++) {
			if (PixelChangesBuffer[i + 1].first != PixelChangesBuffer[i].first) {
				size_t rand = iprev + (size_t)randInt(i - iprev);

				size_t dst = PixelChangesBuffer[rand].first;
				size_t src = PixelChangesBuffer[rand].second;

				Pixel keptPixel = Pixels[dst];
				Pixels[dst] = Pixels[src];
				Pixels[src] = keptPixel;

				iprev = i + 1;
			}
		}

		PixelChangesBuffer.clear();
	}

	Pixel selectedPixel = PixelTypes[types::VACUUM];

	//slows down pixels

	bool MoveDown(
		size_t x, size_t y,
		const Pixel& cell)
	{
		bool down = IsMoveableTo(x,y, x, y + 1);
		if (down) {
			TranslatePixel(GetIndex(x, y + 1), GetIndex(x, y));
		}

		return down;
	}

	bool MoveDownSide(
		size_t x, size_t y,
		const Pixel& cell)
	{
		bool downLeft = IsMoveableTo(x,y, x - 1, y + 1);
		bool downRight = IsMoveableTo(x,y, x + 1, y + 1);

		if (downLeft && downRight) {
			downLeft = randInt(1)-1 == 0;
			downRight = !downLeft;
		}

		if (downLeft)  TranslatePixel(GetIndex(x - 1, y + 1), GetIndex(x, y));
		else if (downRight) TranslatePixel(GetIndex(x + 1, y + 1), GetIndex(x, y));

		return downLeft || downRight;
	}

	bool MoveSide(
		size_t x, size_t y,
		const Pixel& cell)
	{
		bool left = IsMoveableTo(x,y, x - 1, y);
		bool right = IsMoveableTo(x,y, x + 1, y);

		if (left && right) {
			left = randInt(2) - 1 > 0;
			right = !left;
		}

		if (left)  TranslatePixel(GetIndex(x - 1, y), GetIndex(x, y));
		else if (right) TranslatePixel(GetIndex(x + 1, y), GetIndex(x, y));

		return left || right;
	}

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
								SetPixel( GetClosestPixel((size_t)pxlTurtle.x, (size_t)pxlTurtle.y), selectedPixel);
								break;
							case SDL_BUTTON_X1:
								SetPixel(GetClosestPixel((size_t)pxlTurtle.x, (size_t)pxlTurtle.y), Pixels[types::VACUUM]);
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
			for (int x = 0; x != PIXELGRID_WIDTH; x++)
			for (int y = 0; y != PIXELGRID_HEIGHT; y++) {
				const Pixel& currentPixel = GetPixel(x,y);
				if (currentPixel.ID != types::VACUUM && currentPixel.Phase != types::SOLID_STATIC) {
					if (MoveDown(x, y, currentPixel)){}
					else if (MoveDownSide(x, y, currentPixel)) {}
					else if (currentPixel.Phase != types::SOLID_POWDER) {
						MoveSide(x, y, currentPixel);
					}
				}
			}
		}

		CommitPixels();
	}

	void LoadPixels(SDL_Point closestPixel, int pxlState) {
		const int screenWidthPxl = SCREEN_WIDTH / PIXEL_SIZE;
		const int screenHeightPxl = SCREEN_HEIGHT / PIXEL_SIZE;
		if (pxlState == 0) {
			//makes all pixels darker
			for (int i = 0; i < SCREEN_WIDTH / PIXEL_SIZE; i++) {
				for (int j = 0; j < SCREEN_HEIGHT / PIXEL_SIZE; j++) {
					SDL_Rect newSquare = SDL_Rect{ i * PIXEL_SIZE, j * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
					SDL_Color targetCol = GetPixel(i, j).Color;
					SDL_SetRenderDrawColor(gRenderer, clampInt(targetCol.r - 25, 0, 254), clampInt(targetCol.g - 25, 0, 254), clampInt(targetCol.b - 25, 0, 254), targetCol.a);
					SDL_RenderFillRect(gRenderer, &newSquare);
				}
			}
		}
		else {
			for (int i = 0; i < SCREEN_WIDTH / PIXEL_SIZE; i++) {
				for (int j = 0; j < SCREEN_HEIGHT / PIXEL_SIZE; j++) {
					SDL_Rect newSquare = SDL_Rect{ i * PIXEL_SIZE, j * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
					SDL_Color targetCol = GetPixel(i, j).Color;
					SDL_SetRenderDrawColor(gRenderer, targetCol.r, targetCol.g, targetCol.b, targetCol.a);
					SDL_RenderFillRect(gRenderer, &newSquare);
				}
			}
			if (pxlState == 2) {
				//closest pixel to mouse is brighter
				SDL_Rect newSquare = SDL_Rect{ closestPixel.x, closestPixel.y, PIXEL_SIZE, PIXEL_SIZE };
				SDL_Color targetCol = GetPixel(closestPixel.x, closestPixel.y).Color;
				newSquare.x *= PIXEL_SIZE;
				newSquare.y *= PIXEL_SIZE;
				SDL_SetRenderDrawColor(gRenderer, clampInt(targetCol.r + 50, 0, 254), clampInt(targetCol.g + 50, 0, 254), clampInt(targetCol.b + 50, 0, 254), targetCol.a);
				SDL_RenderFillRect(gRenderer, &newSquare);
			}
		}
	}
}