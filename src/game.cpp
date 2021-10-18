#include <iostream>
#include <string>
#include <ctime>

#include "SDL2/SDL.h"
#include "game.hpp"

using namespace std;

Game::Game()
{
    for (int i = 0; i < GRID_WIDTH; ++i)
        for (int j = 0; j < GRID_HEIGHT; ++j)
        {
            grid[i][j] = Block::empty;
        }

    srand(static_cast<unsigned int>(time(0)));
}

bool Game::Run()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    // Create Window
    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    alive = true;
    running = true;
    GrowBody(SIZE_AT_BEGINNING); // create a snake with a specific size
    ReplaceFood();
    return GameLoop();
}

void Game::ReplaceFood()
{
    int x, y;
    while (true)
    {
        x = rand() % GRID_WIDTH;
        y = rand() % GRID_HEIGHT;

        if (grid[x][y] == Block::empty)
        {
            grid[x][y] = Block::food;
            food.x = x;
            food.y = y;
            break;
        }
    }
}

bool Game::GameLoop()
{
    Uint32 before, second = SDL_GetTicks(), after;
    int frame_time, frames = 0;
    bool nextgame = false;

    while (running)
    {
        before = SDL_GetTicks();

        PollEvents();
        Update();
        Render();

        frames++;
        after = SDL_GetTicks();
        frame_time = after - before;

        if (after - second >= 1000)
        {
            fps = frames;
            frames = 0;
            second = after;
            UpdateWindowTitle();
        }

        if (FRAME_RATE > frame_time)
        {
            SDL_Delay(FRAME_RATE - frame_time);
        }
        nextgame = TryAgain();
    }
    return nextgame;
}

void Game::PollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            running = false;
        }
        else if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
            case SDLK_UP:
                if (last_dir != Move::down || size == 1)
                    dir = Move::up;
                break;

            case SDLK_DOWN:
                if (last_dir != Move::up || size == 1)
                    dir = Move::down;
                break;

            case SDLK_LEFT:
                if (last_dir != Move::right || size == 1)
                    dir = Move::left;
                break;

            case SDLK_RIGHT:
                if (last_dir != Move::left || size == 1)
                    dir = Move::right;
                break;
            }
        }
    }
}

int Game::GetSize()
{
    return size + SIZE_AT_BEGINNING;
}

void Game::GrowBody(int quantity)
{
    growing += quantity;
}

void Game::Update()
{
    if (!alive)
        return;

    switch (dir)
    {
    case Move::up:
        pos.y -= speed;
        pos.x = floorf(pos.x);
        break;

    case Move::down:
        pos.y += speed;
        pos.x = floorf(pos.x);
        break;

    case Move::left:
        pos.x -= speed;
        pos.y = floorf(pos.y);
        break;

    case Move::right:
        pos.x += speed;
        pos.y = floorf(pos.y);
        break;
    }

    // Wrap
    /*
    if (pos.x < 0) pos.x = GRID_WIDTH - 1;
    else if (pos.x > GRID_WIDTH - 1) pos.x = 0;

    if (pos.y < 0) pos.y = GRID_HEIGHT - 1;
    else if (pos.y > GRID_HEIGHT - 1) pos.y = 0;
    */

    // kill if the head touches the border of the screen
    if (pos.x < 0) alive = false;
    else if (pos.x > GRID_WIDTH - 1) alive = false;

    if (pos.y < 0) alive = false;
    else if (pos.y > GRID_HEIGHT - 1) alive = false;

    int new_x = static_cast<int>(pos.x);
    int new_y = static_cast<int>(pos.y);

    // Check if head position has changed
    if (new_x != head.x || new_y != head.y)
    {
        last_dir = dir;

        // If we are growing, just make a new neck
        if (growing > 0)
        {
            size++;
            body.push_back(head);
            growing--;
            grid[head.x][head.y] = Block::body;
        }
        else
        {
            // We need to shift the body
            SDL_Point free = head;
            vector<SDL_Point>::reverse_iterator rit = body.rbegin();
            for (; rit != body.rend(); ++rit)
            {
                grid[free.x][free.y] = Block::body;
                swap(*rit, free);
            }

            grid[free.x][free.y] = Block::empty;
        }

    }

    head.x = new_x;
    head.y = new_y;

    Block& next = grid[head.x][head.y];
    // Check if there's food over here
    if (next == Block::food)
    {
        score++;
        ReplaceFood();
        GrowBody(1);
    }
    // Check if we're dead
    else if (next == Block::body)
    {
        alive = false;
    }

    next = Block::head;
}

int Game::GetScore()
{
    return score;
}

void Game::UpdateWindowTitle()
{
    string title = "Snakle++ Score: " + to_string(score); // + " FPS: " + to_string(fps);
    SDL_SetWindowTitle(window, title.c_str());
}

void Game::Render()
{
    SDL_Rect block;
    block.w = SCREEN_WIDTH / GRID_WIDTH;
    block.h = SCREEN_WIDTH / GRID_HEIGHT;

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Render food
    SDL_SetRenderDrawColor(renderer, 239, 19, 19, 255);
    block.x = food.x * block.w;
    block.y = food.y * block.h;
    SDL_RenderFillRect(renderer, &block);

    // Render snake's body
    SDL_SetRenderDrawColor(renderer, 32, 148, 0, 255);
    for (SDL_Point& point : body)
    {
        block.x = point.x * block.w;
        block.y = point.y * block.h;
        SDL_RenderFillRect(renderer, &block);
    }

    // Render snake's head
    block.x = head.x * block.w;
    block.y = head.y * block.h;
    if (alive) SDL_SetRenderDrawColor(renderer, 160, 83, 30, 255);
    else       SDL_SetRenderDrawColor(renderer, 93, 38, 130, 255);
    SDL_RenderFillRect(renderer, &block);

    // Update Screen
    SDL_RenderPresent(renderer);
}

void Game::Close()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Game::TryAgain()
{
    if (alive == false)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
                return false;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_KP_ENTER:
                    running = false;
                    return true;
                }
            }
        }
    }
    return false;
}
