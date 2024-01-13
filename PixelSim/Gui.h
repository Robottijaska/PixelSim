#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>

#include "Graphics.h"
#include "Pixel.h"

namespace Gui {

	bool mouseInGui = false;

	class GuiButton {
	public:
		//X and Y are offsets of the Gui's X and Y

		int XOffSet = 0;

		int YOffSet = 0;

		SDL_Color Color = SDL_Color{ 255,255,255,255 };

		bool useTexture = true;

		SDL_Texture* Texture = nullptr;

		SDL_Rect Zone = SDL_Rect{ 0,0,0,0 };

		virtual void Update(bool mouseClick, double deltaTime) {}

		virtual void Draw() {};

		~GuiButton() {
			SDL_DestroyTexture(Texture);
			Texture = nullptr;
		}

	};

	class GuiMap {
	public:
		std::vector<std::unique_ptr<GuiButton>> Buttons;

		SDL_Texture* Texture = nullptr;

		SDL_Color Color = SDL_Color{ 255,255,255,255 };

		bool useTexture = true;

		virtual void Update(bool mouseClick, double deltaTime) {}

		~GuiMap() {
			SDL_DestroyTexture(Texture);
			Texture = nullptr;
		}
	};

	struct BaseGUI {

		//Functions as both the clickbox and the back texture
		SDL_Rect Zone = SDL_Rect{ 0,0,0,0 };

		std::vector<std::unique_ptr<GuiMap>> Maps;

		int currentMap = 0;

		virtual void Update(bool mouseClick, double deltaTime) {}
	};

	std::vector<std::unique_ptr<BaseGUI>> Guis;

	namespace Types {
		//CLASS: PIXELPICKER;
		class PixelPicker : public BaseGUI {
		private:

			//Screens
			class MainScreen : public GuiMap {
			private:
				struct CloseButton : public GuiButton {

					MainScreen* Parent;

					CloseButton(MainScreen* parent) {
						Parent = parent;

						//makes invisible
						useTexture = true;
						Texture = Gfx::loadTexture("res/textures/gui/close.png");
						Zone = SDL_Rect{ Parent->Parent->Zone.x + Parent->Parent->Zone.w - 7 * 4, Parent->Parent->Zone.y, 7 * 4, 7 * 4 };
					}

					~CloseButton() {
					}

					void Update(bool mouseClick, double deltaTime) {
						//if clicked
						if (isInsideRect(Zone, mPosition) && mouseClick) {
							mouseInGui = true;

							//closes gui
							Guis.erase(Guis.end() - 1);
						}
					}
				};

				struct PixelButton : public GuiButton {

					MainScreen* Parent;

					Pxl::Pixel Pixel;

					SDL_Color TtfColor;

					PixelButton(MainScreen* parent, Pxl::Pixel pixel, SDL_Point offset) {
						Parent = parent;
						Pixel = pixel;

						useTexture = false;
						Color = pixel.Color;

						float avgColor = (float)((int)pixel.Color.r + (int)pixel.Color.b + (int)pixel.Color.g) / 3;
						avgColor = 255 - avgColor;

						TtfColor = SDL_Color{ (Uint8)avgColor, (Uint8)avgColor, (Uint8)avgColor };

						Zone = SDL_Rect{ offset.x + Parent->Parent->Zone.x, offset.y + Parent->Parent->Zone.y, 7 * 8, 7 * 8 };
					}

					void TtfName() {
						std::string display = Pixel.Name;
						SDL_Color textColor = TtfColor;
						SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, display.c_str(), textColor);
						SDL_Texture* text = SDL_CreateTextureFromSurface(gRenderer, textSurface);
						int text_width = (int)((double)textSurface->w * ((double)Zone.w / (double)textSurface->w));
						int text_height = (int)(((double)textSurface->h / (double)textSurface->w) * (double)text_width);
						SDL_FreeSurface(textSurface);
						SDL_Rect renderQuad = { Zone.x + (28 - text_width / 2), Zone.y + (28 - text_height / 2), text_width, text_height };
						SDL_RenderCopy(gRenderer, text, NULL, &renderQuad);
						SDL_DestroyTexture(text);
					}

					void Draw() {
						TtfName();
					}

					void Update(bool mouseClick, double deltaTime) {
						//if clicked
						if (isInsideRect(Zone, mPosition) && mouseClick) {
							mouseInGui = true;

							Pxl::selectedPixel = Pixel;

							//closes gui
							Guis.erase(Guis.end() - 1);
						}
					}
				};

