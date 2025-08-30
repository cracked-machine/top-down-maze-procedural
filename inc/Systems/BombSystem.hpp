#ifndef __COMPONENTS_BOMB_SYSTEM_HPP__
#define __COMPONENTS_BOMB_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/Loot.hpp>
#include <Systems/BaseSystem.hpp>
#include <Components/Movement.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Settings.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

// this currently only supports one bomb at a time
class BombSystem : public BaseSystem {
public:
    BombSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::shared_ptr<Sprites::SpriteFactory> sprite_factory
    ) 
    : 
        BaseSystem(reg),
        m_sprite_factory(sprite_factory)
    {
    }


    void suspend() 
    { 
        auto player_collision_view = m_reg->view<Cmp::Armed>();
        for (auto [_pc_entt, armed] : player_collision_view.each())
        {
            if( armed.m_clock.isRunning()) armed.m_clock.stop(); 
        }
    }
    void resume() 
    { 
        auto player_collision_view = m_reg->view<Cmp::Armed>();
        for (auto [_pc_entt, armed] : player_collision_view.each())
        {
            if( not armed.m_clock.isRunning()) armed.m_clock.start();
        }
    }


    void arm_occupied_location()
    {
        auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>();
        for (auto [_pc_entt, _pc, _pc_pos, _movement] : player_collision_view.each())
        {
            if( _pc.has_active_bomb ) continue; // skip if player already placed a bomb
            if( _pc.bomb_inventory == 0 ) continue; // skip if player has no bombs left, -1 is infini bombs
            
            // Store movement velocity before bomb placement
            sf::Vector2f original_velocity = _movement.velocity;

            auto obstacle_collision_view =  m_reg->view<Cmp::Obstacle, Cmp::Position>(entt::exclude<typename Cmp::Armed>);
            for (auto [_ob_entt_lvl1, _obstacle_cmp_lvl1, _ob_pos_cmp_lvl1] : obstacle_collision_view.each())
            {
                auto player_hitbox = sf::FloatRect({_pc_pos.x, _pc_pos.y},  Settings::PLAYER_SIZE_2F);
                
                // reduce the size of the hitbox and center it 
                player_hitbox.size.x /= 2.f;
                player_hitbox.size.y /= 2.f;
                player_hitbox.position.x += 4.f;
                player_hitbox.position.y += 4.f;

                auto obstacle_hitbox = sf::FloatRect(_ob_pos_cmp_lvl1, Settings::OBSTACLE_SIZE_2F);     

                // are we standing on this tile?
                if( player_hitbox.findIntersection(obstacle_hitbox) )
                {
                    // has the bomb spamming cooldown expired?
                    if( _pc.m_bombdeploycooldowntimer.getElapsedTime() >= _pc.m_bombdeploydelay ) 
                    {
                        if(m_fuse_sound_player.getStatus() != sf::Sound::Status::Playing) m_fuse_sound_player.play();
                        // arm the current tile (center of the bomb)
                        m_reg->emplace_or_replace<Cmp::Armed>(entt::entity(_ob_entt_lvl1), sf::seconds(3), true, sf::Color::Blue);
                        
                        // Calculate tile coordinates for the center (bomb location)
                        // (convert from pixel position to grid position)
                        sf::Vector2i centerTile = {
                            static_cast<int>(_ob_pos_cmp_lvl1.x / Settings::OBSTACLE_SIZE_2F.x),
                            static_cast<int>(_ob_pos_cmp_lvl1.y / Settings::OBSTACLE_SIZE_2F.y)
                        };

                        // Define blast radius
                        const int BLAST_RADIUS = _pc.blast_radius;
                        
                        // Get all obstacles with positions for the blast pattern
                        auto all_obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
                        
                        // Create a square blast pattern by calculating Manhattan distance
                        for (auto [entity, obstacle, pos] : all_obstacle_view.each()) {
                            // Skip the center tile (already armed)
                            if (entity == _ob_entt_lvl1) continue;
                            
                            // Skip already armed obstacles
                            if (m_reg->any_of<Cmp::Armed>(entity)) continue;
                            
                            // Calculate this obstacle's grid position
                            sf::Vector2i obstacleTile = {
                                static_cast<int>(pos.x / Settings::OBSTACLE_SIZE_2F.x),
                                static_cast<int>(pos.y / Settings::OBSTACLE_SIZE_2F.y)
                            };
                            
                            // Calculate Manhattan distance (creates diamond pattern)
                            int dx = std::abs(obstacleTile.x - centerTile.x);
                            int dy = std::abs(obstacleTile.y - centerTile.y);
                            int distance = dx + dy; // Manhattan distance
                            // int distance = std::max(dx, dy); // square pattern
                            
                            // If within blast radius, arm it
                            if (distance > 0 && distance <= BLAST_RADIUS) {
                                // Determine color and delay based on distance
                                sf::Color color = (distance == 1) ? sf::Color::Green : sf::Color::Yellow;
                                sf::Time delay = sf::seconds(3 + (distance * 0.3));
                                
                                m_reg->emplace<Cmp::Armed>(entity, delay, true, color);
                            }
                        }

                        _pc.m_bombdeploycooldowntimer.restart();
                        _pc.has_active_bomb = true;
                        _pc.bomb_inventory = (_pc.bomb_inventory > 0) ? _pc.bomb_inventory - 1 : _pc.bomb_inventory;
                        
                        // Apply a smooth velocity transition instead of abrupt restoration
                        // This will blend the current velocity with the original to prevent jumping
                        sf::Vector2f current_velocity = _movement.velocity;
                        float blend_factor = 0.3f; // Adjust for smoothness (0 = full original, 1 = no change)
                        
                        _movement.velocity = sf::Vector2f(
                            current_velocity.x * blend_factor + original_velocity.x * (1.0f - blend_factor),
                            current_velocity.y * blend_factor + original_velocity.y * (1.0f - blend_factor)
                        );
                        
                        // Cap the restored velocity to avoid sudden bursts
                        const float max_restore_speed = _movement.max_speed * 0.5f;
                        if (_movement.velocity.length() > max_restore_speed) {
                            _movement.velocity = (_movement.velocity / _movement.velocity.length()) * max_restore_speed;
                        }
                    }
                }
            }
        }
    }

    void detonate_neighbour_entity(entt::entity &neighbour_entity)
    {
        if( not m_reg->valid(entt::entity(neighbour_entity)) ) 
        {
            SPDLOG_WARN("List provided invalid neighbour entity: {}", entt::to_integral(neighbour_entity));
            assert(m_reg->valid(entt::entity(neighbour_entity)) && "List provided invalid neighbour entity: " 
                && entt::to_integral(neighbour_entity));
            return;
        }

        Cmp::Obstacle* nb_obstacle = m_reg->try_get<Cmp::Obstacle>(entt::entity(neighbour_entity));
        if ( nb_obstacle && nb_obstacle->m_enabled && not nb_obstacle->m_broken)
        {
            // tell the render system to draw detonated obstacle differently
            nb_obstacle->m_broken = true;
            nb_obstacle->m_enabled = false;
            
            // add loot to any broken pot neighbour entities
            if( nb_obstacle->m_type == Sprites::SpriteFactory::Type::POT)
            {
                auto random_selected_loot_metadata = m_sprite_factory->get_random_metadata(std::vector<Sprites::SpriteFactory::Type>{
                    Sprites::SpriteFactory::Type::EXTRA_HEALTH,
                    Sprites::SpriteFactory::Type::EXTRA_BOMBS,
                    Sprites::SpriteFactory::Type::INFINI_BOMBS,
                    Sprites::SpriteFactory::Type::CHAIN_BOMBS,
                    Sprites::SpriteFactory::Type::LOWER_WATER
                });
                m_reg->emplace<Cmp::Loot>(neighbour_entity,
                    random_selected_loot_metadata->get_type(),
                    random_selected_loot_metadata->pick_random_texture_index()
                );
            }
        }
        
        m_detonate_sound_player.play();
        
    }

    void update()
    {
        auto explosion_zone = sf::FloatRect(); // tbd
        explosion_zone.size = max_explosion_zone_size;

        auto armed_view = m_reg->view<Cmp::Armed, Cmp::Obstacle, Cmp::Neighbours, Cmp::Position>();
        for( auto [_entt, _armed_cmp, _obstacle_cmp, _neighbours_cmp, _ob_pos_comp]: armed_view.each() ) 
        {
            if (_armed_cmp.getElapsedTime() < _armed_cmp.m_detonation_delay) continue;

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
                detonate_neighbour_entity(neighbour_entity);
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

            for(auto [npc_entt, npc_cmp, npc_pos_cmp] : m_reg->view<Cmp::NPC, Cmp::Position>().each())
            {
                auto npc_bounding_box = sf::FloatRect{ npc_pos_cmp, sf::Vector2f{ Settings::OBSTACLE_SIZE_2F } };
                // Check if the NPC is within the explosion zone
                if(explosion_zone.findIntersection(npc_bounding_box))
                {
                    // kill npc
                    m_reg->destroy(npc_entt);
                }

            }   

            // if we got this far then the bomb detonated, we can destroy the armed component
            m_reg->erase<Cmp::Armed>(_entt);

            if(m_fuse_sound_player.getStatus() == sf::Sound::Status::Playing) m_fuse_sound_player.stop();
            if(m_detonate_sound_player.getStatus() != sf::Sound::Status::Playing) m_detonate_sound_player.play();

            // allow player to place next bomb
            for (auto [_pc_entt, _pc] :m_reg->view<Cmp::PlayableCharacter>().each())
            {
                _pc.has_active_bomb = false;
            }
        }
    }

private:
    std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory;
    
    int player_damage = 10; // Amount of damage to deal to the player when hit by explosion
    const sf::Vector2f max_explosion_zone_size{Settings::OBSTACLE_SIZE.x * 3.f, Settings::OBSTACLE_SIZE.y * 3.f};

    sf::SoundBuffer m_fuse_sound_buffer{"res/audio/fuse.wav"};
    sf::Sound m_fuse_sound_player{m_fuse_sound_buffer};
    sf::SoundBuffer m_detonate_sound_buffer{"res/audio/detonate.wav"};
    sf::Sound m_detonate_sound_player{m_detonate_sound_buffer};
};

} // namespace ProceduralMaze::Sys

#endif // __COMPONENTS_BOMB_SYSTEM_HPP__