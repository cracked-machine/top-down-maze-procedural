#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <Position.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Sys {

class BaseSystem {
public:
    BaseSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
        : m_reg(reg) {}

    ~BaseSystem() = default;

    // Get a grid position from an entity's Position component
    std::optional<sf::Vector2i> getGridPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) {
            return std::optional<sf::Vector2i>{{
                static_cast<int>(pos->x / Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE.x), 
                static_cast<int>(pos->y / Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE.y)
            }};
        }
        return std::nullopt; 
    }

    // Get a pixel position from an entity's Position component
    std::optional<sf::Vector2f> getPixelPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) { return *pos; }
        return std::nullopt; 
    }

    // sum( (posA.x - posB.x) + (posA.y - posB.y) )
    // cardinal directions only
    unsigned int getManhattanDistance(sf::Vector2i posA, sf::Vector2i posB ) const
    {
        return std::abs(posA.x - posB.x) + std::abs(posA.y - posB.y);
    }

    // sum( (posA.x - posB.x) + (posA.y - posB.y) )
    // cardinal directions only
    float getManhattanDistance(sf::Vector2f posA, sf::Vector2f posB ) const
    {
        return std::abs(posA.x - posB.x) + std::abs(posA.y - posB.y);
    }

    // max( (posA.x - posB.x), (posA.y - posB.y) )
    // cardinal and diagonal directions
    unsigned int getChebyshevDistance(sf::Vector2i posA, sf::Vector2i posB ) const
    {
        return std::max(std::abs(posA.x - posB.x), std::abs(posA.y - posB.y));
    }

    // max( (posA.x - posB.x), (posA.y - posB.y) )
    // cardinal and diagonal directions
    float getChebyshevDistance(sf::Vector2f posA, sf::Vector2f posB ) const
    {
        return std::max(std::abs(posA.x - posB.x), std::abs(posA.y - posB.y));
    }

    sf::FloatRect get_hitbox(sf::Vector2f pos)
    {
        return sf::FloatRect(
            { pos.x, pos.y }, 
            sf::Vector2f{Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE}
        );
    }

protected:
    // Entity registry
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_BASE_SYSTEM_HPP__