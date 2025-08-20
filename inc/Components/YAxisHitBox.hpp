#ifndef __COMPONENTS_YBB_HPP__
#define __COMPONENTS_YBB_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <HitBox.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Derived class Bounding Box for Y-axis Collision detection
class Ybb : public BoundingBox {
public:
    Ybb(const sf::Vector2f &pos, const sf::Vector2f &size)
    : 
        BoundingBox(pos, size, sf::Color::Red)
    {
    }

};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_YBB_HPP__