#include <algorithm>
#include <iostream>
#include <string>

#include "sdl_check.h"

#include "renderer.h"

Renderer::Renderer(const std::size_t screen_width,
    const std::size_t screen_height, const std::size_t grid_width,
    const std::size_t grid_height)
    : screen_width(screen_width)
    , screen_height(screen_height)
    , grid_width(grid_width)
    , grid_height(grid_height)
{
    // Initialize SDL
    auto res = SDL_Init(SDL_INIT_VIDEO);
    SDL_CHECK(res == 0, "SDL_Init");

    // Create Window
    sdl_window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    SDL_CHECK(sdl_window, "SDL_CreateWindow");

    // Create renderer
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_CHECK(sdl_renderer, "SDL_CreateRenderer");

    // Create sprites
    auto surface = SDL_LoadBMP("../sprites.bmp");
    SDL_CHECK(surface, "SDL_LoadBMP(\"sprites.bmp\")");
    sprites = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_CHECK(sprites, "SDL_CreateTextureFromSurface");
    SDL_FreeSurface(surface);
}

Renderer::~Renderer()
{
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

bool Renderer::equalCell(SDL_Point const& cell1, SDL_Point const& cell2)
{
    return ((cell1.x + grid_width) % grid_width
               == (cell2.x + grid_width) % grid_width)
        && ((cell1.y + grid_height) % grid_height
            == (cell2.y + grid_height) % grid_height);
}

bool Renderer::foodIsNext(Snake const& snake, SDL_Point const& food)
{
    int head_x = static_cast<int>(snake.head_x);
    int head_y = static_cast<int>(snake.head_y);

    switch (snake.direction) {
    case Snake::Direction::kRight:
        return equalCell(SDL_Point { head_x + 1, head_y }, food);
    case Snake::Direction::kLeft:
        return equalCell(SDL_Point { head_x - 1, head_y }, food);
    case Snake::Direction::kDown:
        return equalCell(SDL_Point { head_x, head_y + 1 }, food);
    case Snake::Direction::kUp:
        return equalCell(SDL_Point { head_x, head_y - 1 }, food);
    default:
        return false;
    }
}

int Renderer::tailDirection(
    SDL_Point const& tail, SDL_Point const& lastBodyCell)
{
    int ds[][3] = {
        { 1, 0, 0 },
        { 0, 1, 1 },
        { -1, 0, 2 },
        { 0, -1, 3 },
    };

    auto result = std::find_if(
        std::begin(ds), std::end(ds), [&tail, &lastBodyCell, this](auto d) {
            return this->equalCell(
                SDL_Point { tail.x + d[0], tail.y + d[1] }, lastBodyCell);
        });

    if (result != std::end(ds)) {
        return (*result)[2];
    }
    return 0;
}

int Renderer::turnDirection(SDL_Point const& currentCell,
    SDL_Point const& previousCell, SDL_Point const& nextCell)
{
    bool up = false;
    if (equalCell(SDL_Point { currentCell.x, currentCell.y - 1 },
            previousCell)) {
        up = true;
    }
    if (equalCell(SDL_Point { currentCell.x, currentCell.y - 1 },
            nextCell)) {
        up = true;
    }
    bool right = false;
    if (equalCell(SDL_Point { currentCell.x + 1, currentCell.y },
            previousCell)) {
        right = true;
    }
    if (equalCell(SDL_Point { currentCell.x + 1, currentCell.y },
            nextCell)) {
        right = true;
    }
    bool down = false;
    if (equalCell(SDL_Point { currentCell.x, currentCell.y + 1 },
            previousCell)) {
        down = true;
    }
    if (equalCell(SDL_Point { currentCell.x, currentCell.y + 1 },
            nextCell)) {
        down = true;
    }
    bool left = false;
    if (equalCell(SDL_Point { currentCell.x - 1, currentCell.y },
            previousCell)) {
        left = true;
    }
    if (equalCell(SDL_Point { currentCell.x - 1, currentCell.y },
            nextCell)) {
        left = true;
    }
    if (up && right) {
        return 0;
    } else if (right && down) {
        return 1;
    } else if (down && left) {
        return 2;
    } else if (left && up) {
        return 3;
    }
    return 0;
}

int Renderer::bodyDirectionAndSource(SDL_Point const& currentCell,
    SDL_Point const& previousCell, SDL_Point const& nextCell,
    int& srcIndex)
{
    if (nextCell.x == previousCell.x) {
        srcIndex = Straight;
        return 1;
    }

    if (nextCell.y == previousCell.y) {
        srcIndex = Straight;
        return 0;
    }

    srcIndex = Turn;
    return turnDirection(currentCell, previousCell, nextCell);
}

void Renderer::Render(Snake const& snake, SDL_Point const& food)
{
    SDL_Rect block;
    SDL_Rect src;
    block.w = screen_width / grid_width;
    block.h = screen_height / grid_height;

    src.x = 0;
    src.y = 0;
    src.w = screen_width / grid_width;
    src.h = screen_height / grid_height;

    // Clear screen
    SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
    SDL_RenderClear(sdl_renderer);

    // Render food
    src.x = Food * src.w;
    block.x = food.x * block.w;
    block.y = food.y * block.h;
    SDL_RenderCopyEx(
        sdl_renderer, sprites, &src, &block, 0, nullptr, SDL_FLIP_NONE);

    // Render snake's body
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (auto iter = std::begin(snake.body); iter != std::end(snake.body);
         iter++) {
        block.x = iter->x * block.w;
        block.y = iter->y * block.h;
        int direction = 0;

        if (iter == std::begin(snake.body)) {
            src.x = Tail * src.w;
            direction = tailDirection(*iter, *(iter + 1));
        } else {
            int sourceIndex = 0;
            auto nextCell = *(iter + 1);
            if (iter + 1 == std::end(snake.body)) {
                nextCell.x = static_cast<int>(snake.head_x);
                nextCell.y = static_cast<int>(snake.head_y);
            }

            direction = bodyDirectionAndSource(
                *iter, *(iter - 1), nextCell, sourceIndex);
            src.x = sourceIndex * src.w;
        }

        SDL_RenderCopyEx(sdl_renderer, sprites, &src, &block, direction * 90,
            nullptr, SDL_FLIP_NONE);
    }

    // Render snake's head
    block.x = static_cast<int>(snake.head_x) * block.w;
    block.y = static_cast<int>(snake.head_y) * block.h;
    if (!snake.alive) {
        src.x = DeadHead * src.w;
    } else if (foodIsNext(snake, food)) {
        src.x = MouthOpenHead * src.w;
    } else {
        src.x = Head * src.w;
    }
    SDL_RenderCopyEx(sdl_renderer, sprites, &src, &block,
        static_cast<int>(snake.direction) * 90, nullptr, SDL_FLIP_NONE);

    // Update Screen
    SDL_RenderPresent(sdl_renderer);
}

void Renderer::UpdateWindowTitle(int score, int fps)
{
    std::string title { "Snake Score: " + std::to_string(score)
        + " FPS: " + std::to_string(fps) };
    SDL_SetWindowTitle(sdl_window, title.c_str());
}
