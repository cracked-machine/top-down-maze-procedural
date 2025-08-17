#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
#include <iterator>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Systems/ProcGen/RandomSystem.hpp>
#include <spdlog/spdlog.h>

#include <Sprites/Brick.hpp>
#include <Components/Random.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class CellAutomataSystem {
public:
    CellAutomataSystem(const RandomSystem &rs)
    : 
        m_randsys(rs)
    {

    }
    // BROKEN and V SLOW!!
    // Iterate through every node and check its geometric distance to every other element.
    // This is O(n^2) but we optimize by skipping the current==neighbour node and any disabled nodes.
    // We don't navigate any containers so this algo has additional benefit of not requiring bounds checking!
    void iterate_quadratic(entt::basic_registry<entt::entity> &reg) 
    {
        // 1. find neighbours
        int count = 0;
        using entity_trait = entt::entt_traits<entt::entity>;
        for( auto [_entt, _ob, _pos]: reg.view<Cmp::Obstacle, Cmp::Position>().each() ) {

            _ob.neighbours = 0;
            if( _ob.m_type == Cmp::Obstacle::Type::BEDROCK) { continue; }
            for( auto [_entt_nb, _ob_nb, _pos_nb]: reg.view<Cmp::Obstacle, Cmp::Position>().each() ) {
                
                // you can't be your own neighbour!
                if( _entt == _entt_nb ) { continue; }
                
                auto nb_dist = _pos - _pos_nb;
                if( not _ob_nb.m_enabled ) { continue; }
                if( static_cast<int>(abs(nb_dist.x)) < static_cast<int>((Sprites::Brick::WIDTH + (Sprites::Brick::LINEWIDTH*2))) and
                    static_cast<int>(abs(nb_dist.y)) < static_cast<int>((Sprites::Brick::HEIGHT + (Sprites::Brick::LINEWIDTH*2)))
                ) 
                {
                    count++;
                    _ob.neighbours++;
                }
            }
        }

        // 2. apply rules
        for( auto [_entt, _ob, _pos]: reg.view<Cmp::Obstacle, Cmp::Position>().each() ) {
            if( _ob.m_type == Cmp::Obstacle::Type::BEDROCK) { continue; }
            SPDLOG_INFO("Entity {} has {} neighbours", entity_trait::to_entity(_entt), _ob.neighbours);
            if      ( _ob.neighbours == 0)                          { _ob.m_enabled = true; }
            else if ( _ob.neighbours > 0 and _ob.neighbours < 5 )   { _ob.m_enabled = false; }
            else                                                    { _ob.m_enabled = true; }
        }
        SPDLOG_INFO("Total Iterations: {}", count);
    }

    
    //  1 0 1 1 0 1 1 0 1 1
    //  1 1 0 1 1 0 1 1 1 1
    //  0 1 1 0 1 1 0 0 1 1

    // if n - 1
    // {
    //      if n - 10
    //      {
    //          // always true
    //          n - (10 + 1) 
    //      }
    //      if n - (10 - 1) { }
    // }
    // if n + 1
    // {
    //      if n + 10
    //      {
    //          // always true
    //          n + (10 - 1)
    //      }
    //      if n + (10 + 1) { }
    // }


    void iterate_linear(entt::basic_registry<entt::entity> &reg)
    {
        using entity_trait = entt::entt_traits<entt::entity>;
        int count = 0;

        // 1. find neighbours
        for(auto it = m_randsys.begin(); it != m_randsys.end(); it++) {
            
            auto _ob = reg.get<Cmp::Obstacle>( entt::entity(*it) );
            reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours = 0; });
                    
            count++;
            SPDLOG_TRACE("");
            const int idx = std::distance(m_randsys.begin(), it);
            SPDLOG_TRACE("max x is {}, idx is {}", m_size.x, idx);
            
            bool has_left_map_edge = not ( (idx) % m_randsys.m_grid_size.x );
            bool has_right_map_edge = not ( (idx + 1) % m_randsys.m_grid_size.x );
            
            SPDLOG_TRACE("Entity {} has left map edge: {}", (*it), has_left_map_edge);                
            SPDLOG_TRACE("Entity {} has right map edge: {}",(*it), has_right_map_edge);  

            // -----------------------------------------
            // |           |       |           |        |
            // ------------------------------------------
            // |   N - 1   |   N   |           |        |
            // ------------------------------------------
            // | N - (x-1) | N - x | N - (x+1) |        |
            // ------------------------------------------
            // where N is iterator, x is row length
            // N - 1
            if(std::prev(it) >= m_randsys.begin()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it))).m_enabled && not has_left_map_edge ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - 1:{}", *(std::prev(it)) ); 
                }
            }
            // N - (x - 1)
            if( std::prev(it, (m_randsys.m_grid_size.x + 1)) >= m_randsys.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, m_randsys.m_grid_size.x + 1))).m_enabled && not has_left_map_edge)  {
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - (x - 1):{}", *(std::prev(it, m_size.x + 1))); 
                }
            }
            // N - x
            if( std::prev(it, m_randsys.m_grid_size.x) >= m_randsys.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, m_randsys.m_grid_size.x))).m_enabled ) {
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - x:{}", *(std::prev(it, m_size.x))); 
                }
            }
            // N - (x + 1)
            if( (std::prev(it, (m_randsys.m_grid_size.x - 1))) >= m_randsys.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, m_randsys.m_grid_size.x - 1))).m_enabled && not has_right_map_edge) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - (x + 1):{}", *(std::prev(it, m_size.x - 1)) ); 
                }
            }

            // ------------------------------------------
            // |        | N + (x-1) | N + x | N + (x+1) |
            // ------------------------------------------ 
            // |        |           |   N   |   N + 1   |
            // ------------------------------------------ 
            // |        |           |       |           |
            // ------------------------------------------
            // where N is iterator, x is row length
            
            // N + (x - 1) 
            if( std::next(it, (m_randsys.m_grid_size.x - 1)) < m_randsys.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, m_randsys.m_grid_size.x - 1))).m_enabled && not has_left_map_edge) { 
                   reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                   SPDLOG_TRACE("N + (x - 1):{}", (*std::next(it, m_size.x - 1)) ); 
                }
            }
            // N + x
            if( m_randsys.m_grid_size.x < m_randsys.size() && std::next(it, m_randsys.m_grid_size.x) < m_randsys.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, m_randsys.m_grid_size.x))).m_enabled ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N + x:{}",  (*std::next(it, m_size.x))); 
                }
            }
            // N + (x + 1)
            if( (m_randsys.m_grid_size.x + 1) < m_randsys.size() && std::next(it, (m_randsys.m_grid_size.x + 1)) < m_randsys.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, (m_randsys.m_grid_size.x + 1)))).m_enabled && not has_right_map_edge ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N + (x + 1):{}", (*std::next(it, m_size.x + 1)) ); 
                }
            }
            // N + 1
            if( std::next(it) < m_randsys.end() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it))).m_enabled && not has_right_map_edge ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                     SPDLOG_TRACE("N + 1:{}",  (*std::next(it))); 
                }
            }
                
            
            SPDLOG_DEBUG("Entity {} has {} neighbours", *it, _ob.neighbours);
        }
        // 2. apply rules

        for( auto [_entt, _ob, _pos]: reg.view<Cmp::Obstacle, Cmp::Position>().each() ) {
            if( _ob.m_type == Cmp::Obstacle::Type::BEDROCK) { continue; }
            SPDLOG_DEBUG("Entity {} has {} neighbours", entity_trait::to_entity(_entt), _ob.neighbours);
            if      ( _ob.neighbours <= 0)                          { _ob.m_enabled = true; }
            else if ( _ob.neighbours > 0 and _ob.neighbours < 5 )   { _ob.m_enabled = false; }
            else                                                    { _ob.m_enabled = true; }
        }
        SPDLOG_INFO("Total Iterations: {}", count);
  

    }
    ~CellAutomataSystem() = default;

private:
    Cmp::Random rng{0, 1};
    RandomSystem m_randsys;
};

} // namespace ProceduralMaze::Systems::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__