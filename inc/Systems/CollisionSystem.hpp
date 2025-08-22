#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <Neighbours.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <Collision.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <spdlog/spdlog.h>
#include <Components/System.hpp>
#include <Components/Movement.hpp>


#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>
#include <Systems/BaseSystem.hpp>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem {
public:
    CollisionSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : m_reg(reg) {}
    ~CollisionSystem() = default;

    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;

    entt::reactive_mixin<entt::storage<void>> m_collision_updates;

    sf::Vector2f getCenter(sf::Vector2f pos, sf::Vector2f size)
    {
        return sf::FloatRect(pos, size).getCenter();
    }

    void track_path(bool place_bomb)
    {
        for (auto [_pc_entt, _pc, _pc_pos] :
            m_collision_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each())
        {
            for (auto [_ob_entt, _ob, _ob_pos, _ob_nb_list] :
                m_collision_updates.view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each())
            {
                auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y},  Settings::PLAYER_SIZE_2F);
                
                // reduce the size of the hitbox and center it 
                player_hitbox.size.x /= 2.f;
                player_hitbox.size.y /= 2.f;
                player_hitbox.position.x += 4.f;
                player_hitbox.position.y += 4.f;

                auto brick_hitbox = sf::FloatRect(_ob_pos, Settings::OBSTACLE_SIZE_2F);     

                // arm the occupied  tile if the player doesn't have an active bomb
                if( player_hitbox.findIntersection(brick_hitbox) && place_bomb ) {
                    if( not _pc.has_active_bomb )
                    {
                        _ob.m_armed = true;
                        _ob.m_bomb_timer.restart();
                        _pc.has_active_bomb = true;
                    }
                }
                // detonate the bomb if it has timed out
                if( _ob.m_armed && _ob.m_bomb_timer.getElapsedTime().asSeconds() > Settings::MAX_BOMB_TIME ) 
                {
                    // reset the occupied state
                    _ob.m_armed = false;
                    
                    _ob.m_bomb_timer.reset();
                    _pc.has_active_bomb = false;

                    // Iterate list of neighbours from the current obstacle 
                    // and mark each one as broken
                    for( auto [_dir, _nb_entt] : _ob_nb_list) 
                    {
                        // TODO why doesn't this work?
                        // if( m_reg->valid(entt::entity(_nb_entt)) ) {
                            auto &nb_obstacle = m_reg->get<Cmp::Obstacle>(_nb_entt);
                            if( nb_obstacle.m_enabled && not nb_obstacle.m_broken )
                            {
                                nb_obstacle.m_broken = true;
                                nb_obstacle.m_enabled = false;
                                SPDLOG_INFO("Detonated neighbour: {}", entt::entt_traits<entt::entity>::to_entity(_nb_entt));
                            }
                        // }
                        // else
                        // {
                        //     SPDLOG_WARN("Invalid neighbour entity: {}", entt::entt_traits<entt::entity>::to_entity(_nb_entt));
                        // }
                    }
                                    
                }

            }

        }
        
    }

    void check()
    {
        const float PUSH_FACTOR = 1.1f;  // Push slightly more than minimum to avoid floating point issues
        
        for (auto [_pc_entt, _pc, _pc_pos, _movement] :
            m_collision_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>().each())
        {
            sf::Vector2f starting_pos = {_pc_pos.x, _pc_pos.y};
            int stuck_loop = 0;
            bool had_collision = false;
            
            // Reset collision flag at start of frame
            _movement.is_colliding = false;

            for (auto [_ob_entt, _ob, _ob_pos] :
                m_collision_updates.view<Cmp::Obstacle, Cmp::Position>().each())
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
                    for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each())
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
                    
                    // Calculate speed-based friction coefficient
                    float speed_ratio = std::abs(_movement.velocity.y) / _movement.max_speed;
                    float dynamic_friction = _movement.friction_coefficient * 
                        (1.0f - (_movement.friction_falloff * speed_ratio));
                    
                    // Apply friction to Y velocity with smooth falloff
                    _movement.velocity.y *= (1.0f - dynamic_friction);
                    
                    // Check if Y velocity is below minimum
                    if (std::abs(_movement.velocity.y) < _movement.min_velocity) {
                        _movement.velocity.y = 0.0f;
                    }
                }
                else
                {
                    // Push out along Y axis
                    _pc_pos.y += depthY * PUSH_FACTOR;
                    
                    // Calculate speed-based friction coefficient
                    float speed_ratio = std::abs(_movement.velocity.x) / _movement.max_speed;
                    float dynamic_friction = _movement.friction_coefficient * 
                        (1.0f - (_movement.friction_falloff * speed_ratio));
                    
                    // Apply friction to X velocity with smooth falloff
                    _movement.velocity.x *= (1.0f - dynamic_friction);
                    
                    // Check if X velocity is below minimum
                    if (std::abs(_movement.velocity.x) < _movement.min_velocity) {
                        _movement.velocity.x = 0.0f;
                    }
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
                        _movement.velocity.x *= (1.0f - _movement.friction_coefficient);
                    }
                    else
                    {
                        _pc_pos.x += depthX * PUSH_FACTOR;
                        _movement.velocity.y *= (1.0f - _movement.friction_coefficient);
                    }
                }
                
                // Mark that we're colliding for this frame
                _movement.is_colliding = true;

                SPDLOG_DEBUG("Collision resolved - new pos: {},{}", _pc_pos.x, _pc_pos.y);
            }
        }
    }
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__