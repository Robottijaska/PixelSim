#pragma once

#include <SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <SDL_image.h>
#include <iostream>

int clampInt(int input, int min, int max) {
    return input > max ? input = max : (input < min ? input = min : input);
}
void clampInt(int* input, int min, int max) {
    *input = *input > max ? *input = max : (*input < min ? *input = min : *input);
}

struct Vector2 {
    double x = 0;
    double y = 0;
};

//distance between two points
float findGLen(SDL_Point p1, SDL_Point p2) {
    return (float)sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

SDL_Point findClosestPxl(SDL_Point p1) {
    return SDL_Point{ (int)floor(p1.x / PIXEL_SIZE), (int)floor(p1.y / PIXEL_SIZE) };
}

//code borrowed from LazyFoo
class LTimer
{
public:
    //Initializes variables
    LTimer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    Uint32 getTicks();

    //Checks the status of the timer
    bool isStarted();
    bool isPaused();

private:
    //The clock time when the timer started
    Uint32 mStartTicks;

    //The ticks stored when the timer was paused
    Uint32 mPausedTicks;

    //The timer status
    bool mPaused;
    bool mStarted;
};

LTimer::LTimer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

    //Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if (mStarted && !mPaused)
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if (mStarted && mPaused)
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks()
{
    //The actual timer time
    Uint32 time = 0;

    //If the timer is running
    if (mStarted)
    {
        //If the timer is paused
        if (mPaused)
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
    //Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
    //Timer is running and paused
    return mPaused && mStarted;
}

bool isInsideRect(SDL_Rect Zone, SDL_Point Point) {
    return (Point.x >= Zone.x && Point.x <= Zone.x + Zone.w) && (Point.y >= Zone.y && Point.y <= Zone.y + Zone.h);
}