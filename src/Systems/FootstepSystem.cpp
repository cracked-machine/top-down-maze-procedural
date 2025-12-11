#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/SpawnArea.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/FootstepSystem.hpp>

namespace ProceduralMaze::Sys
{

void FootstepSystem::onPause()
{
  auto footstep_timer_view = getReg().view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // pause each footstep timer clock
    timer.m_clock.stop();
  }
}

void FootstepSystem::onResume()
{
  auto footstep_timer_view = getReg().view<Cmp::FootStepTimer>();
  for ( auto [entity, timer] : footstep_timer_view.each() )
  {
    // resume each footstep timer clock
    timer.m_clock.start();
  }
}

void FootstepSystem::add_footstep( const Cmp::Position &pos_cmp, const Cmp::Direction &direction )
{
  if ( update_clock.getElapsedTime() >=
       sf::seconds(
           get_persistent_component<Cmp::Persistent::PlayerFootstepAddDelay>().get_value() ) )
  {

    auto nopathfind_view = getReg().view<Cmp::NoPathFinding, Cmp::Position>();
    for ( auto [nopath_entity, nopath_cmp, nopath_pos_cmp] : nopathfind_view.each() )
    {
      if ( nopath_pos_cmp.findIntersection( pos_cmp ) )
      {
        // Don't create footstep in no-pathfinding area
        return;
      }
    }

    auto entity = getReg().create();
    getReg().emplace<Cmp::Direction>( entity, direction );
    getReg().emplace<Cmp::FootStepTimer>( entity );
    getReg().emplace<Cmp::FootStepAlpha>( entity );
    getReg().emplace<Cmp::ZOrderValue>(
        entity,
        pos_cmp.position.y - Constants::kGridSquareSizePixelsF.y ); // footsteps render below player
    // select the footstep tilset index based on the players current direction
    // tileset indices: [0: right] [1: left] [2: down] [3: up]
    // We use absolute rotation component to rotate diagonal footsteps from the "right" sprite
    if ( direction == sf::Vector2f( 1.f, 0.f ) )
    { // moving right
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 0 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      // getReg().emplace<Cmp::AbsoluteOffset>( entity, 0.f, -3.f );
    }
    else if ( direction == sf::Vector2f( -1.f, 0.f ) )
    { // moving left
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 1 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      // getReg().emplace<Cmp::AbsoluteOffset>( entity, 0.f, -3.f );
    }
    else if ( direction == sf::Vector2f( 0.f, 1.f ) )
    { // moving down
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 2 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
    }
    else if ( direction == sf::Vector2f( 0.f, -1.f ) )
    { // moving up
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 3 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
    }
    else if ( direction == sf::Vector2f( 1.f, 1.f ) )
    { // moving down/right
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 0 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      getReg().emplace<Cmp::AbsoluteRotation>( entity, 45.f );
      getReg().emplace<Cmp::AbsoluteOffset>( entity, Constants::kGridSquareSizePixels.x / 2.f,
                                             Constants::kGridSquareSizePixels.y / 2.f );
    }
    else if ( direction == sf::Vector2f( -1.f, 1.f ) )
    { // moving down/left
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 2 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      getReg().emplace<Cmp::AbsoluteRotation>( entity, 45.f );
      getReg().emplace<Cmp::AbsoluteOffset>( entity, Constants::kGridSquareSizePixels.x / 2.f,
                                             Constants::kGridSquareSizePixels.y / 2.f );
    }
    else if ( direction == sf::Vector2f( -1.f, -1.f ) )
    { // moving up/left
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 1 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      getReg().emplace<Cmp::AbsoluteRotation>( entity, 45.f );
      getReg().emplace<Cmp::AbsoluteOffset>( entity, Constants::kGridSquareSizePixels.x / 2.f,
                                             Constants::kGridSquareSizePixels.y / 2.f );
    }
    else if ( direction == sf::Vector2f( 1.f, -1.f ) )
    { // moving up/right
      getReg().emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "FOOTSTEPS", 3 );
      getReg().emplace<Cmp::Position>(
          entity, sf::Vector2f{ pos_cmp.position.x, pos_cmp.position.y }, pos_cmp.size );
      getReg().emplace<Cmp::AbsoluteRotation>( entity, 45.f );
      getReg().emplace<Cmp::AbsoluteOffset>( entity, Constants::kGridSquareSizePixels.x / 2.f,
                                             Constants::kGridSquareSizePixels.y / 2.f );
    }

    // Reset the timer to enforce discrete steps
    update_clock.restart();
  }
}

void FootstepSystem::update()
{

  // add new footstep for player
  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, player, pos_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { continue; }
    add_footstep( pos_cmp, dir_cmp );
  }

  auto view = getReg().view<Cmp::FootStepTimer, Cmp::FootStepAlpha>();
  for ( auto [entity, timer, alpha] : view.each() )
  {
    if ( timer.m_clock.getElapsedTime() >
         sf::seconds(
             get_persistent_component<Cmp::Persistent::PlayerFootstepFadeDelay>().get_value() ) )
    {
      alpha.m_alpha -= kFootstepFadeFactor;
      if ( alpha.m_alpha <= 0.f )
      {
        SPDLOG_DEBUG( "Removing footstep entity {}", int( entity ) );
        getReg().destroy( entity );
      }
    }
  }
}

} // namespace ProceduralMaze::Sys