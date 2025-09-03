#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Position.hpp>
#include <Components/Movement.hpp>
#include <Direction.hpp>
#include <PCDetectionBounds.hpp>
#include <PlayableCharacter.hpp>
#include <entt/entity/registry.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

class PlayerSystem : public BaseSystem {
public:
    PlayerSystem(std::shared_ptr<entt::basic_registry<entt::entity>> registry)
        : BaseSystem(registry) {}


    void add_player_entity()
    {
        if( not m_reg->view<Cmp::PlayableCharacter>()->empty()) {
            SPDLOG_WARN("Player entity already exists, skipping creation");
            return;
        }
        SPDLOG_INFO("Creating player entity");
        auto entity = m_reg->create();
        m_reg->emplace<Cmp::Position>(entity, Settings::PLAYER_START_POS);
        m_reg->emplace<Cmp::PlayableCharacter>(entity);
        m_reg->emplace<Cmp::Movement>(entity);
        m_reg->emplace<Cmp::Direction>(entity, sf::Vector2f{0,0});
        m_reg->emplace<Cmp::PCDetectionBounds>(entity, Settings::PLAYER_START_POS, Settings::OBSTACLE_SIZE_2F);
    }

    void update(sf::Time deltaTime) {
        const float dt = deltaTime.asSeconds();

        for(auto [ entity, pc_cmp, pos_cmp, move_cmp, dir_cmp, pc_bounds] : 
            m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::Direction, Cmp::PCDetectionBounds>().each())
        {
            // Apply acceleration in the desired dir_cmp
            if (dir_cmp != sf::Vector2f(0.0f, 0.0f)) 
            {
                move_cmp.acceleration = dir_cmp * move_cmp.acceleration_rate;
            } 
            else 
            {
                // Apply deceleration when no input
                if (move_cmp.velocity != sf::Vector2f(0.0f, 0.0f)) 
                {
                    move_cmp.acceleration = -move_cmp.velocity.normalized() * move_cmp.deceleration_rate;
                } 
                else 
                {
                    move_cmp.acceleration = sf::Vector2f(0.0f, 0.0f);
                }
            }

            // Update velocity (change in velocity = acceleration * dt)
            move_cmp.velocity += move_cmp.acceleration * dt;

            // Stop completely if current velocity magnitude is below minimum velocity
            if (move_cmp.velocity.length() < move_cmp.min_velocity) {
                move_cmp.velocity = sf::Vector2f(0.0f, 0.0f);
                move_cmp.acceleration = sf::Vector2f(0.0f, 0.0f);
            }
            // Clamp velocity to max speed if current velocity magnitude exceeds max speed
            else if (move_cmp.velocity.length() > move_cmp.max_speed) {
                move_cmp.velocity = (move_cmp.velocity / move_cmp.velocity.length()) * move_cmp.max_speed;
            }

            // Apply velocity to position (change in position = velocity * dt)
            pos_cmp += move_cmp.velocity * dt;
            pc_bounds.position(pos_cmp);

        }
    }

};

} // namespace Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
