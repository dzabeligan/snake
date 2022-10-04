#include <algorithm>
#include <cmath>
#include <iostream>

#include "snake.h"

/**
 * @brief Update snake head and body
 *
 */
void Snake::Update()
{
    // We first capture the head's cell before updating.
    SDL_Point prev_cell { static_cast<int>(head_x), static_cast<int>(head_y) };

    UpdateHead();

    // Capture the head's cell after updating.
    SDL_Point current_cell { static_cast<int>(head_x),
        static_cast<int>(head_y) };

    // Update all of the body vector items if the snake head has moved to a new
    // cell.
    if (current_cell.x != prev_cell.x || current_cell.y != prev_cell.y) {
        UpdateBody(current_cell, prev_cell);
    }
}

/**
 * @brief Update snake head
 *
 */
void Snake::UpdateHead()
{
    switch (direction) {
    case Direction::kUp:
        head_y -= speed;
        break;

    case Direction::kDown:
        head_y += speed;
        break;

    case Direction::kLeft:
        head_x -= speed;
        break;

    case Direction::kRight:
        head_x += speed;
        break;
    }

    // Wrap the Snake around to the beginning if going off of the screen.
    head_x = fmod(head_x + grid_width, grid_width);
    head_y = fmod(head_y + grid_height, grid_height);
}

/**
 * @brief Update snake body
 *
 * @param current_head_cell
 * @param prev_head_cell
 */
void Snake::UpdateBody(SDL_Point& current_head_cell, SDL_Point& prev_head_cell)
{
    // Add previous head location to vector
    body.emplace_back(prev_head_cell);

    if (!growing) {
        // Remove the tail from the vector.
        body.erase(body.begin());
    } else {
        growing = false;
        size++;
    }

    if (std::any_of(body.cbegin(), body.cend(),
            [&current_head_cell](const SDL_Point& point) {
                return current_head_cell.x == point.x
                    && current_head_cell.y == point.y;
            })) {
        alive = false;
    }
}

/**
 * @brief Grow snake body
 *
 */
void Snake::GrowBody() { growing = true; }

/**
 * @brief Check is cell is occupied by snake
 *
 * @param x
 * @param y
 * @return true
 * @return false
 */
bool Snake::SnakeCell(int x, int y)
{
    if (x == static_cast<int>(head_x) && y == static_cast<int>(head_y)) {
        return true;
    }
    if (std::any_of(body.cbegin(), body.cend(), [x, y](const SDL_Point& point) {
            return x == point.x && y == point.y;
        })) {
        return true;
    }
    return false;
}
