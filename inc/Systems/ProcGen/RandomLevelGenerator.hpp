#ifndef __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__

#include <SFML/System/Vector2.hpp>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

#include <Components/Exit.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <map>
#include <memory>
#include <optional>

namespace ProceduralMaze::Sys::ProcGen {

class RandomLevelGenerator : public BaseSystem
{
public:
  RandomLevelGenerator( std::shared_ptr<entt::basic_registry<entt::entity>> reg )
      : BaseSystem( reg )
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
    for ( unsigned int x = 0; x < Sys::BaseSystem::kMapGridSize.x; x++ )
    {
      for ( unsigned int y = 0; y < Sys::BaseSystem::kMapGridSize.y; y++ )
      {
        auto entity = m_reg->create();
        m_reg->emplace<Cmp::Position>(
            entity,
            sf::Vector2f{
                ( x * Sprites::MultiSprite::kDefaultSpriteDimensions.x ) +
                    ( Sys::BaseSystem::kMapGridOffset.x * Sprites::MultiSprite::kDefaultSpriteDimensions.x ),
                ( y * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) +
                    ( Sys::BaseSystem::kMapGridOffset.y * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) } );
        // track the contiguous creation order of the entity so we can easily
        // find its neighbours later
        m_data.push_back( entity );
        auto sprite_factory = m_reg->ctx().get<std::shared_ptr<Sprites::SpriteFactory>>();
        if ( not sprite_factory )
        {
          SPDLOG_CRITICAL( "SpriteFactory not found in registry context" );
          std::get_terminate();
        }
        // pick a random obstacle type and texture index
        auto [obstacle_type, random_obstacle_texture_index] = sprite_factory->get_random_type_and_texture_index(
            { Sprites::SpriteFactory::SpriteMetaType::ROCK, Sprites::SpriteFactory::SpriteMetaType::POT,
              Sprites::SpriteFactory::SpriteMetaType::BONES } );
        m_reg->emplace<Cmp::Obstacle>( entity, obstacle_type, random_obstacle_texture_index, true,
                                       m_activation_selector.gen() );

        m_reg->emplace<Cmp::Neighbours>( entity );
      }
    }
  }

  // These obstacles are for the map border.
  // The textures are picked randomly, but their positions are fixed
  void gen_border()
  {
    std::size_t texture_index = 0;
    bool enabled = true;
    for ( float x = 0; x < kDisplaySize.x; x += Sprites::MultiSprite::kDefaultSpriteDimensions.x )
    {
      if ( x == 0 || x == kDisplaySize.x - Sprites::MultiSprite::kDefaultSpriteDimensions.x )
        texture_index = 2;
      else
        texture_index = 1;
      // top edge
      add_border_entity( { x, ( kMapGridOffset.y - 1 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.y },
                         texture_index );
      // bottom edge
      add_border_entity(
          { x, kMapGridOffset.y + ( ( kMapGridSize.y + 2 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) - 2 },
          texture_index );
    }
    for ( float y = 0; y < kDisplaySize.y; y += Sprites::MultiSprite::kDefaultSpriteDimensions.y )
    {
      if ( y == 0 || y == kDisplaySize.y - 1 )
        texture_index = 2;
      else if ( y == ( kDisplaySize.y / 2.f ) - Sprites::MultiSprite::kDefaultSpriteDimensions.y )
        texture_index = 3;
      else if ( y == ( kDisplaySize.y / 2.f ) )
        texture_index = 5; // closed door entrance
      else if ( y == ( kDisplaySize.y / 2.f ) + Sprites::MultiSprite::kDefaultSpriteDimensions.y )
        texture_index = 4;
      else
        texture_index = 0;
      // left edge
      add_border_entity( { 0, y }, texture_index );
      if ( y == ( kDisplaySize.y / 2.f ) )
      {
        texture_index = 6;
        enabled = false;
      } // open door exit
      // right edge
      add_border_entity( { static_cast<float>( kDisplaySize.x ) - Sprites::MultiSprite::kDefaultSpriteDimensions.x, y },
                         texture_index, enabled );
      enabled = true;
    }
  }

  void add_border_entity( const sf::Vector2f &pos, std::size_t texture_index, bool enabled = true )
  {
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::Position>( entity, pos );
    m_reg->emplace<Cmp::Obstacle>( entity, Sprites::SpriteFactory::SpriteMetaType::WALL, texture_index, true, enabled );
    m_reg->emplace<Cmp::Exit>( entity );
  }

  void stats()
  {
    std::map<std::string, int> results;
    for ( auto [entity, _pos, _ob] : m_reg->view<Cmp::Position, Cmp::Obstacle>().each() )
    {
      auto sprite_factory = m_reg->ctx().get<std::shared_ptr<Sprites::SpriteFactory>>();
      if ( not sprite_factory ) continue;
      results[sprite_factory->get_spritedata_type_string( _ob.m_type )]++;
    }
    SPDLOG_INFO( "Obstacle Pick distribution:" );
    for ( auto [bin, freq] : results )
    {
      SPDLOG_INFO( "[{}]:{}", bin, freq );
    }
  }

  std::optional<entt::entity> at( std::size_t idx )
  {
    if ( idx > m_data.size() )
      return std::nullopt;
    else
      return m_data.at( idx );
  }
  auto data() { return m_data.data(); }
  auto begin() { return m_data.begin(); }
  auto end() { return m_data.end(); }
  auto size() { return m_data.size(); }

private:
  std::vector<entt::entity> m_data;
  Cmp::Random m_activation_selector{ 0, 1 };
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__