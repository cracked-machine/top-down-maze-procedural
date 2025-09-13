#include <Persistent/PlayerMinVelocity.hpp>
#include <PlayerSystem.hpp>

namespace ProceduralMaze::Sys {

void PlayerSystem::init_context()
{
  if ( not m_reg->ctx().contains<Cmp::Persistent::BombInventory>() ) { m_reg->ctx().emplace<Cmp::Persistent::BombInventory>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::BlastRadius>() ) { m_reg->ctx().emplace<Cmp::Persistent::BlastRadius>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::WaterMaxSpeed>() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterMaxSpeed>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::LandMaxSpeed>() ) { m_reg->ctx().emplace<Cmp::Persistent::LandMaxSpeed>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::FrictionCoefficient>() ) { m_reg->ctx().emplace<Cmp::Persistent::FrictionCoefficient>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::FrictionFalloff>() ) { m_reg->ctx().emplace<Cmp::Persistent::FrictionFalloff>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::LandAcceleration>() ) { m_reg->ctx().emplace<Cmp::Persistent::LandAcceleration>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::LandDeceleration>() ) { m_reg->ctx().emplace<Cmp::Persistent::LandDeceleration>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::WaterAcceleration>() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterAcceleration>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::WaterDeceleration>() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterDeceleration>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::PCDetectionScale>() ) { m_reg->ctx().emplace<Cmp::Persistent::PCDetectionScale>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::PlayerMinVelocity>() ) { m_reg->ctx().emplace<Cmp::Persistent::PlayerMinVelocity>(); }
}

void PlayerSystem::add_player_entity()
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( entity, PLAYER_START_POS );

  auto &bomb_inventory = m_reg->ctx().get<Cmp::Persistent::BombInventory>();
  auto &blast_radius = m_reg->ctx().get<Cmp::Persistent::BlastRadius>();
  m_reg->emplace<Cmp::PlayableCharacter>( entity, bomb_inventory(), blast_radius() );

  m_reg->emplace<Cmp::Movement>( entity );
  m_reg->emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = m_reg->ctx().get<Cmp::Persistent::PCDetectionScale>();
  m_reg->emplace<Cmp::PCDetectionBounds>(
      entity, sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE }, sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE },
      pc_detection_scale()
  );
}

void PlayerSystem::update( sf::Time deltaTime )
{
  const float dt = deltaTime.asSeconds();

  for ( auto [entity, pc_cmp, pos_cmp, move_cmp, dir_cmp, pc_bounds] :
        m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::Direction, Cmp::PCDetectionBounds>().each() )
  {
    auto &land_acceleration = m_reg->ctx().get<Cmp::Persistent::LandAcceleration>();
    auto &land_deceleration = m_reg->ctx().get<Cmp::Persistent::LandDeceleration>();
    auto &water_acceleration = m_reg->ctx().get<Cmp::Persistent::WaterAcceleration>();
    auto &water_deceleration = m_reg->ctx().get<Cmp::Persistent::WaterDeceleration>();
    auto &land_max_speed = m_reg->ctx().get<Cmp::Persistent::LandMaxSpeed>();
    auto &water_max_speed = m_reg->ctx().get<Cmp::Persistent::WaterMaxSpeed>();
    auto &player_min_velocity = m_reg->ctx().get<Cmp::Persistent::PlayerMinVelocity>();

    // Apply acceleration in the desired direction
    if ( dir_cmp != sf::Vector2f( 0.0f, 0.0f ) )
    {
      if ( pc_cmp.underwater ) { move_cmp.acceleration = dir_cmp * water_acceleration(); }
      else { move_cmp.acceleration = dir_cmp * land_acceleration(); }
    }
    else
    {
      // Apply deceleration when no input
      if ( move_cmp.velocity != sf::Vector2f( 0.0f, 0.0f ) )
      {
        if ( pc_cmp.underwater ) { move_cmp.acceleration = -move_cmp.velocity.normalized() * water_deceleration(); }
        else { move_cmp.acceleration = -move_cmp.velocity.normalized() * land_deceleration(); }
      }
      else { move_cmp.acceleration = sf::Vector2f( 0.0f, 0.0f ); }
    }

    // Update velocity (change in velocity = acceleration * dt)
    move_cmp.velocity += move_cmp.acceleration * dt;

    // Stop completely if current velocity magnitude is below minimum velocity
    if ( move_cmp.velocity.length() < player_min_velocity() )
    {
      move_cmp.velocity = sf::Vector2f( 0.0f, 0.0f );
      move_cmp.acceleration = sf::Vector2f( 0.0f, 0.0f );
    }
    // Clamp velocity to max speed if current velocity magnitude exceeds max speed
    if ( pc_cmp.underwater && move_cmp.velocity.length() > water_max_speed() )
    {
      move_cmp.velocity = ( move_cmp.velocity / move_cmp.velocity.length() ) * water_max_speed();
    }
    else if ( move_cmp.velocity.length() > land_max_speed() )
    {
      move_cmp.velocity = ( move_cmp.velocity / move_cmp.velocity.length() ) * land_max_speed();
    }

    // Apply velocity to position (change in position = velocity * dt)
    pos_cmp += move_cmp.velocity * dt;
    pc_bounds.position( pos_cmp );
  }
}

} // namespace ProceduralMaze::Sys