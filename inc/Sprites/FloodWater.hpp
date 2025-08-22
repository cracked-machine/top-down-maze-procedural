#ifndef __SPRITES_WATER_LEVEL_HPP__
#define __SPRITES_WATER_LEVEL_HPP__

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sprites {

class FloodWaters : public sf::RectangleShape {
public:
    FloodWaters(sf::Vector2f size, sf::Vector2f pos) 
    : sf::RectangleShape(size)
    {
        sf::RectangleShape::setPosition(pos);
        sf::RectangleShape::setFillColor(sf::Color(0,0,255, 100));
    }
    
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_WATER_LEVEL_HPP__