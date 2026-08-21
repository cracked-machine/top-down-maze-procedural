#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/AnimData.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/Player/Character.hpp>
#include <Components/SceneSettings/Footsteps.hpp>
#include <Components/ZOrderValue.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Time.hpp>
#include <optional>

namespace
{

struct FootstepVariant
{
  size_t frame_index_offset;
  bool diagonal;
};

// tileset indices: [0: right] [1: left] [2: down] [3: up]
// diagonal footsteps reuse the cardinal sprite, rotated 45 degrees via AbsoluteRotation
std::optional<FootstepVariant> footstep_variant_for( const sf::Vector2f &dir )
{
  if ( dir == sf::Vector2f( 1.f, 0.f ) ) return FootstepVariant{ .frame_index_offset = 0, .diagonal = false };  // right
  if ( dir == sf::Vector2f( -1.f, 0.f ) ) return FootstepVariant{ .frame_index_offset = 1, .diagonal = false }; // left
  if ( dir == sf::Vector2f( 0.f, 1.f ) ) return FootstepVariant{ .frame_index_offset = 2, .diagonal = false };  // down
  if ( dir == sf::Vector2f( 0.f, -1.f ) ) return FootstepVariant{ .frame_index_offset = 3, .diagonal = false }; // up
  if ( dir == sf::Vector2f( 1.f, 1.f ) ) return FootstepVariant{ .frame_index_offset = 0, .diagonal = true };   // down/right
  if ( dir == sf::Vector2f( -1.f, 1.f ) ) return FootstepVariant{ .frame_index_offset = 2, .diagonal = true };  // down/left
  if ( dir == sf::Vector2f( -1.f, -1.f ) ) return FootstepVariant{ .frame_index_offset = 1, .diagonal = true }; // up/left
  if ( dir == sf::Vector2f( 1.f, -1.f ) ) return FootstepVariant{ .frame_index_offset = 3, .diagonal = true };  // up/right
  return std::nullopt;
}

} // namespace

namespace Game::Sys
{

void FootstepSystem::update( FootStepSfx footstep_sfx )
{

  // add new footstep for player
  auto player_view = reg().view<Cmp::Player::Character, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, player, pos_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { continue; }
    add_footstep( pos_cmp, dir_cmp );
  }

  static const unsigned int kFootstepFadeFactor{ 1 };
  auto view = reg().view<Cmp::FootStepTimer, Cmp::FootStepAlpha>();
  for ( auto [entity, timer, alpha] : view.each() )
  {
    if ( timer.m_clock.getElapsedTime() > sf::seconds( Sys::PersistSystem::get<Cmp::Persist::PlayerFootstepFadeDelay>( reg() ).get_value() ) )
    {
      alpha.m_alpha -= kFootstepFadeFactor;
      if ( alpha.m_alpha <= 0 )
      {
        SPDLOG_DEBUG( "Removing footstep entity {}", int( entity ) );
        reg().destroy( entity );
      }
    }
  }

  Utils::Player::get_direction( reg() ) == sf::Vector2f( 0.f, 0.f ) ? stop_footsteps_sound() : play_footsteps_sound( footstep_sfx );
}

void FootstepSystem::on_pause()
{
  auto footstep_timer_view = reg().view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // pause each footstep timer clock
    timer.m_clock.stop();
  }
}

void FootstepSystem::on_resume()
{
  auto footstep_timer_view = reg().view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // resume each footstep timer clock
    timer.m_clock.start();
  }
}

void FootstepSystem::add_footstep( const Cmp::Position &pos, const Cmp::Direction &dir )
{
  if ( not Utils::scene_setting<Cmp::SceneSettings::Footsteps>( reg() ).enabled ) return;
  if ( update_clock.getElapsedTime() >= sf::seconds( Sys::PersistSystem::get<Cmp::Persist::PlayerFootstepAddDelay>( reg() ).get_value() ) )
  {

    auto nopathfind_view = reg().view<Cmp::Npc::NoPathFinding, Cmp::Position>();
    for ( auto [nopath_entity, nopath_cmp, nopath_pos_cmp] : nopathfind_view.each() )
    {
      if ( nopath_pos_cmp.findIntersection( pos ) )
      {
        // Don't create footstep in no-pathfinding area
        return;
      }
    }

    auto variant = footstep_variant_for( dir );
    if ( not variant ) return;

    auto entity = reg().create();
    reg().emplace<Cmp::Direction>( entity, dir );
    reg().emplace<Cmp::FootStepTimer>( entity );
    reg().emplace<Cmp::FootStepAlpha>( entity );
    reg().emplace<Cmp::ZOrderValue>( entity, -8.f ); // footsteps render below player
    // clang-format off
    reg().emplace<Cmp::AnimData>( entity, Cmp::AnimData::Config{
          .sprite_type = "sprite.player.footsteps",
          .frame_index_offset = variant->frame_index_offset,
          .enabled = true
    });
    // clang-format on
    reg().emplace<Cmp::Position>( entity, pos.position, pos.size );

    // diagonal footsteps reuse the cardinal sprite, rotated via AbsoluteRotation
    if ( variant->diagonal )
    {
      reg().emplace<Cmp::AbsoluteRotation>( entity, 45.f );
      reg().emplace<Cmp::AbsoluteOffset>( entity, Constants::kGridSizePx.x / 2.f, Constants::kGridSizePx.y / 2.f );
    }

    // Reset the timer to enforce discrete steps
    update_clock.restart();
  }
}

void FootstepSystem::play_footsteps_sound( FootStepSfx type )
{
  switch ( type )
  {
    case FootStepSfx::NONE:
      break;
    case FootStepSfx::GRAVEL: {
      // Restarting prematurely creates a stutter effect, so check first
      auto &footsteps = m_sound_bank.get_effect( "footsteps" );
      if ( footsteps.getStatus() == sf::Sound::Status::Playing ) return;
      footsteps.play();
      break;
    }
    case FootStepSfx::FLOORBOARDS: {
      break;
    }
  }
}

void FootstepSystem::stop_footsteps_sound()
{
  // add more footstep sfx here when needed
  m_sound_bank.get_effect( "footsteps" ).stop();
}

} // namespace Game::Sys