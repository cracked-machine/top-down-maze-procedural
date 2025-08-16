#ifndef __COMPONENTS_POSITION_HPP__
#define __COMPONENTS_POSITION_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <base.hpp>

namespace ProceduralMaze::Cmp {

// Coordinate
class Position : public sf::Vector2f, Cmp::Base {
public:
    Position(const sf::Vector2f& pos) 
    : 
        sf::Vector2f(pos)
    {
        SPDLOG_DEBUG("Position()"); 
    }
    ~Position() { SPDLOG_DEBUG("~Position()"); }

    Position& operator-=(const sf::Vector2f& other)
    {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }

    sf::FloatRect vbb()
    {
        return sf::FloatRect({this->x, this->y}, {10,30});
    }

    sf::FloatRect hbb()
    {
        return sf::FloatRect({this->x - 15, this->y}, {30,10});
    }

    sf::RectangleShape get_vbb_drawable() 
    { 
        auto r = sf::RectangleShape(vbb().size);
        r.setPosition(vbb().position);
        r.setFillColor(sf::Color::Transparent);
        r.setOutlineColor(sf::Color::Blue);
        r.setOutlineThickness(2);
        return r;
    }
    sf::RectangleShape get_hbb_drawable() 
    { 
        auto r = sf::RectangleShape(hbb().size);
        r.setPosition(hbb().position);
        r.setFillColor(sf::Color::Transparent);
        r.setOutlineColor(sf::Color::Red);
        r.setOutlineThickness(2);
        return r;
    }
    // // collision boxes
    // sf::FloatRect vBB;
    // sf::FloatRect hBB;
    
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_POSITION_HPP__