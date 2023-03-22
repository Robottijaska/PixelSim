#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <sstream>

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Screen dimension constants
const int SCREEN_WIDTH = 640 * 2;
const int SCREEN_HEIGHT = 480 * 2;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

const int PIXEL_SIZE = 8 * 2;

const int screenWidthPxl = SCREEN_WIDTH / PIXEL_SIZE;
const int screenHeightPxl = SCREEN_HEIGHT / PIXEL_SIZE;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

//Global font
TTF_Font* gFont = NULL;

SDL_Event gCurrentEvent;

SDL_Point mPosition = SDL_Point();
SDL_Point mPositionOld = SDL_Point();

Uint32 mButton = NULL;

#include "Pixel.h"
#include "Graphics.h"
#include "Gui.h"

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow("PixelSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Initialize SDL_ttf
            if (TTF_Init() == -1)
            {
                printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                success = false;
            }
            else {
                //Create renderer for window
                gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
                if (gRenderer == NULL)
                {
                    printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                    success = false;
                }
                else
                {
                    //Initialize renderer color
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);



                    //Load the font file
                    gFont = TTF_OpenFont("calibri.ttf", 32);
                    if (gFont == NULL)
                    {
                        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
                        success = false;
                    }

                    //Initialize PNG loading
                    int imgFlags = IMG_INIT_PNG;
                    if (!(IMG_Init(imgFlags) & imgFlags))
                    {
                        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                        success = false;
                    }

                    for (int x = 0; x < Pxl::PIXELGRID_WIDTH; x++) {
                        for (int y = 0; y < Pxl::PIXELGRID_HEIGHT; y++) {

                            Pxl::Pixels[x][y] = Pxl::GetPixelType(Pxl::types::VACUUM);

                        }
                    }

                    //adding menu gui
                    Gui::Guis.push_back(std::make_unique<Gui::Types::Menu>(SDL_Point{ SCREEN_WIDTH - 17 * 4, -68 * 2 }));
                }
            }
        }
    }

    return success;
}

//deallocates global ptrs
void close()
{
    //Free loaded image
    SDL_DestroyTexture(gTexture);
    gTexture = nullptr;

    //Destroy window    
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = nullptr;

    //free additional pointers

    //Quit SDL subsystems
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        bool quit = false;
        bool continueMouse = false;
        bool isPaused = false;

        //The frames per second timer
        LTimer fpsTimer;

        //The frames per second cap timer
        LTimer capTimer;

        //Start counting frames per second
        int countedFrames = 0;
        fpsTimer.start();
        SDL_Point closestPixel = SDL_Point{ 0, 0 };

        Uint64 ticksNow = SDL_GetPerformanceCounter();
        Uint64 ticksLast = 0;

        Uint32 mTicksCount = 0;

        //pause button's texture settings
        SDL_Texture* pauseTexture = Gfx::loadTexture("x64/Gfx/Pause.png");
        SDL_SetTextureBlendMode(pauseTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(pauseTexture, 100);

        while (!quit)
        {
            //Start cap timer
            capTimer.start();

            bool mouseClick = false;
            //Handle events on queue
            while (SDL_PollEvent(&gCurrentEvent) != 0)
            {
                switch (gCurrentEvent.type) {

                    case SDL_QUIT:
                        quit = true;
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        mouseClick = !Gui::mouseInGui;
                        break;
                    case SDL_MOUSEBUTTONUP:
                        Gui::mouseInGui = false;
                        break;
                    case SDL_KEYDOWN:
                        switch (gCurrentEvent.key.keysym.sym) {
                            case SDLK_F1:
                                isPaused = !isPaused;
                                break;
                        }
                }
            }

            mPositionOld = mPosition;
            mButton = SDL_GetMouseState(&mPosition.x, &mPosition.y);
            clampInt(&mPosition.x, -1, SCREEN_WIDTH);
            clampInt(&mPosition.y, -1, SCREEN_HEIGHT);

            //Calculate fps
            float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);

            //Calculate deltaTime 
            ticksLast = ticksNow;
            ticksNow = SDL_GetPerformanceCounter();
            double deltaTime = deltaTime = ((ticksNow - ticksLast) * 60 / (double)SDL_GetPerformanceFrequency());

            //---UPDATE---
            /* 0 if pixels should NOT be updated and mouse recognition should NOT be available
            * 1 if pixels should be updated and mouse recognition should NOT be available
            * 2 if pixels should be updated and mouse recognition should be available*/
            int pxlState = 2;

            pxlState = Gui::UpdateGuis(mouseClick, deltaTime);

            if (pxlState != 0 && Gui::mouseInGui == false) {
                if (pxlState != 1) { closestPixel = SDL_Point{ (int)floor(mPosition.x / PIXEL_SIZE), (int)floor(mPosition.y / PIXEL_SIZE) }; }
                Pxl::UpdatePixels(isPaused, mPosition, mPositionOld, mButton, mouseClick, pxlState);
            }

            //---DRAW---
            //Clear screen
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);

            //Draw screen
            Pxl::LoadPixels(closestPixel, pxlState);
            if (pxlState != 0) { Gfx::DrawCoordinates(closestPixel); }
            Gfx::DrawFPScounter(avgFPS);
            Gui::DrawGuis();

            if (isPaused) {
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 100);
                SDL_RenderCopy(gRenderer, pauseTexture, NULL, new SDL_Rect{SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT / 2 - 200, 400, 400});
            }

            //capping refresh rate stuff
            SDL_RenderPresent(gRenderer);
            countedFrames++;

            //If frame finished early
            int frameTicks = capTimer.getTicks();
            if (frameTicks < SCREEN_TICKS_PER_FRAME)
            {
                //Wait remaining time
                SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
            }
        }
    }

    //deallocating ptrs
    close();

    return 0;
}