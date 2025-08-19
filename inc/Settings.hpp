#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <Brick.hpp>
#include <SFML/Graphics.hpp>

namespace ProceduralMaze::Settings {

const sf::Vector2u DISPLAY_SIZE{ 1920, 1024 };

// MAP_GRID_OFFSET and MAP_GRID_SIZE are in blocks, not pixels
const sf::Vector2f MAP_GRID_OFFSET{ 10.f,2.f };  
const sf::Vector2u MAP_GRID_SIZE{ 120u, 54u }; 
const sf::Vector2f PLAYER_START_POS{ 20, static_cast<float>(DISPLAY_SIZE.y) / 2 };

// SFML views
const sf::Vector2f LOCAL_MAP_VIEW_SIZE{ 300.f, 200.f };
const sf::Vector2f MINI_MAP_VIEW_SIZE{ DISPLAY_SIZE.x * 0.25f, DISPLAY_SIZE.y * 0.25f };

}

#endif // __SETTINGS_HPP__