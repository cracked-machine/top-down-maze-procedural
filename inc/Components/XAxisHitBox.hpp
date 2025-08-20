#ifndef __COMPONENTS_XBB_HPP__
#define __COMPONENTS_XBB_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <HitBox.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Derived class Bounding Box for X-axis Collision detection
class Xbb : public BoundingBox {
public:
    Xbb(const sf::Vector2f &pos, const sf::Vector2f &size)
    : 
        BoundingBox(pos, size, sf::Color::Blue)
    {
    }

};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_XBB_HPP__