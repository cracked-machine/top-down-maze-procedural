#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace ProceduralMaze::Settings {

const sf::Vector2u DISPLAY_SIZE{ 1920, 1024 };

// MAP_GRID_OFFSET and MAP_GRID_SIZE are in blocks, not pixels
const sf::Vector2f MAP_GRID_OFFSET{ 10.f,1.f };  
const sf::Vector2u MAP_GRID_SIZE{ 100u, 61u }; 

// SFML views


}

#endif // __SETTINGS_HPP__