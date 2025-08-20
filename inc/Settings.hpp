#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <Brick.hpp>
#include <SFML/Graphics.hpp>
#include <string>

namespace ProceduralMaze::Settings {

const sf::Vector2u DISPLAY_SIZE{ 1920, 1024 };

// MAP_GRID_OFFSET and MAP_GRID_SIZE are in blocks, not pixels
const sf::Vector2f MAP_GRID_OFFSET{ 10.f,2.f };  
const sf::Vector2u MAP_GRID_SIZE{ 120u, 54u }; 
const sf::Vector2f PLAYER_START_POS{ 20, static_cast<float>(DISPLAY_SIZE.y) / 2 };
// const sf::Vector2u MAP_GRID_SIZE{ 10u, 10u }; // for troubleshooting ProceduralGeneration
// const sf::Vector2f PLAYER_START_POS{ 20, MAP_GRID_OFFSET.y + 32 };

// SFML views
const sf::Vector2f LOCAL_MAP_VIEW_SIZE{ 300.f, 200.f };
const sf::Vector2f MINI_MAP_VIEW_SIZE{ DISPLAY_SIZE.x * 0.25f, DISPLAY_SIZE.y * 0.25f };

const sf::Vector2u OBSTACLE_SIZE(16, 16);
const sf::Vector2f OBSTACLE_SIZE_2F(OBSTACLE_SIZE.x, OBSTACLE_SIZE.y);
const std::string WALL_TILESET_PATH = "res/Pixel Lands Dungeons/objects.png";
const std::vector<unsigned int> WALL_TILE_POOL = {
    147, 147, 147, 147, 147, 147, 147, 147,      // rock pile #1
    148, 148, 148, 148, 148, 148, 148, 148,      // rock pile #2

    339,                                // brown pot
    270,                                // skull and bones
    271                                 // bones
};

const std::string BORDER_TILESET_PATH = "res/Pixel Lands Dungeons/objects.png";
const std::vector<unsigned int> BORDER_TILE_POOL = {    
    247                                // stone column square top
    // 249                              // stone column circle top};
};

const std::vector<unsigned int> FLOOR_TILE_POOL {42,48,49};

const sf::Vector2u PLAYER_SIZE(16, 16);
const sf::Vector2f PLAYER_SIZE_2F(PLAYER_SIZE.x, PLAYER_SIZE.y);
const sf::Vector2u PLAYER_SPRITE_SIZE(16, 32);
const std::string PLAYER_TILESET_PATH = "res/players.png";
const std::vector<unsigned int> PLAYER_TILE_POOL = {    
    0, 1, 2
};

const float MAX_BOMB_TIME = 3.0f; // seconds
}

#endif // __SETTINGS_HPP__