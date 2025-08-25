#ifndef __COMPONENTS_BOMB_SYSTEM_HPP__
#define __COMPONENTS_BOMB_SYSTEM_HPP__

#include <Armed.hpp>
#include <Neighbours.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Settings.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include <type_traits>
#include <unordered_set>

namespace ProceduralMaze::Sys {

// this currently only supports one bomb at a time
class BombSystem {
public:
    BombSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : m_reg(reg) { }

    void update()
    {
        auto explosion_zone = sf::FloatRect(); // tbd
        explosion_zone.size = max_explosion_zone_size;

        auto armed_view = m_reg->view<Cmp::Armed, Cmp::Obstacle, Cmp::Neighbours, Cmp::Position>();
        for( auto [_entt, _armed_cmp, _obstacle_cmp, _neighbours_cmp, _ob_pos_comp]: armed_view.each() ) 
        {
            if (_armed_cmp.getElapsedTime() < detonation_delay) continue;

            // The `_ob_pos_comp` position component is the position of the explosion center block (marked with C), 
            // so move back up/left one obstacle size to the uptmost top-left corner (marked with X):
            //
            // X---|---|---|
            // |   |   |   |
            // |---C---|---|
            // |   |   |   |
            // |---|---|---|
            // |   |   |   |
            // |---|---|---|
            explosion_zone.position = _ob_pos_comp - sf::Vector2f{Settings::OBSTACLE_SIZE};

            // detonate the neighbour obstacles!
            for( auto [dir, neighbour_entity] : _neighbours_cmp) 
            {
                if( not m_reg->valid(entt::entity(neighbour_entity)) ) 
                {
                    SPDLOG_WARN("List provided invalid neighbour entity: {}", entt::to_integral(neighbour_entity));
                    assert(m_reg->valid(entt::entity(neighbour_entity)) && "List provided invalid neighbour entity: " 
                        && entt::to_integral(neighbour_entity));
                    continue;
                }

                Cmp::Obstacle* nb_obstacle = m_reg->try_get<Cmp::Obstacle>(entt::entity(neighbour_entity));
                if ( nb_obstacle && nb_obstacle->m_enabled && not nb_obstacle->m_broken)
                {
                    // tell the render system to draw detonated obstacle differently
                    nb_obstacle->m_broken = true;
                    nb_obstacle->m_enabled = false;
                }
            }
            // SPDLOG_INFO("Explosion zone is {},{} {},{}", explosion_zone.position.x, explosion_zone.position.y, explosion_zone.size.x, explosion_zone.size.y);
            // Check if any player is in the explosion area and damage them
            auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
            for (auto [player_entt, player, player_position] : player_view.each())
            {
                auto player_bounding_box = sf::FloatRect{ player_position, sf::Vector2f{ Settings::PLAYER_SIZE } };
                if(player_bounding_box.findIntersection(explosion_zone))
                {
          
                    // Damage the player
                    player.health -= player_damage;
                    // SPDLOG_INFO("Player hit by explosion! Health reduced to {}", player.health);
                }

                // You can add additional logic for player death if health reaches 0
                if (player.health <= 0)
                {
                    // SPDLOG_INFO("Player died from explosion!");
                    player.alive = false;
                    // Add any player death handling here
                }
                
            }

            // if we got this far then the bomb detonated, we can destroy the armed component
            m_reg->erase<Cmp::Armed>(_entt);

            // allow player to place next bomb
            for (auto [_pc_entt, _pc] :m_reg->view<Cmp::PlayableCharacter>().each())
            {
                _pc.has_active_bomb = false;
            }
        }
    }

private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    sf::Time detonation_delay{sf::seconds(3)};
    int player_damage = 10; // Amount of damage to deal to the player when hit by explosion
    const sf::Vector2f max_explosion_zone_size{Settings::OBSTACLE_SIZE.x * 3.f, Settings::OBSTACLE_SIZE.y * 3.f};
};

} // namespace ProceduralMaze::Sys

#endif // __COMPONENTS_BOMB_SYSTEM_HPP__