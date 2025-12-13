#include <Components/AltarMultiBlock.hpp>
#include <Components/CryptExit.hpp>
#include <Components/CryptMultiBlock.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

void CryptSystem::unlock_crypt_door()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

      // prevent spamming the activate key
      if ( m_door_cooldown_timer.getElapsedTime().asSeconds() < m_door_cooldown_time ) continue;
      m_door_cooldown_timer.restart();

      // Player can't intersect with a closed crypt door so expand their hitbox slightly to facilitate collision
      // detection
      auto increased_player_bounds = Cmp::RectBounds( pc_pos_cmp.position, pc_pos_cmp.size, 1.5f,
                                                      Cmp::RectBounds::ScaleCardinality::VERTICAL );
      if ( not increased_player_bounds.findIntersection( door_pos_cmp ) ) continue;

      // Crypt door is already opened
      if ( cryptdoor_cmp.is_open() )
      {
        // Set the z-order value
        auto crypt_view = getReg().view<Cmp::CryptMultiBlock>();
        for ( auto [crypt_entity, crypt_cmp] : crypt_view.each() )
        {
          if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
          getReg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entity, crypt_cmp.position.y - 16.f );
        }
        continue;
      }
      else
      {
        // Set the z-order value
        auto crypt_view = getReg().view<Cmp::CryptMultiBlock>();
        for ( auto [crypt_entity, crypt_cmp] : crypt_view.each() )
        {
          if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
          getReg().emplace_or_replace<Cmp::ZOrderValue>( crypt_entity, crypt_cmp.position.y + 16.f );
        }
      }

      // No keys to open the crypt door
      auto player_key_count = getReg().try_get<Cmp::PlayerKeysCount>( pc_entity );
      if ( player_key_count && player_key_count->get_count() == 0 )
      {
        m_sound_bank.get_effect( "crypt_locked" ).play();
        continue;
      }

      // unlock the crypt door
      SPDLOG_INFO( "Player unlocked a crypt door at ({}, {})", door_pos_cmp.position.x, door_pos_cmp.position.y );
      player_key_count->decrement_count( 1 );
      m_sound_bank.get_effect( "crypt_open" ).play();
      cryptdoor_cmp.set_is_open( true );

      // make doorway non-solid and lower z-order so player walks over it
      getReg().emplace_or_replace<Cmp::CryptSegment>( door_entity, Cmp::CryptSegment( false ) );

      // find the crypt multi-block this door belongs to and update the sprite
      auto crypt_view = getReg().view<Cmp::CryptMultiBlock, Cmp::SpriteAnimation>();
      for ( auto [crypt_entity, crypt_cmp, anim_cmp] : crypt_view.each() )
      {
        if ( not door_pos_cmp.findIntersection( crypt_cmp ) ) continue;
        anim_cmp.m_sprite_type = "CRYPT.opened";

        SPDLOG_INFO( "Updated crypt multi-block sprite to open state at ({}, {})", crypt_cmp.position.x,
                     crypt_cmp.position.y );
        break;
      }
    }
  }
}

void CryptSystem::check_entrance_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, crypt_door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), crypt_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds(
          crypt_door_pos_cmp.position, crypt_door_pos_cmp.size, 0.1f,
          Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      // make sure player returns just below the crypt entrance to prevent infinite re-entry
      // m_player_last_known_graveyard_pos = sf::Vector2f( pc_pos_cmp.position.x,
      //                                                   pc_pos_cmp.position.y + Constants::kGridSquareSizePixels.y );

      SPDLOG_INFO( "check_entrance_collision: Player entering crypt from graveyard at position ({}, {})",
                   pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>(
          Events::SceneManagerEvent::Type::ENTER_CRYPT );
    }
  }
}

void CryptSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto cryptdoor_view = getReg().view<Cmp::CryptExit, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, cryptdoor_cmp, crypt_door_pos_cmp] : cryptdoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), crypt_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds(
          crypt_door_pos_cmp.position, crypt_door_pos_cmp.size, 0.1f,
          Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      // // set player position as they return to the graveyard
      // pc_pos_cmp.position = sf::Vector2f(
      //     static_cast<float>( m_player_last_known_graveyard_pos.x ) * Constants::kGridSquareSizePixels.x,
      //     static_cast<float>( m_player_last_known_graveyard_pos.y ) * Constants::kGridSquareSizePixels.y );
      SPDLOG_INFO( "check_exit_collision: Player exiting crypt to graveyard at position ({}, {})",
                   pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_CRYPT );
    }
  }
}

void CryptSystem::spawn_exit( sf::Vector2u spawn_position )
{

  sf::FloatRect spawn_pos_px = sf::FloatRect(
      { static_cast<float>( spawn_position.x ) * Constants::kGridSquareSizePixels.x,
        static_cast<float>( spawn_position.y ) * Constants::kGridSquareSizePixels.y },
      Constants::kGridSquareSizePixelsF );

  // remove any wall
  for ( auto [entt, wall_cmp, pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
  {
    if ( spawn_pos_px.findIntersection( pos_cmp ) ) { getReg().destroy( entt ); }
  }

  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, spawn_pos_px.position, Constants::kGridSquareSizePixelsF );
  getReg().emplace_or_replace<Cmp::Exit>( entity, false ); // unlocked at start
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", 1 );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.position.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
  getReg().emplace_or_replace<Cmp::CryptExit>( entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

} // namespace ProceduralMaze::Sys