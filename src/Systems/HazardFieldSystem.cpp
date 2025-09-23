#include <HazardFieldCell.hpp>
#include <HazardFieldSystem.hpp>
#include <Obstacle.hpp>
#include <Random.hpp>
#include <SpriteFactory.hpp>

namespace ProceduralMaze::Sys {

void HazardFieldSystem::start_hazard_field()
{
  auto hazard_field_view = m_reg->view<Cmp::HazardFieldCell>();
  if ( std::distance( hazard_field_view.begin(), hazard_field_view.end() ) > 0 )
  {
    SPDLOG_DEBUG( "Hazard field already seeded." );
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

  // Now you can use random_entity for your hazard field logic
  // For example, add HazardFieldCell component to it:
  m_reg->emplace<Cmp::HazardFieldCell>( random_entity );
  m_reg->remove<Cmp::Obstacle>( random_entity );
  SPDLOG_DEBUG( "Hazard field seeded at random obstacle index {} (entity {}).", random_index,
                static_cast<uint32_t>( random_entity ) );
}

void HazardFieldSystem::update_hazard_field()
{
  if ( m_clock.getElapsedTime() < m_interval_sec ) return;
  m_clock.restart();

  auto hazard_field_view = m_reg->view<Cmp::HazardFieldCell, Cmp::Position>();
  auto hazard_field_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  auto obstacle_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
  Cmp::Random hazard_field_spread_picker = Cmp::Random( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

  for ( auto [hazard_field_entity, hazard_field_cmp, position_cmp] : hazard_field_view.each() )
  {
    if ( !hazard_field_cmp.active ) continue; // only active hazard fields can spread

    // make the hazard field hitbox slightly larger to find adjacent obstacles
    auto hazard_field_hitbox = sf::FloatRect( position_cmp, hazard_field_hitbox_size * 2.f );
    int adjacent_hazard_fields = 0;
    for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos] : obstacle_view.each() )
    {
      if ( obstacle_cmp.m_type == Sprites::SpriteFactory::SpriteMetaType::WALL ) continue; // skip walls
      auto obstacle_hitbox = sf::FloatRect( obstacle_pos, obstacle_hitbox_size );
      if ( hazard_field_hitbox.findIntersection( obstacle_hitbox ) )
      {
        // check if this obstacle already has a hazard field component
        if ( m_reg->try_get<Cmp::HazardFieldCell>( obstacle_entity ) == nullptr )
        {

          if ( hazard_field_spread_picker.gen() == 0 )
          {
            // add hazard field component to this obstacle
            m_reg->emplace<Cmp::HazardFieldCell>( obstacle_entity );
            m_reg->remove<Cmp::Obstacle>( obstacle_entity );
            SPDLOG_DEBUG( "New hazard field created at entity {}", static_cast<uint32_t>( obstacle_entity ) );
            // only add one hazard field per update
            return;
          }
        }
      }
    }

    for ( auto [adj_hazard_field_entity, adj_hazard_field_cmp, adj_position_cmp] : hazard_field_view.each() )
    {
      if ( hazard_field_entity == adj_hazard_field_entity ) continue; // skip self
      // check if hazard field is adjacent to other hazard fields
      auto adj_hazard_field_hitbox = sf::FloatRect( adj_position_cmp, hazard_field_hitbox_size );
      if ( hazard_field_hitbox.findIntersection( adj_hazard_field_hitbox ) ) { adjacent_hazard_fields++; }
    }
    // if the hazard field is surrounded by hazard fields, then we can exclude it from future searches
    if ( adjacent_hazard_fields >= 2 )
    {
      hazard_field_cmp.active = false;

      SPDLOG_DEBUG( "Hazard field at entity {} is now inactive (surrounded).",
                    static_cast<uint32_t>( hazard_field_entity ) );
    }
  }
}

} // namespace ProceduralMaze::Sys