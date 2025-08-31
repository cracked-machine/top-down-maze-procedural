#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Direction.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/WaterLevel.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/Movement.hpp>
#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <Settings.hpp>
#include <Systems/BaseSystem.hpp>
#include <Sprites/SpriteFactory.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>


#define assertm(exp, msg) assert((void(msg), exp))

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem {
public:
    CollisionSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : BaseSystem (reg) {}
    ~CollisionSystem() = default;

    sf::Vector2f getCenter(sf::Vector2f pos, sf::Vector2f size)
    {
        return sf::FloatRect(pos, size).getCenter();
    }

    void suspend() 
    { 
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
        for (auto [_pc_entt, player] : player_collision_view.each())
        {
            if( player.m_bombdeploycooldowntimer.isRunning()) player.m_bombdeploycooldowntimer.stop(); 
        }
    }
    void resume() 
    { 
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
        for (auto [_pc_entt, player] : player_collision_view.each())
        {
            if( not player.m_bombdeploycooldowntimer.isRunning()) player.m_bombdeploycooldowntimer.start();
        }
    }

    void check_bones_reanimation()
    {
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
        auto obstacle_collision_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
        for (auto [_pc_entt, _pc, _pc_pos] : player_collision_view.each())
        {
            auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y}, Settings::PLAYER_SIZE_2F);
            for (auto [_obstacle_entt, _obstacle, _obstacle_pos] : obstacle_collision_view.each())
            {
                if( _obstacle.m_type != Sprites::SpriteFactory::Type::BONES || not _obstacle.m_enabled || not _obstacle.m_visible ) continue;
                
                auto obstacle_hitbox = sf::FloatRect({_obstacle_pos.x, _obstacle_pos.y}, Settings::OBSTACLE_SIZE_2F);
                obstacle_hitbox.size *= 2.f;
                obstacle_hitbox.position.x -= Settings::OBSTACLE_SIZE_2F.x * 0.5f;
                obstacle_hitbox.position.y -= Settings::OBSTACLE_SIZE_2F.y * 0.5f;
                
                if (player_hitbox.findIntersection(obstacle_hitbox))
                {
                    // dont really care what obstacle this is now as long as its disabled.
                    m_reg->emplace_or_replace<Cmp::Obstacle>(_obstacle_entt, Sprites::SpriteFactory::Type::BONES , 0, false, false);
                    // create a new NPC entity and put an NPC there
                    auto new_npc_entity = m_reg->create();
                    m_reg->emplace<Cmp::NPC>(new_npc_entity, true);
                    m_reg->emplace<Cmp::Position>(new_npc_entity, _obstacle_pos);
                    m_reg->emplace<Cmp::NPCScanBounds>(new_npc_entity, _obstacle_pos, Settings::OBSTACLE_SIZE_2F);
                }
            }
        }
    }

    void check_player_to_npc_collision()
    {
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::Movement>();
        auto npc_collision_view = m_reg->view<Cmp::NPC, Cmp::Position>();
        for (auto [_pc_entt, _pc, _pc_pos, _direction, _movement] : player_collision_view.each())
        {
            sf::Vector2f starting_pos = {_pc_pos.x, _pc_pos.y};
            auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y}, Settings::PLAYER_SIZE_2F);
            for (auto [_npc_entt, _npc, _npc_pos] : npc_collision_view.each())
            {

                auto npc_hitbox = sf::FloatRect({_npc_pos.x, _npc_pos.y}, Settings::OBSTACLE_SIZE_2F);

                if (player_hitbox.findIntersection(npc_hitbox))
                {
                    if( _npc.m_damage_cooldown.getElapsedTime() < _npc.DAMAGE_DELAY ) continue;
                    _pc.health -= 10;
                    _npc.m_damage_cooldown.restart();
                    // Check if player is moving
                    if (_direction.x != 0.f || _direction.y != 0.f) {
                        // Push back in opposite direction of travel
                        _pc_pos -= _direction.normalized() * 8.f;
                    } else {
                        // If not moving, use the distance as a direction of travel
                        sf::Vector2f push_dir = { _pc_pos.x - _npc_pos.x, _pc_pos.y - _npc_pos.y };
                        _pc_pos += push_dir.normalized() * 8.f;
                    }
                }

                if (_pc.health <= 0)
                {
                    _pc.alive = false;
                }
            }
        }
    }

    void check_loot_collision()
    {
        // Store both loot effects and the player velocities
        struct LootEffect {
            entt::entity loot_entity;
            Sprites::SpriteFactory::Type type;
            entt::entity player_entity;
            sf::Vector2f original_velocity;
        };
        
        std::vector<LootEffect> loot_effects;

        // First pass: detect collisions and gather effects to apply
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>();
        auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

        for (auto [_pc_entt, _pc, _pc_pos, _movement] : player_collision_view.each())
        {
            auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y}, Settings::PLAYER_SIZE_2F);
            
            for (auto [_loot_entt, _loot, _loot_pos] : loot_collision_view.each())
            {
                auto loot_hitbox = sf::FloatRect(_loot_pos, Settings::OBSTACLE_SIZE_2F);
                if (player_hitbox.findIntersection(loot_hitbox))
                {
                    // Store effect to apply after collision detection, along with original velocity
                    loot_effects.push_back({
                        _loot_entt, 
                        _loot.m_type, 
                        _pc_entt,
                        _movement.velocity
                    });
                }
            }
        }
        
        // Second pass: apply effects and remove loots
        for (const auto& effect : loot_effects)
        {
            if (!m_reg->valid(effect.player_entity)) continue;
            
            auto& _pc = m_reg->get<Cmp::PlayableCharacter>(effect.player_entity);
            
            // Apply the effect
            switch (effect.type)
            {
                case Sprites::SpriteFactory::Type::EXTRA_HEALTH:
                    _pc.health = std::min(_pc.health + 10, 100);
                    break;

                case Sprites::SpriteFactory::Type::EXTRA_BOMBS:
                    if (_pc.bomb_inventory >= 0) _pc.bomb_inventory += 5;
                    break;

                case Sprites::SpriteFactory::Type::LOWER_WATER:
                    for (auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each())
                    {
                        water_level.m_level = std::min(water_level.m_level + 100.f, static_cast<float>(Settings::DISPLAY_SIZE.y));
                        break;
                    }
                    break;

                case Sprites::SpriteFactory::Type::INFINI_BOMBS:
                    _pc.bomb_inventory = -1;
                    break;
                    
                case Sprites::SpriteFactory::Type::CHAIN_BOMBS:
                    _pc.blast_radius = std::clamp(_pc.blast_radius + 1, 0, 2);
                    break;

                default:
                    break;
            }
            
            // Restore original movement velocity
            if (m_reg->valid(effect.player_entity) && m_reg->all_of<Cmp::Movement>(effect.player_entity)) {
                auto& movement = m_reg->get<Cmp::Movement>(effect.player_entity);
                movement.velocity = effect.original_velocity;
            }
            
            // Remove the loot entity
            if (m_reg->valid(effect.loot_entity)) {
                m_reg->erase<Cmp::Loot>(effect.loot_entity);
            }
        }
    }

    void check_end_zone_collision()
    {
        for (auto [_entt, _pc, _pc_pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each())
        {
            auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y}, Settings::PLAYER_SIZE_2F);
            if (player_hitbox.findIntersection(m_end_zone))
            {
                SPDLOG_INFO("Player reached the end zone!");
                for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each())
                {
                    _sys.level_complete = true;
                }
            }
        }
    }

    void update_obstacle_distances()
    {
        auto player_view =  m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::PCDetectionBounds>();
        for (auto [_pc_entt, _pc, _pc_pos, _movement, pc_detection_bounds] : player_view.each())
        {

            auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
            for (auto [_ob_entt, _ob, _ob_pos] : obstacle_view.each())
            {
                // while we are here calculate the obstacle/player distance for any traversable obstacles
                if(not _ob.m_enabled && pc_detection_bounds.findIntersection(sf::FloatRect(_ob_pos, Settings::OBSTACLE_SIZE_2F))) {
                    auto distance = std::floor(getChebyshevDistance(_pc_pos, _ob_pos));
                    m_reg->emplace_or_replace<Cmp::PlayerDistance>(_ob_entt, distance);
                }
                else {
                    m_reg->remove<Cmp::PlayerDistance>(_ob_entt);
                }
            }
        }

    }

    void check_collision()
    {
        const float PUSH_FACTOR = 1.1f;  // Push slightly more than minimum to avoid floating point issues
        auto player_view =  m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>();
        for (auto [_pc_entt, _pc, _pc_pos, _movement] : player_view.each())
        {
            sf::Vector2f starting_pos = {_pc_pos.x, _pc_pos.y};
            int stuck_loop = 0;
         
            // Reset collision flag at start of frame
            _movement.is_colliding = false;

            auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
            for (auto [_ob_entt, _ob, _ob_pos] : obstacle_view.each())
            {

                // otherwise we are not interested in collision detection on traversable obstacles
                if (not _ob.m_enabled) { continue; }

                auto player_floatrect = sf::FloatRect({ _pc_pos.x, _pc_pos.y }, Settings::PLAYER_SIZE_2F);
                auto brick_floatRect = sf::FloatRect(_ob_pos, Settings::OBSTACLE_SIZE_2F);

                auto collision = player_floatrect.findIntersection(brick_floatRect);
                if (!collision) continue;

                stuck_loop++;

                // We'll keep track if we're near the top wall for special handling
                bool near_top_wall = (_pc_pos.y < Settings::MAP_GRID_OFFSET.y * 16.f);

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

                // Near top wall: Slightly bias vertical resolution but don't force it
                if (near_top_wall && depthY > 0) {
                    // If pushing down would resolve collision and we're at the top wall,
                    // slightly prefer vertical resolution (by reducing the X penetration)
                    depthX *= 1.2f;  // Makes horizontal resolution slightly less likely
                }

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

                // Special case for top wall: prevent any upward movement
                if (near_top_wall && _pc_pos.y < Settings::MAP_GRID_OFFSET.y * 16.f + 4.0f) {
                    _movement.velocity.y = std::max(0.0f, _movement.velocity.y);
                }
                
                // Mark that we're colliding for this frame
                _movement.is_colliding = true;

                // Extra safety for top wall
                if (near_top_wall && _pc_pos.y < Settings::MAP_GRID_OFFSET.y * 16.f) {
                    _pc_pos.y = Settings::MAP_GRID_OFFSET.y * 16.f + 1.0f;
                }
                
                SPDLOG_DEBUG("Collision resolved - new pos: {},{}", _pc_pos.x, _pc_pos.y);
            }
        }
    }

private:

    sf::FloatRect m_end_zone{
        {Settings::DISPLAY_SIZE.x * 1.f, 0}, 
        {500.f, Settings::DISPLAY_SIZE.y * 1.f}
    };
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__