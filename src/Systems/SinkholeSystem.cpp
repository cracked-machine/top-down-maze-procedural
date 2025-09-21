#include <Obstacle.hpp>
#include <Random.hpp>
#include <SinkHole.hpp>
#include <SinkholeSystem.hpp>
#include <SpriteFactory.hpp>

namespace ProceduralMaze::Sys {

void SinkholeSystem::start_sinkhole()
{
  auto sinkhole_view = m_reg->view<Cmp::SinkHole>();
  if ( std::distance( sinkhole_view.begin(), sinkhole_view.end() ) > 0 )
  {
    SPDLOG_DEBUG( "Sinkhole already seeded." );
    return;
  }

  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();

  auto obstacle_count = std::distance( obstacle_view.begin(), obstacle_view.end() );
  SPDLOG_INFO( "Found {} obstacles in the maze.", obstacle_count );

  // Get random index and advance iterator to that position
  Cmp::Random sinkhole_seed_picker = Cmp::Random( 0, static_cast<int>( obstacle_count - 1 ) );
  int random_index = sinkhole_seed_picker.gen();
  auto it = obstacle_view.begin();
  std::advance( it, random_index );

  // Get the random entity
  entt::entity random_entity = *it;

  // Now you can use random_entity for your sinkhole logic
  // For example, add SinkHole component to it:
  m_reg->emplace<Cmp::SinkHole>( random_entity );
  m_reg->remove<Cmp::Obstacle>( random_entity );
  SPDLOG_DEBUG( "Sinkhole seeded at random obstacle index {} (entity {}).", random_index,
                static_cast<uint32_t>( random_entity ) );
}

void SinkholeSystem::update_sinkhole()
{
  if ( m_clock.getElapsedTime() < m_interval_sec ) return;
  m_clock.restart();

  auto sinkhole_view = m_reg->view<Cmp::SinkHole, Cmp::Position>();
  auto sinkhole_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  auto obstacle_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
  Cmp::Random sinkhole_spread_picker = Cmp::Random( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

  for ( auto [sinkhole_entity, sinkhole_cmp, position_cmp] : sinkhole_view.each() )
  {
    if ( !sinkhole_cmp.active ) continue; // only active sinkholes can spread

    // make the sinkhole hitbox slightly larger to find adjacent obstacles
    auto sinkhole_hitbox = sf::FloatRect( position_cmp, sinkhole_hitbox_size * 2.f );
    int adjacent_sinkholes = 0;
    for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos] : obstacle_view.each() )
    {
      if ( obstacle_cmp.m_type == Sprites::SpriteFactory::SpriteMetaType::WALL ) continue; // skip walls
      auto obstacle_hitbox = sf::FloatRect( obstacle_pos, obstacle_hitbox_size );
      if ( sinkhole_hitbox.findIntersection( obstacle_hitbox ) )
      {
        // check if this obstacle already has a sinkhole component
        if ( m_reg->try_get<Cmp::SinkHole>( obstacle_entity ) == nullptr )
        {

          if ( sinkhole_spread_picker.gen() == 0 )
          {
            // add sinkhole component to this obstacle
            m_reg->emplace<Cmp::SinkHole>( obstacle_entity );
            m_reg->remove<Cmp::Obstacle>( obstacle_entity );
            SPDLOG_DEBUG( "New sinkhole created at entity {}", static_cast<uint32_t>( obstacle_entity ) );
            // only add one sinkhole per update
            return;
          }
        }
      }
    }

    for ( auto [adj_sinkhole_entity, adj_sinkhole_cmp, adj_position_cmp] : sinkhole_view.each() )
    {
      if ( sinkhole_entity == adj_sinkhole_entity ) continue; // skip self
      // check if sinkhole is adjacent to other sinkholes
      auto adj_sinkhole_hitbox = sf::FloatRect( adj_position_cmp, sinkhole_hitbox_size );
      if ( sinkhole_hitbox.findIntersection( adj_sinkhole_hitbox ) ) { adjacent_sinkholes++; }
    }
    // if the sinkhole is surrounded by sinkholes, then we can exclude it from future searches
    if ( adjacent_sinkholes >= 2 )
    {
      sinkhole_cmp.active = false;

      SPDLOG_DEBUG( "Sinkhole at entity {} is now inactive (surrounded).", static_cast<uint32_t>( sinkhole_entity ) );
    }
  }
}

} // namespace ProceduralMaze::Sys