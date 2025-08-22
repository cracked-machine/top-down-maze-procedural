#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <BaseSystem.hpp>
#include <Neighbours.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
#include <iterator>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <memory>
#include <spdlog/spdlog.h>

#include <Sprites/Brick.hpp>
#include <Components/Random.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class CellAutomataSystem {
public:
    CellAutomataSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::unique_ptr<RandomLevelGenerator> random_level)
    : 
        m_reg(reg),
        m_random_level(std::move(random_level))
    {}

    void iterate(unsigned int iterations)
    {
        sf::Clock iteration_timer;
        for(unsigned int i = 0; i < iterations; i++)
        {
            find_neighbours();
            apply_rules();
            SPDLOG_INFO("#{} took {}ms", i, iteration_timer.restart().asMilliseconds());
        }
        
        // run one last time to get the latest neighbour data
        find_neighbours();
        
        SPDLOG_INFO("Total Iterations: {}", iterations);
        
    }
    
private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    std::unique_ptr<RandomLevelGenerator> m_random_level;

    void find_neighbours()
    {  
        using entity_trait = entt::entt_traits<entt::entity>;

        // 1. find neighbours
        for(auto it = m_random_level->begin(); it != m_random_level->end(); it++) {
            if (!m_reg->valid(entt::entity(*it))) 
            {
                SPDLOG_WARN("Entity {} is not valid! Valid entities are:", (*it));
                std::string valid_entities;
                for([[maybe_unused]] auto entity: m_reg->view<entt::entity>()) { 
                    valid_entities += " " + std::to_string(entity_trait::to_integral(entity)); }
                SPDLOG_WARN("{}", valid_entities); 
            }


            SPDLOG_TRACE("Entity {} has neighbours:", (*it));
            m_reg->patch<Cmp::Neighbours>(entt::entity(*it), [](auto &_nb_update){ _nb_update.clear(); });

            SPDLOG_TRACE("");
            const int idx = std::distance(m_random_level->begin(), it);
            
            bool has_left_map_edge = not ( (idx) % Settings::MAP_GRID_SIZE.y );
            bool has_right_map_edge = not ( (idx + 1) % Settings::MAP_GRID_SIZE.y );
            
            SPDLOG_TRACE("Entity {} has left map edge: {}", (*it), has_left_map_edge);                
            SPDLOG_TRACE("Entity {} has right map edge: {}",(*it), has_right_map_edge);  
            auto current_entity = entt::entity(*it);

            //   Columns
            // ---------------------------------------
            // |  N - (y-1) |   N - 1   |            |  Rows
            // ---------------------------------------
            // |   N - y    |     N     |            |
            // ---------------------------------------
            // | N - (y+1)  |           |            |
            // ---------------------------------------
            // where N is iterator, y is column length

            // N - 1
            if(std::prev(it) >= m_random_level->begin()) 
            {
                auto left_entt = entt::entity(*std::prev(it));
                if( m_reg->get<Cmp::Obstacle>( left_entt ).m_enabled && not has_left_map_edge ) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::LEFT, left_entt ); 
                    });
                }
            }
            // N - (y - 1)
            if( std::prev(it, (Settings::MAP_GRID_SIZE.y + 1)) >= m_random_level->begin() ) 
            {
                auto down_left_entt = entt::entity(*std::prev(it, Settings::MAP_GRID_SIZE.y + 1));
                if( m_reg->get<Cmp::Obstacle>( down_left_entt ).m_enabled && not has_left_map_edge)  
                {
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update){ 
                        _nb_update.set( Cmp::Neighbours::Dir::DOWN_LEFT, down_left_entt ); 
                    });
                }
            }
            // N - y
            if( std::prev(it, Settings::MAP_GRID_SIZE.y) >= m_random_level->begin() ) 
            {
                auto down_entt = entt::entity(*std::prev(it, Settings::MAP_GRID_SIZE.y));
                if( m_reg->get<Cmp::Obstacle>( down_entt ).m_enabled ) 
                {
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::DOWN, down_entt );
                    });
                }
            }
            // N - (y + 1)
            if( (std::prev(it, (Settings::MAP_GRID_SIZE.y - 1))) >= m_random_level->begin() ) 
            {
                auto down_right_entt = entt::entity(*std::prev(it, Settings::MAP_GRID_SIZE.y - 1));
                if( m_reg->get<Cmp::Obstacle>( down_right_entt ).m_enabled && not has_right_map_edge) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::DOWN_RIGHT, down_right_entt ); 
                    });
                }
            }

            //   Columns
            // ---------------------------------------
            // |            |           |  N + (y-1) |  Rows
            // ---------------------------------------
            // |            |     N     |    N + y   |
            // ---------------------------------------
            // |            |   N + 1   | N + (y+1)  |
            // ---------------------------------------

            // where N is iterator, y is column length

            // N + (y - 1) 
            if( std::next(it, (Settings::MAP_GRID_SIZE.y - 1)) < m_random_level->end()) 
            {
                auto top_left_entt = entt::entity(*std::next(it, Settings::MAP_GRID_SIZE.y - 1));
                if( m_reg->get<Cmp::Obstacle>( top_left_entt ).m_enabled && not has_left_map_edge) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update) 
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::UP_LEFT, top_left_entt ); 
                    });
                }
            }
            // N + y
            if( Settings::MAP_GRID_SIZE.y < m_random_level->size() && std::next(it, Settings::MAP_GRID_SIZE.y) < m_random_level->end()) 
            {
                auto top_entt = entt::entity(*std::next(it, Settings::MAP_GRID_SIZE.y));
                if( m_reg->get<Cmp::Obstacle>(  top_entt ).m_enabled ) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::UP, top_entt ); 
                    });
                }
            }
            // N + (y + 1)
            if( (Settings::MAP_GRID_SIZE.y + 1) < m_random_level->size() && std::next(it, (Settings::MAP_GRID_SIZE.y + 1)) < m_random_level->end()) 
            {
                auto top_right_entt = entt::entity(*std::next(it, (Settings::MAP_GRID_SIZE.y + 1)));
                if( m_reg->get<Cmp::Obstacle>( top_right_entt ).m_enabled && not has_right_map_edge ) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::UP_RIGHT, top_right_entt ); 
                    });
                }
            }
            // N + 1
            if( std::next(it) < m_random_level->end() ) 
            {
                auto right_entt = entt::entity(*std::next(it));
                if( m_reg->get<Cmp::Obstacle>( right_entt ).m_enabled && not has_right_map_edge )
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::RIGHT, right_entt ); 
                    });
                }
            }
                    
        }
        SPDLOG_INFO("Processed neighbours for {} entities.", m_random_level->size());
        