				PixelPicker* Parent;

			public:
				MainScreen(PixelPicker* parent) {
					Parent = parent;

					useTexture = false;
					Color = SDL_Color{ 150,150,150 };

					Buttons.push_back(std::make_unique<CloseButton>(this));

					//pixels
					Buttons.push_back(std::make_unique<PixelButton>(this, Pxl::PixelTypes[Pxl::types::WATER], SDL_Point{ 28,28 }));
					Buttons.push_back(std::make_unique<PixelButton>(this, Pxl::PixelTypes[Pxl::types::SAND], SDL_Point{ 28 * 4,28 }));
					Buttons.push_back(std::make_unique<PixelButton>(this, Pxl::PixelTypes[Pxl::types::STONE], SDL_Point{ 28 * 7,28 }));
				}

				~MainScreen() {
				}

				void Update(bool mouseClick, double deltaTime) {
					for (int i = 0; i < Buttons.size(); i++) {
						Buttons[i]->Update(mouseClick, deltaTime);
					}
				}
			};

		public:
			PixelPicker(SDL_Point startPos) {
				Zone = SDL_Rect{ startPos.x, startPos.y, 500, 250 };

				Maps.push_back(std::make_unique<MainScreen>(this));

				Update(false, 0);
			}

			~PixelPicker() {
			}

			void Update(bool mouseClick, double deltaTime) {
				Maps[currentMap]->Update(mouseClick, deltaTime);
			}
		};

		//CLASS: MENU; dropdown menu should always be on screen
		class Menu : public BaseGUI {
		private:
			//Screens
			class MainScreen : public GuiMap {
			private:
				struct ClearAllButton : public GuiButton {
					MainScreen* Parent;

					ClearAllButton(MainScreen* parent) {
						Parent = parent;

						XOffSet = 0;
						YOffSet = 0;

						//makes invisible
						useTexture = true;
						Texture = NULL;
						Zone = SDL_Rect{ 0,0,68,68 };
					}

					~ClearAllButton() {
						Parent = nullptr;
					}

					void Update(bool mouseClick, double deltaTime) {
						Zone.x = Parent->Parent->Zone.x + XOffSet;
						Zone.y = Parent->Parent->Zone.y + YOffSet;

						//if clicked
						if (isInsideRect(Zone, mPosition) && mouseClick) {
							mouseInGui = true;

							for (int i = 0; i < Pxl::PIXELGRID_SIZE; i++) {
								if (Pxl::GetPixel(i).Name != "Vacuum") {
									Pxl::SetPixel( i, Pxl::PixelTypes[Pxl::types::VACUUM]);
								}
							}
						}
					}
				};

				struct ColorPickerButton : public GuiButton {
					MainScreen* Parent;

					ColorPickerButton(MainScreen* parent) {
						Parent = parent;

						XOffSet = 0;
						YOffSet = 68;

						//makes invisible
						useTexture = true;
						Texture = NULL;
						Zone = SDL_Rect{ 0,0,68,68 };
					}

					~ColorPickerButton() {
						Parent = nullptr;
					}

					void Update(bool mouseClick, double deltaTime) {
						Zone.x = Parent->Parent->Zone.x + XOffSet;
						Zone.y = Parent->Parent->Zone.y + YOffSet;

						//if clicked
						if (isInsideRect(Zone, mPosition) && mouseClick) {
							mouseInGui = true;
							Guis.push_back(std::make_unique<PixelPicker>(SDL_Point{ 100,100 }));
						}
					}
				};

				struct DropdownButton : public GuiButton {
					MainScreen* Parent;

					bool isExtended = false;
					bool isMoving = false;

					int startY;
					int endY;

					DropdownButton(MainScreen* parent) {
						Parent = parent;

						XOffSet = 0;
						YOffSet = 68*2;

						startY = Parent->Parent->Zone.y;
						endY = startY + Parent->Parent->Zone.h - 68;

						Color.a = 0;
						useTexture = true;
						Texture = NULL;
						Zone = SDL_Rect{ 0,0,68,68 };
					}

					~DropdownButton() {
						Parent = nullptr;
					}

