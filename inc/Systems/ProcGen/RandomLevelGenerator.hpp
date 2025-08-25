#ifndef __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__

#include <MultiSprite.hpp>
#include <Neighbours.hpp>
#include <SFML/System/Vector2.hpp>
#include <Settings.hpp>
#include <SpriteFactory.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>


namespace ProceduralMaze::Sys::ProcGen {

class RandomLevelGenerator {
public:
    RandomLevelGenerator(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::shared_ptr<Sprites::SpriteFactory> sprite_factory
    )
    : m_reg(reg),
      m_sprite_factory(sprite_factory)
    {
        gen_objects();
        gen_border();
        stats();
    }

    ~RandomLevelGenerator() = default;

    // These obstacles in the game map area.
    // The enabled status and texture of each one is picked randomly
    void gen_objects()
    {
        for(unsigned int x = 0; x < ProceduralMaze::Settings::MAP_GRID_SIZE.x; x++)
        {
            for(unsigned int y = 0; y < ProceduralMaze::Settings::MAP_GRID_SIZE.y; y++)
            {
                auto entity = m_reg->create();
                m_reg->emplace<Cmp::Position>( 
                    entity, 
                    sf::Vector2f{
                        (x * m_sprite_factory->DEFAULT_SPRITE_SIZE.x)  + (ProceduralMaze::Settings::MAP_GRID_OFFSET.x * m_sprite_factory->DEFAULT_SPRITE_SIZE.x), 
                        (y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y)  + (ProceduralMaze::Settings::MAP_GRID_OFFSET.y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y)
                    } 
                ); 
                // track the contiguous creation order of the entity so we can easily find its neighbours later
                m_data.push_back(entity);

                auto obstacle_metadata = m_sprite_factory->get_random_metadata(
                    {Sprites::SpriteFactory::Type::ROCK, Sprites::SpriteFactory::Type::POT, Sprites::SpriteFactory::Type::BONES});
                if (!obstacle_metadata) {
                    SPDLOG_WARN("Failed to get random obstacle metadata");
                    continue;
                }
                auto random_obstacle_texture_index = obstacle_metadata->pick_random_texture_index();
                m_reg->emplace<Cmp::Obstacle>(entity, obstacle_metadata->get_type(), random_obstacle_texture_index, true, m_activation_selector.gen());

                m_reg->emplace<Cmp::Neighbours>(entity);
            }
        }
    }

    // These obstacles are for the map border.
    // The textures are picked randomly, but their positions are fixed
    void gen_border()
    {
        using namespace ProceduralMaze::Settings;

        for(float x = 0 ; x < DISPLAY_SIZE.x; x += m_sprite_factory->DEFAULT_SPRITE_SIZE.x)
        {
            // top edge
            add_border_entity({
                x, 
                (MAP_GRID_OFFSET.y - 1) * m_sprite_factory->DEFAULT_SPRITE_SIZE.y
            });
            // bottom edge
            add_border_entity({
                x, 
                MAP_GRID_OFFSET.y + ((MAP_GRID_SIZE.y + 2) * m_sprite_factory->DEFAULT_SPRITE_SIZE.y) - 2
            });
        }
        for( float y = 0; y < DISPLAY_SIZE.y; y += m_sprite_factory->DEFAULT_SPRITE_SIZE.y)
        {
            // left edge 
            add_border_entity({0, y});
            // right edge
            add_border_entity({static_cast<float>(DISPLAY_SIZE.x) -  m_sprite_factory->DEFAULT_SPRITE_SIZE.x, y});
        }

    }

    void add_border_entity(const sf::Vector2f &pos)
    {
        auto entity = m_reg->create();
        m_reg->emplace<Cmp::Position>(entity, pos);
        auto wall_ms = m_sprite_factory->get_metadata_by_type(Sprites::SpriteFactory::Type::WALL);
        if( not wall_ms ) { SPDLOG_CRITICAL("Unable to get WALL multisprite from SpriteFactory"); std::get_terminate(); }
        m_reg->emplace<Cmp::Obstacle>(entity, Sprites::SpriteFactory::Type::WALL, wall_ms->pick_random_texture_index(), true, true);
    }

    void stats()
    {
        std::map<std::string, int> results;
        for(auto [entity, _pos, _ob]: m_reg->view<Cmp::Position, Cmp::Obstacle>().each()) {
            results[m_sprite_factory->get_metadata_type_string(_ob.m_type)]++;
        }
        SPDLOG_INFO("Obstacle Pick distribution:");
        for(auto [bin,freq]: results)
        {
            SPDLOG_INFO("[{}]:{}", bin, freq);
        }
    }

    std::optional<entt::entity> at(std::size_t idx) 
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
    std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory;

    std::vector<entt::entity> m_data;
    Cmp::Random m_activation_selector{0,1};

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__