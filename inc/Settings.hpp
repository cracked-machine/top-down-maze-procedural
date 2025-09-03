#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace ProceduralMaze::Settings {

const sf::Vector2u DISPLAY_SIZE{ 1920, 1024 };

// MAP_GRID_OFFSET and MAP_GRID_SIZE are in blocks, not pixels
const sf::Vector2f MAP_GRID_OFFSET{ 10.f,1.f };  
const sf::Vector2u MAP_GRID_SIZE{ 100u, 61u }; 
const sf::FloatRect PLAYER_END_ZONE{{DISPLAY_SIZE.x - 8.f, MAP_GRID_OFFSET.y}, {8, static_cast<float>(MAP_GRID_SIZE.y)}};

// SFML views
const sf::Vector2f LOCAL_MAP_VIEW_SIZE{ 300.f, 200.f };
const sf::Vector2f MINI_MAP_VIEW_SIZE{ DISPLAY_SIZE.x * 0.25f, DISPLAY_SIZE.y * 0.25f };

const std::vector<unsigned int> FLOOR_TILE_POOL {   // res/Pixel Lands Dungeons/objects.png
    48,48,48,48,48,48,
    49
};    

}

#endif // __SETTINGS_HPP__