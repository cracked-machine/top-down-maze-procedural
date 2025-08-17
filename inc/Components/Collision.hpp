#ifndef __COMPONENTS_COLLISION_HPP__
#define __COMPONENTS_COLLISION_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <entt/entity/registry.hpp>
#include <optional>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// This component always returns true. 
// It should be assigned to entity when collsion has occurred.
// It should be de-assigned from entity when collsion is resolved.
class Collision {
public:
    Collision(const std::optional<sf::Rect<float>> &r) : m_rect(r) {}
    // We don't really need an attribute/accessor but a
    // valid component must have at least one member attribute.
    // This will also allow the component to be used in an expression

    void set(std::optional<sf::Rect<float>> r) { m_rect = r; }
    std::optional<sf::Rect<float>> get() { return m_rect; }
private:

    std::optional<sf::Rect<float>> m_rect;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_COLLISION_HPP__