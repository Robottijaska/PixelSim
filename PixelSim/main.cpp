#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
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

SDL_Event gCurrentEvent;
bool mouseDown;

SDL_Point mPosition = SDL_Point();

#include "Pixel.h"
#include "Graphics.h"

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

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
                else {
                    
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

        //The frames per second timer
        LTimer fpsTimer;

        //The frames per second cap timer
        LTimer capTimer;

        //Start counting frames per second
        int countedFrames = 0;
        fpsTimer.start();
        while (!quit)
        {
            //Start cap timer
            capTimer.start();

            //Handle events on queue
            while (SDL_PollEvent(&gCurrentEvent) != 0)
            {
                //User requests quit
                if (gCurrentEvent.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
            mouseDown = SDL_GetMouseState(&mPosition.x, &mPosition.y);
            int closestPixel = Pxl::GetClosestPixel(mPosition);

            Pxl::UpdatePixels(&gCurrentEvent, closestPixel);

            //Calculate and correct fps
            /*float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
            if (avgFPS > 2000000)
            {
                avgFPS = 0;
            }*/

            //std::cout << "Average Frames Per Second (With Cap) " << avgFPS << std::endl;

            //Clear screen
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);

            //Draw screen
            Gfx::LoadPixels(closestPixel);

            //capping refresh rate stuff
            SDL_RenderPresent(gRenderer);
            //countedFrames++;

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