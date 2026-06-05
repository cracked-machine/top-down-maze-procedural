#include <Audio/SoundBank.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/System.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Sprites/Shockwave.hpp>
#include <Stats/ProjectileAction.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <Systems/Threats/ShockwaveSystem.hpp>

namespace Game::Sys
{

ShockwaveSystem::ShockwaveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                  Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

Sprites::Shockwave::CircleSegments ShockwaveSystem::split_segment_by_obstacle( const Sprites::CircleSegment &segment,
                                                                               const sf::FloatRect &obstacle_rect, sf::Vector2f shockwave_position,
                                                                               float radius, const int samples )
{
  Sprites::Shockwave::CircleSegments result;

  float angle_range = segment.get_end_angle() - segment.get_start_angle();
  const float inv_s = 1.0f / static_cast<float>( samples - 1 );

  int start_idx = -1;
  for ( int i = 0; i < samples; ++i )
  {
    float angle = segment.get_start_angle() + ( static_cast<float>( i ) * inv_s * angle_range );
    sf::Vector2f point = shockwave_position + sf::Vector2f( std::cos( angle ) * radius, std::sin( angle ) * radius );
    bool inside = obstacle_rect.contains( point );

    if ( !inside && start_idx == -1 )
    {
      start_idx = i;
    }
    else if ( inside && start_idx != -1 )
    {
      float start_angle = segment.get_start_angle() + ( static_cast<float>( start_idx ) * inv_s * angle_range );
      float end_angle = segment.get_start_angle() + ( static_cast<float>( i - 1 ) * inv_s * angle_range );
      if ( end_angle > start_angle ) { result.emplace_back( start_angle, end_angle, true ); }
      start_idx = -1;
    }
  }

  if ( start_idx != -1 )
  {
    float start_angle = segment.get_start_angle() + ( static_cast<float>( start_idx ) * inv_s * angle_range );
    result.emplace_back( start_angle, segment.get_end_angle(), true );
  }

  return result;
}

bool ShockwaveSystem::intersects_with_visible_segments( const Cmp::NpcShockwave &shockwave, const sf::FloatRect &player_pos )
{
  sf::Vector2f position = shockwave.sprite.get_position();
  float radius = shockwave.sprite.get_radius();
  float outline_thickness = shockwave.sprite.get_outline_thickness();
  int points_per_segment = shockwave.sprite.get_points_per_segment();

  for ( const auto &segment : shockwave.sprite.get_visible_segments() )
  {
    float angle_range = segment.get_end_angle() - segment.get_start_angle();

    // Check both inner and outer radius points to account for thickness
    float inner_radius = radius - ( outline_thickness / 2.0f );
    float outer_radius = radius + ( outline_thickness / 2.0f );

    for ( int i = 0; i < points_per_segment; ++i )
    {
      float t = static_cast<float>( i ) / static_cast<float>( points_per_segment - 1 );
      float angle = segment.get_start_angle() + ( t * angle_range );

      auto cos_angle = std::cos( angle );
      auto sin_angle = std::sin( angle );
      sf::Vector2f inner_point = position + sf::Vector2f( cos_angle * inner_radius, sin_angle * inner_radius );
      sf::Vector2f outer_point = position + sf::Vector2f( cos_angle * outer_radius, sin_angle * outer_radius );

      if ( player_pos.contains( inner_point ) || player_pos.contains( outer_point ) )
      {
        // do shockwave/player knockback
        sf::Vector2f shockwave_direction( cos_angle, sin_angle );

        auto &player_pos_cmp = Utils::Player::get_position( reg() );
        auto new_position = Utils::snap_to_grid( player_pos_cmp.position + ( shockwave_direction.componentWiseMul( Constants::kGridSizePxF ) ) );
        SPDLOG_DEBUG( "Player position was {},{} - Knockback direction is {}, {} - New Position should be {},{}", player_pos_cmp.position.x,
                      player_pos_cmp.position.y, normalised_direction.x, normalised_direction.y, new_position.x, new_position.y );

        // make sure player isnt knocked into an obstacle
        bool is_valid = true;
        for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : reg().view<Cmp::Obstacle, Cmp::Position>().each() )
        {
          if ( sf::FloatRect( new_position, Constants::kGridSizePxF ).findIntersection( obstacle_pos_cmp ) ) is_valid = false;
        }
        if ( is_valid ) { player_pos_cmp.position = new_position; }
        else { SPDLOG_DEBUG( "New Position was invalid so cancelled" ); }

        return true;
      }
    }
  }
  return false;
}

void ShockwaveSystem::remove_intersecting_segments( const sf::FloatRect &rect, Cmp::NpcShockwave &shockwave )
{
  Sprites::Shockwave::CircleSegments new_segments;

  for ( const auto &segment : shockwave.sprite.get_segments() )
  {
    if ( not segment.is_visible() ) continue;

    // Instead of deleting segments, rebuild the segment list without the intersecting segments
    Sprites::Shockwave::CircleSegments non_intersecting = Sys::ShockwaveSystem::split_segment_by_obstacle(
        segment, rect, shockwave.sprite.get_position(), shockwave.sprite.get_radius(), shockwave.sprite.get_points_per_segment() );
    new_segments.insert( new_segments.end(), non_intersecting.begin(), non_intersecting.end() );
  }

  shockwave.sprite.set_segments( std::move( new_segments ) );
}

void ShockwaveSystem::check_shockwave_player_collision()
{
  if ( Utils::getSystemCmp( reg() ).collisions_disabled ) return;

  // we need the projectile_action modifiers for this NPC type.
  auto priest_npc_cmp = Sys::NpcStore::instance().get_item( "npc.priest" );
  auto priest_projectile_action = priest_npc_cmp.actions.at( std::type_index( typeid( Cmp::ProjectileAction ) ) );
  auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( reg() );
  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PlayerStats, Cmp::PlayerMortality>();

  for ( auto entt : reg().view<Cmp::NpcShockwave>() )
  {
    Cmp::NpcShockwave &shockwave = reg().get<Cmp::NpcShockwave>( entt );

    for ( auto [player_entity, player_cmp, player_pos, player_stats_cmp, player_mort_cmp] : player_view.each() )
    {
      // dont spam death events if the player is already dead
      if ( player_mort_cmp.state == Cmp::PlayerMortality::State::DEAD ) continue;
      if ( player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() ) continue;
      if ( intersects_with_visible_segments( shockwave, player_pos ) )
      {
        player_stats_cmp.apply_modifiers( priest_projectile_action.action );
        m_sound_bank.get_effect( "damage_player" ).play();
        player_cmp.m_damage_cooldown_timer.restart();
        SPDLOG_INFO( "Player (health:{}) INTERSECTS with Shockwave (position: {},{} - effective_radius: {})", player_stats_cmp.health(),
                     shockwave.sprite.get_position().x, shockwave.sprite.get_position().y, shockwave.sprite.get_radius() );

        // trigger death animation
        if ( player_stats_cmp.health() <= 0 )
        {
          get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SHOCKED, player_pos ) );
        }
      }
      else { SPDLOG_DEBUG( "Player does NOT intersect with shockwave (effective_radius: {})", shockwave.sprite.getRadius() ); }
    }
  }
}

} // namespace Game::Sys