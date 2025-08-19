#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <ProcGen/RandomObstacleGenerator.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/registry.hpp>
#include <iterator>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <spdlog/spdlog.h>

#include <Sprites/Brick.hpp>
#include <Components/Random.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class CellAutomataSystem {
public:
    CellAutomataSystem(const RandomObstacleGenerator &rs)
    : 
        m_obs_gen(rs)
    {

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
        for(auto it = m_obs_gen.begin(); it != m_obs_gen.end(); it++) {
            
            auto _ob = reg.get<Cmp::Obstacle>( entt::entity(*it) );
            reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours = 0; });
                    
            count++;
            SPDLOG_TRACE("");
            const int idx = std::distance(m_obs_gen.begin(), it);
            SPDLOG_TRACE("max x is {}, idx is {}", m_size.x, idx);
            
            bool has_left_map_edge = not ( (idx) % ProceduralMaze::Settings::MAP_GRID_SIZE.x );
            bool has_right_map_edge = not ( (idx + 1) % ProceduralMaze::Settings::MAP_GRID_SIZE.x );
            
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
            if(std::prev(it) >= m_obs_gen.begin()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it))).m_enabled && not has_left_map_edge ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - 1:{}", *(std::prev(it)) ); 
                }
            }
            // N - (x - 1)
            if( std::prev(it, (ProceduralMaze::Settings::MAP_GRID_SIZE.x + 1)) >= m_obs_gen.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x + 1))).m_enabled && not has_left_map_edge)  {
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - (x - 1):{}", *(std::prev(it, m_size.x + 1))); 
                }
            }
            // N - x
            if( std::prev(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x) >= m_obs_gen.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x))).m_enabled ) {
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N - x:{}", *(std::prev(it, m_size.x))); 
                }
            }
            // N - (x + 1)
            if( (std::prev(it, (ProceduralMaze::Settings::MAP_GRID_SIZE.x - 1))) >= m_obs_gen.begin() ) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::prev(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x - 1))).m_enabled && not has_right_map_edge) { 
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
            if( std::next(it, (ProceduralMaze::Settings::MAP_GRID_SIZE.x - 1)) < m_obs_gen.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x - 1))).m_enabled && not has_left_map_edge) { 
                   reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                   SPDLOG_TRACE("N + (x - 1):{}", (*std::next(it, m_size.x - 1)) ); 
                }
            }
            // N + x
            if( ProceduralMaze::Settings::MAP_GRID_SIZE.x < m_obs_gen.size() && std::next(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x) < m_obs_gen.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, ProceduralMaze::Settings::MAP_GRID_SIZE.x))).m_enabled ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N + x:{}",  (*std::next(it, m_size.x))); 
                }
            }
            // N + (x + 1)
            if( (ProceduralMaze::Settings::MAP_GRID_SIZE.x + 1) < m_obs_gen.size() && std::next(it, (ProceduralMaze::Settings::MAP_GRID_SIZE.x + 1)) < m_obs_gen.end()) {
                if( reg.get<Cmp::Obstacle>( entt::entity(*std::next(it, (ProceduralMaze::Settings::MAP_GRID_SIZE.x + 1)))).m_enabled && not has_right_map_edge ) { 
                    reg.patch<Cmp::Obstacle>(entt::entity(*it), [](auto &_ob_update){ _ob_update.neighbours++; });
                    SPDLOG_TRACE("N + (x + 1):{}", (*std::next(it, m_size.x + 1)) ); 
                }
            }
            // N + 1
            if( std::next(it) < m_obs_gen.end() ) {
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

    // "Optimized" by claude sonnet 3.5. Might be cleaner looking.
    // Actually runs slightly slower than the original iterate_linear!
    void iterate_linear_optimized(entt::basic_registry<entt::entity> &reg)
    {
        using entity_trait = entt::entt_traits<entt::entity>;
        const int width = ProceduralMaze::Settings::MAP_GRID_SIZE.x;
        
        // Pre-calculate neighbor offsets
        static const std::array<int, 8> neighbor_offsets = {
            -1,                  // left
            1,                   // right
            -width,             // up
            width,              // down
            -(width + 1),       // up-left
            -(width - 1),       // up-right
            (width - 1),        // down-left
            (width + 1)         // down-right
        };

        // First pass: count neighbors
        std::vector<int> neighbor_counts(m_obs_gen.size(), 0);
        int idx = 0;
        int count = 0;
        
        for(auto it = m_obs_gen.begin(); it != m_obs_gen.end(); ++it, ++idx, ++count) {
            const bool has_left_edge = (idx % width) == 0;
            const bool has_right_edge = ((idx + 1) % width) == 0;
            
            const auto current_entity = entt::entity(*it);
            
            for(size_t n = 0; n < neighbor_offsets.size(); ++n) {
                // Skip edge cases
                if((has_left_edge && (n == 0 || n == 4 || n == 6)) ||
                   (has_right_edge && (n == 1 || n == 5 || n == 7)))
                    continue;
                    
                const int neighbor_idx = idx + neighbor_offsets[n];
                if(neighbor_idx >= 0 && neighbor_idx < static_cast<int>(m_obs_gen.size())) {
                    auto neighbor_it = m_obs_gen.begin() + neighbor_idx;
                    if(reg.get<Cmp::Obstacle>(entt::entity(*neighbor_it)).m_enabled) {
                        neighbor_counts[idx]++;
                    }
                }
            }
        }

        // Second pass: apply rules
        idx = 0;
        for(auto it = m_obs_gen.begin(); it != m_obs_gen.end(); ++it, ++idx) {
            auto &obstacle = reg.get<Cmp::Obstacle>(entt::entity(*it));
            if(obstacle.m_type == Cmp::Obstacle::Type::BEDROCK) continue;
            
            const int neighbors = neighbor_counts[idx];
            obstacle.neighbours = neighbors; // Update neighbor count
            
            // Apply cellular automata rules
            obstacle.m_enabled = (neighbors <= 0 || neighbors >= 5);
        }
        SPDLOG_INFO("Total Iterations: {}", count);

#ifdef _DEBUG
        SPDLOG_INFO("Total cells processed: {}", m_obs_gen.size());
#endif
  

    }
    ~CellAutomataSystem() = default;

private:
    Cmp::Random rng{0, 1};
    RandomObstacleGenerator m_obs_gen;
};

} // namespace ProceduralMaze::Systems::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__