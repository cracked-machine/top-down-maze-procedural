#ifndef __COMPONENTS_DIRECTION_HPP__
#define __COMPONENTS_DIRECTION_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <Base.hpp>

namespace ProceduralMaze::Cmp {

// Coordinate
class Direction : public sf::Vector2f, Cmp::Base {
public:

    Direction(const sf::Vector2f& dir) : 
        sf::Vector2f(dir)
    {
        SPDLOG_DEBUG("Direction()"); 
    }
    
    ~Direction() { SPDLOG_DEBUG("~Direction()"); }

};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_DIRECTION_HPP__