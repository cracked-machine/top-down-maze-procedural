#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Movement.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/BlastRadius.hpp>
#include <Persistent/BombInventory.hpp>
#include <Persistent/FrictionCoefficient.hpp>
#include <Persistent/FrictionFalloff.hpp>
#include <Persistent/LandAcceleration.hpp>
#include <Persistent/LandDeacceleration.hpp>
#include <Persistent/PlayerMaxSpeed.hpp>
#include <Persistent/WaterAcceleration.hpp>
#include <Persistent/WaterDeacceleration.hpp>
#include <PlayableCharacter.hpp>
#include <Systems/BaseSystem.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys {

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( std::shared_ptr<entt::basic_registry<entt::entity>> registry ) : BaseSystem( registry ) {}

  void init_context()
  {
    if ( m_reg->view<Cmp::Persistent::BombInventory>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::BombInventory>(); }
    if ( m_reg->view<Cmp::Persistent::BlastRadius>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::BlastRadius>(); }
    if ( m_reg->view<Cmp::Persistent::PlayerMaxSpeed>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::PlayerMaxSpeed>(); }
    if ( m_reg->view<Cmp::Persistent::FrictionCoefficient>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::FrictionCoefficient>(); }
    if ( m_reg->view<Cmp::Persistent::FrictionFalloff>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::FrictionFalloff>(); }
    if ( m_reg->view<Cmp::Persistent::LandAcceleration>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::LandAcceleration>(); }
    if ( m_reg->view<Cmp::Persistent::LandDeceleration>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::LandDeceleration>(); }
    if ( m_reg->view<Cmp::Persistent::WaterAcceleration>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterAcceleration>(); }
    if ( m_reg->view<Cmp::Persistent::WaterDeceleration>()->empty() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterDeceleration>(); }
  }

  // These arguments should be fetched from SettingsSystem
  void add_player_entity()
  {
    SPDLOG_INFO( "Creating player entity" );
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::Position>( entity, PLAYER_START_POS );

    auto &bomb_inventory = m_reg->ctx().get<Cmp::Persistent::BombInventory>();
    auto &blast_radius = m_reg->ctx().get<Cmp::Persistent::BlastRadius>();
    m_reg->emplace<Cmp::PlayableCharacter>( entity, bomb_inventory(), blast_radius() );

    auto &max_speed = m_reg->ctx().get<Cmp::Persistent::PlayerMaxSpeed>();
    auto &friction_coefficient = m_reg->ctx().get<Cmp::Persistent::FrictionCoefficient>();
    auto &friction_falloff = m_reg->ctx().get<Cmp::Persistent::FrictionFalloff>();
    auto &land_acceleration = m_reg->ctx().get<Cmp::Persistent::LandAcceleration>();
    auto &land_deceleration = m_reg->ctx().get<Cmp::Persistent::LandDeceleration>();
    auto &water_acceleration = m_reg->ctx().get<Cmp::Persistent::WaterAcceleration>();
    auto &water_deceleration = m_reg->ctx().get<Cmp::Persistent::WaterDeceleration>();

    m_reg->emplace<Cmp::Movement>(
        entity, max_speed(), friction_coefficient(), friction_falloff(), land_acceleration(), land_deceleration(), water_acceleration(),
        water_deceleration()
    );

    m_reg->emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );
    m_reg->emplace<Cmp::PCDetectionBounds>(
        entity, sf::Vector2f{ Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE }, sf::Vector2f{ Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE }
    );
  }

  void update( sf::Time deltaTime )
  {
    const float dt = deltaTime.asSeconds();

    for ( auto [entity, pc_cmp, pos_cmp, move_cmp, dir_cmp, pc_bounds] :
          m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::Direction, Cmp::PCDetectionBounds>().each() )
    {
      // Apply acceleration in the desired dir_cmp
      if ( dir_cmp != sf::Vector2f( 0.0f, 0.0f ) ) { move_cmp.acceleration = dir_cmp * move_cmp.acceleration_rate; }
      else
      {
        // Apply deceleration when no input
        if ( move_cmp.velocity != sf::Vector2f( 0.0f, 0.0f ) )
        {
          move_cmp.acceleration = -move_cmp.velocity.normalized() * move_cmp.deceleration_rate;
        }
        else { move_cmp.acceleration = sf::Vector2f( 0.0f, 0.0f ); }
      }

      // Update velocity (change in velocity = acceleration * dt)
      move_cmp.velocity += move_cmp.acceleration * dt;

      // Stop completely if current velocity magnitude is below minimum velocity
      if ( move_cmp.velocity.length() < move_cmp.min_velocity )
      {
        move_cmp.velocity = sf::Vector2f( 0.0f, 0.0f );
        move_cmp.acceleration = sf::Vector2f( 0.0f, 0.0f );
      }
      // Clamp velocity to max speed if current velocity magnitude exceeds max
      // speed
      else if ( move_cmp.velocity.length() > move_cmp.max_speed )
      {
        move_cmp.velocity = ( move_cmp.velocity / move_cmp.velocity.length() ) * move_cmp.max_speed;
      }

      // Apply velocity to position (change in position = velocity * dt)
      pos_cmp += move_cmp.velocity * dt;
      pc_bounds.position( pos_cmp );
    }
  }
};

} // namespace ProceduralMaze::Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
