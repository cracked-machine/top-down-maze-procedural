#ifndef __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__

#include <Neighbours.hpp>
#include <SFML/System/Vector2.hpp>
#include <Settings.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <optional>
#include <spdlog/spdlog.h>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Sprites/Brick.hpp>

namespace ProceduralMaze::Sys::ProcGen {

class RandomLevelGenerator {
public:
    RandomLevelGenerator(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        const std::vector<unsigned int> &object_tile_pool,
        const std::vector<unsigned int> &border_tile_pool,
        unsigned long seed = 0
    )
        :       m_reg(reg),
                m_activation_selector(0,1),
                m_object_tile_choices(object_tile_pool),
                m_random_object_tile_picker(0, object_tile_pool.size() - 1),
                m_border_tile_choices(border_tile_pool),
                m_random_border_tile_picker(0, border_tile_pool.size() - 1)
    {
        if (seed) Cmp::Random::seed(seed);
        gen_objects();
        gen_border();
        stats();
    }

    ~RandomLevelGenerator() = default;

    // These obstacles in the game map area.
    // The enabled status and texture of each one is picked randomly
    void gen_objects()
    {

        using entity_trait = entt::entt_traits<entt::entity>;
        for(unsigned int x = 0; x < ProceduralMaze::Settings::MAP_GRID_SIZE.x; x++)
        {
            for(unsigned int y = 0; y < ProceduralMaze::Settings::MAP_GRID_SIZE.y; y++)
            {
                auto entity = m_reg->create();
                m_reg->emplace<Cmp::Position>( 
                    entity, 
                    sf::Vector2f{
                        (x * Sprites::Brick::WIDTH)  + (ProceduralMaze::Settings::MAP_GRID_OFFSET.x * Sprites::Brick::WIDTH), 
                        (y * Sprites::Brick::HEIGHT)  + (ProceduralMaze::Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT)
                    } 
                ); 
                // track the contiguous creation order of the entity so we can easily find its neighbours later
                m_data.push_back(entity_trait::to_entity(entity));

                auto tile_pick = m_object_tile_choices[m_random_object_tile_picker.gen()];
                m_reg->emplace<Cmp::Obstacle>(entity, tile_pick, Cmp::Obstacle::Type::BRICK, true, m_activation_selector.gen() );  
                m_reg->emplace<Cmp::Neighbours>(entity);                             
            }
        }
    }

    // These obstacles are for the map border.
    // The textures are picked randomly, but their positions are fixed
    void gen_border()
    {
        using namespace ProceduralMaze::Settings;

        for(float x = 0 ; x < DISPLAY_SIZE.x; x += Sprites::Brick::WIDTH)
        {
            // top edge
            add_border_entity({
                x, 
                (MAP_GRID_OFFSET.y - 1) * Sprites::Brick::HEIGHT
            });
            // bottom edge
            add_border_entity({
                x, 
                MAP_GRID_OFFSET.y + ((MAP_GRID_SIZE.y + 2) * Sprites::Brick::HEIGHT) - 2
            });
        }
        for( float y = 0; y < DISPLAY_SIZE.y; y += Sprites::Brick::HEIGHT)
        {
            // left edge 
            add_border_entity({0, y});
            // right edge
            add_border_entity({static_cast<float>(DISPLAY_SIZE.x) -  Sprites::Brick::WIDTH, y});
        }

    }

    void add_border_entity(const sf::Vector2f &pos)
    {
        auto entity = m_reg->create();
        m_reg->emplace<Cmp::Position>(entity, pos); 
        auto tile_pick = m_border_tile_choices[m_random_border_tile_picker.gen()];
        m_reg->emplace<Cmp::Obstacle>(entity, tile_pick, Cmp::Obstacle::Type::BEDROCK, true, true );
    }

    void stats()
    {
        std::map<int, int> results;
        for(auto [entity, _pos, _ob]: m_reg->view<Cmp::Position, Cmp::Obstacle>().each()) {
            results[_ob.m_tile_pick]++;
        }
        SPDLOG_INFO("Obstacle Tile Pick distribution:");
        for(auto [bin,freq]: results)
        {
            SPDLOG_INFO("[{}]:{}", bin, freq);
        }
    }

    std::optional<uint32_t> at(std::size_t idx) 
    { 
        if( idx > m_data.size() ) return std::nullopt ;
        else return m_data.at(idx); 
    }
    auto data() { return m_data.data(); }
    auto begin() { return m_data.begin(); }
    auto end() { return m_data.end(); }
    auto size() { return m_data.size(); }
    
private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;

    std::vector<uint32_t> m_data;
    Cmp::Random m_activation_selector;

    const std::vector<unsigned int> m_object_tile_choices;
    Cmp::Random m_random_object_tile_picker;
    
    const std::vector<unsigned int> m_border_tile_choices;
    Cmp::Random m_random_border_tile_picker;
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__