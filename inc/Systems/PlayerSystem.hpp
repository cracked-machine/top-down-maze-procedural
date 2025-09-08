#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Movement.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <PCDetectionBounds.hpp>
#include <PlayableCharacter.hpp>
#include <Systems/BaseSystem.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys {

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( std::shared_ptr<entt::basic_registry<entt::entity>> registry )
      : BaseSystem( registry )
  {
  }

  struct Settings
  {
    int bomb_inventory{ 10 };
    int blast_radius{ 1 };
    // Maximum speed in pixels per second
    float max_speed{ 100.f };
    // Base friction coefficient when colliding
    float friction_coefficient{ 0.02f };
    // How quickly friction decreases with speed (0-1)
    float friction_falloff{ 0.5f };
    // Above Water acceleration rate
    float above_water_default_acceleration_rate{ 500.0f };
    // Above Water deceleration rate
    float above_water_default_deceleration_rate{ 600.0f };
    // Under Water acceleration rate
    float under_water_default_acceleration_rate{ 250.0f };
    // Under Water deceleration rate
    float under_water_default_deceleration_rate{ 90.0f };
  };

  PlayerSystem::Settings m_settings;

  // These arguments should be fetched from SettingsSystem
  void add_player_entity()
  {
    if ( not m_reg->view<Cmp::PlayableCharacter>()->empty() )
    {
      SPDLOG_WARN( "Player entity already exists, skipping creation" );
      return;
    }
    SPDLOG_INFO( "Creating player entity" );
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::Position>( entity, PLAYER_START_POS );

    m_reg->emplace<Cmp::PlayableCharacter>(
        entity, m_settings.bomb_inventory, m_settings.blast_radius
    );

    m_reg->emplace<Cmp::Movement>(
        entity, m_settings.max_speed, m_settings.friction_coefficient, m_settings.friction_falloff,
        m_settings.above_water_default_acceleration_rate,
        m_settings.above_water_default_deceleration_rate,
        m_settings.under_water_default_acceleration_rate,
        m_settings.under_water_default_deceleration_rate
    );
    m_reg->emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );
    m_reg->emplace<Cmp::PCDetectionBounds>(
        entity, sf::Vector2f{ Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE },
        sf::Vector2f{ Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE }
    );
  }

  void update( sf::Time deltaTime )
  {
    const float dt = deltaTime.asSeconds();

    for ( auto [entity, pc_cmp, pos_cmp, move_cmp, dir_cmp, pc_bounds] :
          m_reg
              ->view<
                  Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::Direction,
                  Cmp::PCDetectionBounds>()
              .each() )
    {
      // Apply acceleration in the desired dir_cmp
      if ( dir_cmp != sf::Vector2f( 0.0f, 0.0f ) )
      {
        move_cmp.acceleration = dir_cmp * move_cmp.acceleration_rate;
      }
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
