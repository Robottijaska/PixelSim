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
		float Moles = 0;
		Vector2 Velocity = {0,0};
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

	enum class PixelMoveTypes : size_t {
		SWAP,
		FLOOD,
		TRUEFLOOD,
		INJECT
	};

	Pixel Pixels[PIXELGRID_SIZE];

	const Pixel& GetPixel(size_t pos) { return Pixels[pos]; }
	const Pixel& GetPixel(size_t x, size_t y) { return Pixels[x + y * PIXELGRID_WIDTH]; }

	bool InBounds(size_t x, size_t y) { return x < PIXELGRID_WIDTH&& y < PIXELGRID_HEIGHT; }
	bool IsEmpty(size_t x, size_t y) { return InBounds(x, y) && GetPixel(x, y).ID == types::VACUUM; }

	size_t GetIndex(size_t x, size_t y) { return x + y * PIXELGRID_WIDTH; }

	bool IsMoveableTo(size_t xSrc, size_t ySrc, size_t x, size_t y, size_t type) {
		if (type == (size_t)PixelMoveTypes::SWAP) {
			return GetPixel(x, y).Phase != types::SOLID_STATIC &&
				InBounds(x, y) &&
				Pixels[GetIndex(xSrc, ySrc)].GetMass() > Pixels[GetIndex(x, y)].GetMass()
				;
		}
		else if (type == (size_t)PixelMoveTypes::FLOOD || type == (size_t)PixelMoveTypes::TRUEFLOOD) {
			return InBounds(x, y) &&
				(GetPixel(x, y).ID == GetPixel(xSrc, ySrc).ID || GetPixel(x, y).ID == types::VACUUM) &&
				GetPixel(x, y).Moles < 1 //1 is temporary, it represents moles# a pixel can hold
				;
		}
		else if (type == (size_t)PixelMoveTypes::INJECT) {
			return InBounds(x, y) &&
				GetPixel(x, y).ID == GetPixel(xSrc, ySrc).ID
				;
		}
	}

	size_t GetClosestPixel(size_t x, size_t y) { return GetIndex((size_t)floor(x / PIXEL_SIZE), (size_t)floor(y / PIXEL_SIZE)); }

	void SetPixel(size_t x, size_t y, const Pixel& pixel) { Pixels[GetIndex(x, y)] = pixel; }
	void SetPixel(size_t index, const Pixel& pixel) { Pixels[index] = pixel; }

	struct PixelMoveRequest {
		size_t StartIndex;
		size_t EndIndex;
		size_t Type;
	};
	std::vector<PixelMoveRequest> PixelChangesBuffer; // (1)ending pixel <- (2)starting pixel

	void CommitPixels() {

		std::sort(PixelChangesBuffer.begin(), PixelChangesBuffer.end(),
			[](auto& a, auto& b) { return a.EndIndex < b.EndIndex; }
		);

		//if multiple movements terminate at the same location, choose a random one to keep and delete the rest
		size_t similarChangesIndex = 0;
		size_t similarChangesCount = 0;
		for (size_t i = 1; i < PixelChangesBuffer.size() + 1; i++) {
			if (i != PixelChangesBuffer.size() &&
				PixelChangesBuffer[i - 1].EndIndex == PixelChangesBuffer[i].EndIndex)
			{
				similarChangesCount++;
			}
			else {
				if (similarChangesCount != 0) {

					PixelMoveRequest chosenMove = PixelChangesBuffer[randInt(similarChangesCount) + similarChangesIndex];

					for (size_t j = 0; j < similarChangesCount; j++) {
						PixelChangesBuffer.erase(PixelChangesBuffer.begin() + similarChangesIndex);
					}
					PixelChangesBuffer[similarChangesIndex] = chosenMove;
				}
				similarChangesIndex = i;
				similarChangesCount = 0;
			}
		}

		std::sort(PixelChangesBuffer.begin(), PixelChangesBuffer.end(),
			[](auto& a, auto& b) { return a.Type > b.Type; }
		);

		// pick random source for each destination

		size_t iprev = 0;

		PixelChangesBuffer.emplace_back(PixelMoveRequest{ (size_t)-1, (size_t)-1, (size_t)-1 }); // to catch final move

		for (size_t i = 0; i < PixelChangesBuffer.size() - 1; i++) {
			PixelMoveRequest currentChange = PixelChangesBuffer[i];
			if (PixelChangesBuffer[i + 1].EndIndex != currentChange.EndIndex) {
				size_t rand = iprev + (size_t)randInt(i - iprev);

				size_t dst = PixelChangesBuffer[rand].EndIndex;
				size_t src = PixelChangesBuffer[rand].StartIndex;

				if (currentChange.Type == (size_t)PixelMoveTypes::SWAP &&
					GetPixel(dst).Phase != types::SOLID_STATIC &&
					Pixels[src].GetMass() > Pixels[dst].GetMass()
					) {
					Pixel keptPixel = Pixels[dst];
					Pixels[dst] = Pixels[src];
					Pixels[src] = keptPixel;
				}
				else if ((currentChange.Type == (size_t)PixelMoveTypes::FLOOD || currentChange.Type == (size_t)PixelMoveTypes::TRUEFLOOD) &&
					(GetPixel(dst).ID == GetPixel(src).ID || GetPixel(dst).ID == types::VACUUM) &&
					GetPixel(dst).Moles < 1 //1 is temporary, it represents moles# a pixel can hold
					) {
					if (currentChange.Type == (size_t)PixelMoveTypes::FLOOD) {
						float moleAddAmount = std::min(1 - GetPixel(dst).Moles, GetPixel(src).Moles); //1 is temporary, it represents moles# a pixel can hold (again...)
						if (GetPixel(dst).ID == types::VACUUM) {
							Pixels[dst] = Pixels[src];
							Pixels[src] = PixelTypes[types::VACUUM];
						}
						else {
							Pixels[dst].Moles += moleAddAmount;
							Pixels[src].Moles -= moleAddAmount;
							if (Pixels[src].Moles == 0) {
								Pixels[src] = PixelTypes[types::VACUUM];
							}
						}
					}
					else if (Pixels[src].Moles > 0.1) { //TRUEFLOOD
						float moleSetAmount = 0.5 * (GetPixel(dst).Moles + GetPixel(src).Moles); //finding average
						if (GetPixel(dst).ID == types::VACUUM) {
							Pixels[dst] = Pixels[src];

							Pixels[src].Moles = moleSetAmount;
							Pixels[dst].Moles = moleSetAmount;
						}
						else {
							Pixels[dst].Moles = moleSetAmount;
							Pixels[src].Moles = moleSetAmount;
						}
					}
				}
				else if (currentChange.Type == (size_t)PixelMoveTypes::INJECT) {
					Pixels[dst] = Pixels[src];
					Pixels[dst].Moles += Pixels[src].Moles;
					SetPixel(src, PixelTypes[types::VACUUM]);
				}

				iprev = i + 1;
			}
		}

		PixelChangesBuffer.clear();
	}

	Pixel selectedPixel = PixelTypes[types::VACUUM];

	//slows down pixels

	bool MovePixelDirect(
		size_t x, size_t y,
		size_t endX, size_t endY,
		const Pixel& cell,
		size_t moveType
		)
	{
		bool canMove = IsMoveableTo(x,y, endX, endY, moveType);
		if (canMove) {
			PixelChangesBuffer.emplace_back(PixelMoveRequest{ GetIndex(x, y), GetIndex(endX, endY), moveType });
		}

		return canMove;
	}

	bool MovePixelRandom(
		size_t x, size_t y, 
		size_t endX1, size_t endY1,
		size_t endX2, size_t endY2,
		const Pixel& cell,
		size_t moveType
		)
	{
		bool choice1 = IsMoveableTo(x,y, endX1, endY1, moveType);
		bool choice2 = IsMoveableTo(x,y, endX2, endY2, moveType);

		if (choice1 && choice2) {
			float Pixel1Mass = Pixels[GetIndex(endX1, endY1)].GetMass();
			float Pixel2Mass = Pixels[GetIndex(endX2, endY2)].GetMass();

			if (Pixel1Mass > Pixel2Mass) {
				choice1 = false;
				choice2 = true;
			}
			else if (Pixel1Mass < Pixel2Mass) {
				choice1 = true;
				choice2 = false;
			}
			else {
				choice1 = randInt(1) - 1 == 0;
				choice2 = !choice1;
			}
		}

		if (choice1) PixelChangesBuffer.emplace_back(PixelMoveRequest{ GetIndex(x, y), GetIndex(endX1, endY1), moveType });
		else if (choice2) PixelChangesBuffer.emplace_back(PixelMoveRequest{ GetIndex(x, y), GetIndex(endX2, endY2), moveType });

		return choice1 || choice2;
	}

	void UpdatePixelsMove(size_t x, size_t y) {
		const Pixel& currentPixel = GetPixel(x, y);
		if (currentPixel.ID != types::VACUUM && currentPixel.Phase != types::SOLID_STATIC) {

			if (currentPixel.Phase > types::SOLID_POWDER) {
				//FLUIDS
				//move down
				if (MovePixelDirect(
					x, y,

					x, y + 1,

					currentPixel,
					(size_t)PixelMoveTypes::FLOOD
				)){}
				//move down-sideways
				else if (MovePixelRandom(
					x, y,

					x - 1, y + 1,
					x + 1, y + 1,

					currentPixel,
					(size_t)PixelMoveTypes::FLOOD
				)){}
				//move sideways
				else if (MovePixelRandom(
					x, y,

					x-1, y,
					x+1, y,
					currentPixel,
					(size_t)PixelMoveTypes::TRUEFLOOD
				)){}
				if(false){}
			}

			//POWDERS
			//move down
			else if (MovePixelDirect(
				x, y,

				x, y + 1,

				currentPixel,
				(size_t)PixelMoveTypes::SWAP
			)){}

			//move down and sideways (for powders only)
			else if (MovePixelRandom(
				x, y,

				x - 1, y + 1,
				x + 1, y + 1,

				currentPixel,
				(size_t)PixelMoveTypes::SWAP
			)){}
		}
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

							const size_t targetIndex = GetClosestPixel((size_t)pxlTurtle.x, (size_t)pxlTurtle.y);
							switch (mButton) {
							case SDL_BUTTON_LEFT:
								SetPixel(targetIndex, selectedPixel);
								Pixels[targetIndex].Moles = 1;
								break;
							case SDL_BUTTON_X1:
								SetPixel(targetIndex, PixelTypes[types::VACUUM]);
								Pixels[targetIndex].Moles = 0;
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
			for (int x = 0; x < PIXELGRID_WIDTH; x++)
			for (int y = 0; y < PIXELGRID_HEIGHT; y++) {
				UpdatePixelsMove(x, y);
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