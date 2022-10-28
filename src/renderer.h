#ifndef RENDERER_H
#define RENDERER_H

#include "SDL.h"
#include "snake.h"
#include <vector>

class Renderer {
public:
    Renderer(const std::size_t screen_width, const std::size_t screen_height,
        const std::size_t grid_width, const std::size_t grid_height);
    ~Renderer();

    void Render(Snake const& snake, SDL_Point const& food,
        std::vector<SDL_Point> walls);
    void UpdateWindowTitle(int score, int fps);

private:
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sprites;

    const std::size_t screen_width;
    const std::size_t screen_height;
    const std::size_t grid_width;
    const std::size_t grid_height;

    static const auto Food = 0;
    static const auto Tail = 1;
    static const auto Turn = 2;
    static const auto DeadHead = 3;
    static const auto MouthOpenHead = 4;
    static const auto Straight = 5;
    static const auto Head = 6;

    bool EqualCell(SDL_Point const& cell1, SDL_Point const& cell2);
    bool NextToFood(Snake const& snake, SDL_Point const& food);
    int tailDirection(SDL_Point const& tail, SDL_Point const& lastBodyCell);
    int turnDirection(SDL_Point const& currentCell,
        SDL_Point const& previousCell, SDL_Point const& nextCell);
    int bodyDirectionAndSource(SDL_Point const& currentCell,
        SDL_Point const& previousCell, SDL_Point const& nextCell,
        int& srcIndex);
};

#endif
