#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <SFML/Graphics.hpp>

namespace ProceduralMaze::Settings {

const sf::Vector2u DISPLAY_SIZE{1920, 1024};
const sf::Vector2u MAP_GRID_SIZE{120u,69u};     // blocks not pixels
const sf::Vector2f MAP_GRID_OFFSET{ 10.f,2.f};  // blocks not pixels
const sf::Vector2f PLAYER_START_POS{
    20, 
    static_cast<float>(DISPLAY_SIZE.y) / 2
};


}

#endif // __SETTINGS_HPP__