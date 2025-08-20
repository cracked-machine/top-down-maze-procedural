#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <entt/entity/registry.hpp>

#include <Collision.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <spdlog/spdlog.h>
#include <Components/System.hpp>


#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>
#include <Systems/BaseSystem.hpp>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem {
public:
    CollisionSystem() {}
    ~CollisionSystem() = default;

     entt::reactive_mixin<entt::storage<void>> m_position_updates;

    sf::Vector2f getCenter(sf::Vector2f pos, sf::Vector2f size)
    {
        return sf::FloatRect(pos, size).getCenter();
    }

    void check(entt::basic_registry<entt::entity> &reg)
    {
        const float PUSH_FACTOR = 1.1f;  // Push slightly more than minimum to avoid floating point issues
        
        for (auto [_pc_entt, _pc, _pc_pos] :
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each())
        {
            sf::Vector2f starting_pos = {_pc_pos.x, _pc_pos.y};
            int stuck_loop = 0;
            bool had_collision = false;

            for (auto [_ob_entt, _ob, _ob_pos] :
                m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each())
            {
                if (not _ob.m_enabled) { continue; }

                auto player_floatrect = sf::FloatRect({ _pc_pos.x, _pc_pos.y }, Settings::PLAYER_SIZE_2F);
                auto brick_floatRect = sf::FloatRect(_ob_pos, Settings::OBSTACLE_SIZE_2F);

                auto collision = player_floatrect.findIntersection(brick_floatRect);
                if (!collision) continue;

                had_collision = true;
                stuck_loop++;

                if (stuck_loop > 5) // Reduced threshold, but we'll be smarter about resolution
                {
                    // First try moving back to starting position
                    _pc_pos.x = starting_pos.x;
                    _pc_pos.y = starting_pos.y;
                    
                    player_floatrect = sf::FloatRect({ _pc_pos.x, _pc_pos.y }, Settings::PLAYER_SIZE_2F);
                    if (!player_floatrect.findIntersection(brick_floatRect))
                    {
                        SPDLOG_INFO("Recovered by reverting to start position");
                        continue;
                    }

                    // If still stuck, reset to spawn
                    SPDLOG_INFO("Could not recover, resetting to spawn");
                    _pc_pos = ProceduralMaze::Settings::PLAYER_START_POS;
                    for (auto [_entt, _sys] : reg.view<Cmp::System>().each())
                    {
                        _sys.player_stuck = true;
                    }
                    return;
                }

                auto brickCenter = brick_floatRect.getCenter();
                auto playerCenter = player_floatrect.getCenter();
                
                auto diffX = playerCenter.x - brickCenter.x;
                auto diffY = playerCenter.y - brickCenter.y;
                
                auto minXDist = (player_floatrect.size.x / 2.0f) + (brick_floatRect.size.x / 2.0f);
                auto minYDist = (player_floatrect.size.y / 2.0f) + (brick_floatRect.size.y / 2.0f);

                // Calculate signed penetration depths
                float depthX = (diffX > 0 ? 1.0f : -1.0f) * (minXDist - std::abs(diffX));
                float depthY = (diffY > 0 ? 1.0f : -1.0f) * (minYDist - std::abs(diffY));

                // Store current position in case we need to revert
                sf::Vector2f pre_resolve_pos = {_pc_pos.x, _pc_pos.y};

                // Always resolve along the axis of least penetration
                if (std::abs(depthX) < std::abs(depthY))
                {
                    // Push out along X axis
                    _pc_pos.x += depthX * PUSH_FACTOR;
                }
                else
                {
                    // Push out along Y axis
                    _pc_pos.y += depthY * PUSH_FACTOR;
                }

                // Verify the resolution worked
                player_floatrect = sf::FloatRect({ _pc_pos.x, _pc_pos.y }, Settings::PLAYER_SIZE_2F);
                if (player_floatrect.findIntersection(brick_floatRect))
                {
                    // If resolution failed, try reverting and using the other axis
                    _pc_pos = pre_resolve_pos;
                    if (std::abs(depthX) < std::abs(depthY))
                    {
                        _pc_pos.y += depthY * PUSH_FACTOR;
                    }
                    else
                    {
                        _pc_pos.x += depthX * PUSH_FACTOR;
                    }
                }

                SPDLOG_DEBUG("Collision resolved - new pos: {},{}", _pc_pos.x, _pc_pos.y);
            }
        }
    }
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__