					void Update(bool mouseClick, double deltaTime) {
						Zone.x = Parent->Parent->Zone.x + XOffSet;
						Zone.y = Parent->Parent->Zone.y + YOffSet;

						//if clicked
						if (isInsideRect(Zone, mPosition) && mouseClick) {
							mouseInGui = true;
							//checks whether the gui is between move cycles
							if (!isMoving) {
								isMoving = true;
								Parent->velocityY = 0;
							}
						}

						if (isMoving) {
							if (isExtended) {
								if (Parent->Parent->Zone.y - 10 < startY) {
									Parent->Parent->Zone.y = startY;
									Parent->velocityY = 0;
									isExtended = false;
									isMoving = false;
								}
								else {
									Parent->velocityY = -10;
								}
							}
							else {
								if (Parent->Parent->Zone.y + 10 > endY) {
									Parent->Parent->Zone.y = endY;
									Parent->velocityY = 0;
									isExtended = true;
									isMoving = false;
								}
								else {
									Parent->velocityY = 10;
								}
							}
						}
					}
				};

				Menu* Parent;

			public:
				double velocityY = 0;
				MainScreen(Menu* parent) {
					Parent = parent;

					//Initializing Buttons
					Buttons.push_back(std::make_unique<DropdownButton>(this));
					Buttons.push_back(std::make_unique<ColorPickerButton>(this));
					Buttons.push_back(std::make_unique<ClearAllButton>(this));

					useTexture = true;
					Texture = Gfx::loadTexture("res/textures/gui/menu.png");
				}

				~MainScreen() {
					delete Parent;
					Parent = nullptr;
				}

				void Update(bool mouseClick, double deltaTime) {
					for (int i = 0; i < Buttons.size(); i++) {
						Buttons[i]->Update(mouseClick, deltaTime);
					}
					Parent->Zone.y += (int)(velocityY * deltaTime);
				}
			};
		public:
			Menu(SDL_Point startPos) {
				Zone = SDL_Rect{ startPos.x, startPos.y, 68, 68 * 3 };

				Maps.push_back(std::make_unique<MainScreen>(this));

				Update(false, 0);
			}

			void Update(bool mouseClick, double deltaTime) {
				Maps[currentMap]->Update(mouseClick, deltaTime);
			}
		};
	}

	/*
	returns:
	* 0 if pixels should NOT be updated and mouse recognition should NOT be available
	* 1 if pixels should be updated and mouse recognition should NOT be available
	* 2 if pixels should be updated and mouse recognition should be available
	*/
	int UpdateGuis(bool mouseClick, double deltaTime) {
		int output;
		bool shouldPixelsUpdate = Guis.size() < 2;
		bool shouldMouseRecognition = !isInsideRect((Guis[Guis.size() - 1]->Zone), mPosition) && shouldPixelsUpdate;
		if (!shouldPixelsUpdate && !shouldMouseRecognition) { output = 0; }
		else if (shouldPixelsUpdate && !shouldMouseRecognition) { output = 1; }
		else { output = 2; }
		if ((mPosition.y < 0 || mPosition.y > SCREEN_HEIGHT) || (mPosition.x < 0 || mPosition.x > SCREEN_WIDTH) && output == 2) { output = 1; }

		Guis[Guis.size() - 1]->Update(mouseClick, deltaTime);
		return output;
	}

	void DrawGuis() {
		for (int i = (int)Guis.size() - 1; i > -1; i--) {
			GuiMap& NowMap = *Guis[i]->Maps[Guis[i]->currentMap];
			if (NowMap.useTexture) {
				SDL_Color currentColor = NowMap.Color;
				SDL_SetRenderDrawColor(gRenderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
				SDL_RenderCopy(gRenderer, NowMap.Texture, NULL, &Guis[i]->Zone);
			}
			else {
				SDL_Color currentColor = NowMap.Color;
				SDL_SetRenderDrawColor(gRenderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
				SDL_RenderFillRect(gRenderer, &Guis[i]->Zone);
			}

			for (int j = 0; j < NowMap.Buttons.size(); j++) {
				GuiButton& NowButton = *NowMap.Buttons[j];
				if (NowButton.useTexture) {
					SDL_Color currentColor = NowButton.Color;
					SDL_SetRenderDrawColor(gRenderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
					SDL_RenderCopy(gRenderer, NowButton.Texture, NULL, &NowButton.Zone);
				}
				else {
					SDL_Color currentColor = NowButton.Color;
					SDL_SetRenderDrawColor(gRenderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
					SDL_RenderFillRect(gRenderer, &NowButton.Zone);
				}
				NowButton.Draw();
			}
		}
	}
}