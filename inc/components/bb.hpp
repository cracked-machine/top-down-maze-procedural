#ifndef __COMPONENTS_BOUNDINGBOX_HPP__
#define __COMPONENTS_BOUNDINGBOX_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Base class Bounding Box for Collision detection
class BoundingBox : public sf::FloatRect {
public:
    BoundingBox(
        const sf::Vector2f &pos, 
        const sf::Vector2f &size, 
        const sf::Color &line_colour)
    : 
        sf::FloatRect(pos, size),
        m_line_colour(line_colour)
    {
    }
    
    sf::RectangleShape drawable() 
    {
        auto r = sf::RectangleShape(this->size);
        r.setPosition(this->position);
        r.setFillColor(sf::Color::Transparent);
        r.setOutlineColor(m_line_colour);
        r.setOutlineThickness(2);
        return r;
    }
    sf::Color m_line_colour{sf::Color::White};
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_BOUNDINGBOX_HPP__