#ifdef NDEBUG
        
        for( auto [_entt, _ob, _pos, _nb]: m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() ) {
            // SPDLOG_INFO("Entity {} has {} neighbours", entity_trait::to_integral(_entt), _nb.count());
            std::string msg = 
                std::to_string(entity_trait::to_integral(_entt)) 
                + "("
                + std::to_string(_nb.count())
                + ") = ";

            for( auto [_dir, _nb_entt] : _nb) 
            {
                msg += "[" 
                    + _nb.to_string(_dir) 
                    + ":" 
                    + std::to_string(entity_trait::to_integral(_nb_entt)) + "] ";
            }
            SPDLOG_TRACE(msg);
        }
#endif

    }

    void apply_rules()
    {
        // 2. apply rules

        for( auto [_entt, _ob, _pos, _nb]: m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() ) {
            if( _ob.m_type == Cmp::Obstacle::Type::BEDROCK) { continue; }
            if      ( _nb.count() <= 0)                     { _ob.m_enabled = true; }
            else if ( _nb.count() > 0 and _nb.count() < 5 ) { _ob.m_enabled = false; }
            else                                            { _ob.m_enabled = true; }
        }
        SPDLOG_INFO("Finished applying Cellular Automata rules!");
    }

};

} // namespace ProceduralMaze::Systems::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__