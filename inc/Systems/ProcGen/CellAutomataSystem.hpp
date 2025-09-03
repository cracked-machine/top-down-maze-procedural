#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>

#include <iterator>
#include <memory>

namespace ProceduralMaze::Sys::ProcGen {

class CellAutomataSystem : public BaseSystem {
public:
    CellAutomataSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::unique_ptr<RandomLevelGenerator> random_level)
    : BaseSystem(reg),
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
    std::unique_ptr<RandomLevelGenerator> m_random_level;

    void find_neighbours()
    {  
        

        // 1. find neighbours
        for(auto it = m_random_level->begin(); it != m_random_level->end(); it++) {
            if (!m_reg->valid(entt::entity(*it))) 
            {
                SPDLOG_WARN("Entity {} is not valid! Valid entities are:", entt::to_integral(*it));
                std::string valid_entities;
                for([[maybe_unused]] auto entity: m_reg->view<entt::entity>()) { 
                    valid_entities += " " + std::to_string(entt::to_integral(entity)); }
                SPDLOG_WARN("{}", valid_entities); 
            }


            SPDLOG_TRACE("Entity {} has neighbours:", entt::to_integral(*it));
            m_reg->patch<Cmp::Neighbours>(entt::entity(*it), [](auto &_nb_update){ _nb_update.clear(); });

            SPDLOG_TRACE("");
            const int idx = std::distance(m_random_level->begin(), it);
            
            bool has_left_map_edge = not ( (idx) % MAP_GRID_SIZE.y );
            bool has_right_map_edge = not ( (idx + 1) % MAP_GRID_SIZE.y );

            SPDLOG_TRACE("Entity {} has left map edge: {}", entt::to_integral(*it), has_left_map_edge);
            SPDLOG_TRACE("Entity {} has right map edge: {}", entt::to_integral(*it), has_right_map_edge);   
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
                Cmp::Obstacle* left_entt_ob = m_reg->try_get<Cmp::Obstacle>( left_entt );
                if( not left_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on left entity [N - 1] neighbour: {}", entt::to_integral(left_entt));
                    assert(false && "No Obstacle component found on left entity [N - 1] neighbour: " && entt::to_integral(left_entt));
                }
                else if( left_entt_ob && left_entt_ob->m_enabled && not has_left_map_edge ) { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update) { 
                        _nb_update.set( Cmp::Neighbours::Dir::LEFT, left_entt ); 
                    });
                }
            }
            // N - (y - 1)
            if( std::prev(it, (MAP_GRID_SIZE.y + 1)) >= m_random_level->begin() ) 
            {
                auto down_left_entt = entt::entity(*std::prev(it, MAP_GRID_SIZE.y + 1));
                Cmp::Obstacle* down_left_entt_ob = m_reg->try_get<Cmp::Obstacle>( down_left_entt );
                if( not down_left_entt_ob )
                {
                    SPDLOG_WARN("No Obstacle component found on down left entity [N - (y - 1)] neighbour: {}", entt::to_integral(down_left_entt));
                    assert(false && "No Obstacle component found on down left entity [N - (y - 1)] neighbour: " && entt::to_integral(down_left_entt));
                }
                else if( down_left_entt_ob->m_enabled && not has_left_map_edge)  
                {
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update){ 
                        _nb_update.set( Cmp::Neighbours::Dir::DOWN_LEFT, down_left_entt ); 
                    });
                }
            }
            // N - y
            if( std::prev(it, MAP_GRID_SIZE.y) >= m_random_level->begin() ) 
            {
                auto down_entt = entt::entity(*std::prev(it, MAP_GRID_SIZE.y));
                Cmp::Obstacle* down_entt_ob = m_reg->try_get<Cmp::Obstacle>( down_entt );
                if( not down_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on down entity [N - y] neighbour: {}", entt::to_integral(down_entt));
                    assert(false && "No Obstacle component found on down entity [N - y] neighbour: " && entt::to_integral(down_entt));
                }
                else if( down_entt_ob->m_enabled ) 
                {
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::DOWN,  down_entt );
                    });
                }
            }
            // N - (y + 1)
            if( (std::prev(it, (MAP_GRID_SIZE.y - 1))) >= m_random_level->begin() ) 
            {
                auto down_right_entt = entt::entity(*std::prev(it, MAP_GRID_SIZE.y - 1));
                Cmp::Obstacle* down_right_entt_ob = m_reg->try_get<Cmp::Obstacle>( down_right_entt );
                if( not down_right_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on down right entity [N - (y + 1)] neighbour: {}", entt::to_integral(down_right_entt));
                    assert(false && "No Obstacle component found on down right entity [N - (y + 1)] neighbour: " && entt::to_integral(down_right_entt));
                }
                else if( down_right_entt_ob->m_enabled && not has_right_map_edge) 
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
            if( std::next(it, (MAP_GRID_SIZE.y - 1)) < m_random_level->end()) 
            {
                auto top_left_entt = entt::entity(*std::next(it, MAP_GRID_SIZE.y - 1));
                Cmp::Obstacle* top_left_entt_ob = m_reg->try_get<Cmp::Obstacle>( top_left_entt );
                if( not top_left_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on top left entity [N + (y - 1)] neighbour: {}", entt::to_integral(top_left_entt));
                    assert(false && "No Obstacle component found on top left entity [N + (y - 1)] neighbour: " && entt::to_integral(top_left_entt));
                }
                else if( top_left_entt_ob->m_enabled && not has_left_map_edge) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update) 
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::UP_LEFT, top_left_entt ); 
                    });
                }
            }
            // N + y
            if( MAP_GRID_SIZE.y < m_random_level->size() && std::next(it, MAP_GRID_SIZE.y) < m_random_level->end()) 
            {
                auto top_entt = entt::entity(*std::next(it, MAP_GRID_SIZE.y));
                Cmp::Obstacle* top_entt_ob = m_reg->try_get<Cmp::Obstacle>( top_entt );
                if( not top_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on top entity [N + y] neighbour: {}", entt::to_integral(top_entt));
                    assert(false && "No Obstacle component found on top entity [N + y] neighbour: " && entt::to_integral(top_entt));
                }
                else if( top_entt_ob->m_enabled ) 
                { 
                    m_reg->patch<Cmp::Neighbours>(current_entity, [&](auto &_nb_update)
                    { 
                        _nb_update.set( Cmp::Neighbours::Dir::UP, top_entt ); 
                    });
                }
            }
            // N + (y + 1)
            if( (MAP_GRID_SIZE.y + 1) < m_random_level->size() && std::next(it, (MAP_GRID_SIZE.y + 1)) < m_random_level->end()) 
            {
                auto top_right_entt = entt::entity(*std::next(it, (MAP_GRID_SIZE.y + 1)));
                Cmp::Obstacle* top_right_entt_ob = m_reg->try_get<Cmp::Obstacle>( top_right_entt );
                if( not top_right_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on top right entity [N + (y + 1)] neighbour: {}", entt::to_integral(top_right_entt));
                    assert(false && "No Obstacle component found on top right entity [N + (y + 1)] neighbour: " && entt::to_integral(top_right_entt));
                }
                else if( top_right_entt_ob->m_enabled && not has_right_map_edge ) 
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
                Cmp::Obstacle* right_entt_ob = m_reg->try_get<Cmp::Obstacle>( right_entt );
                if( not right_entt_ob ) {
                    SPDLOG_WARN("No Obstacle component found on right entity [N + 1] neighbour: {}", entt::to_integral(right_entt));
                    assert(false && "No Obstacle component found on right entity [N + 1] neighbour: " && entt::to_integral(right_entt));
                }
                else if( right_entt_ob->m_enabled && not has_right_map_edge )
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
            // SPDLOG_INFO("Entity {} has {} neighbours", entt::to_integral(_entt), _nb.count());
            std::string msg = 
                std::to_string(entt::to_integral(_entt)) 
                + "("
                + std::to_string(_nb.count())
                + ") = ";

            for( auto [_dir, _nb_entt] : _nb) 
            {
                msg += "[" 
                    + _nb.to_string(_dir) 
                    + ":" 
                    + std::to_string(entt::to_integral(_nb_entt)) + "] ";
            }
            SPDLOG_TRACE(msg);
        }
#endif

    }

    void apply_rules()
    {
        // 2. apply rules

        for( auto [_entt, _ob, _pos, _nb]: m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() ) {
            if( _ob.m_type == Sprites::SpriteFactory::Type::WALL) { continue; }
            if      ( _nb.count() <= 2)                     { _ob.m_enabled = true; }
            else if ( _nb.count() > 2 and _nb.count() < 5 ) { _ob.m_enabled = false; }
            else                                            { _ob.m_enabled = true; }
        }
        SPDLOG_INFO("Finished applying Cellular Automata rules!");
    }

};

} // namespace ProceduralMaze::Systems::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__