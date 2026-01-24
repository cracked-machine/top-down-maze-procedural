#include <Components/HolyWell/HolyWellEntrance.hpp>
#include <Components/HolyWell/HolyWellExit.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Wall.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Utils/Optimizations.hpp>

namespace ProceduralMaze::Sys
{

void HolyWellSystem::spawn_exit( sf::Vector2u spawn_position )
{

  sf::FloatRect spawn_pos_px = sf::FloatRect( { static_cast<float>( spawn_position.x ) * Constants::kGridSquareSizePixels.x,
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
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "CRYPT.interior_sb", 1 );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.position.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
  getReg().emplace_or_replace<Cmp::HolyWellExit>( entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

void HolyWellSystem::spawn_well( sf::Vector2u spawn_position )
{

  const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "HOLYWELL.interior_well" );
  Cmp::Position spawn_pos_px( { static_cast<float>( spawn_position.x ) * Constants::kGridSquareSizePixels.x,
                                static_cast<float>( spawn_position.y ) * Constants::kGridSquareSizePixels.y },
                              ms.getSpriteSizePixels() );

  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, spawn_pos_px.position, spawn_pos_px.size );

  Factory::createMultiblock<Cmp::HolyWellMultiBlock>( getReg(), entity, spawn_pos_px, ms );
  Factory::createMultiblockSegments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( getReg(), entity, spawn_pos_px, ms );

  SPDLOG_INFO( "Well spawned at position ({}, {})", spawn_pos_px.position.x, spawn_pos_px.position.y );
  return;
}

void HolyWellSystem::check_entrance_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto holywelldoor_view = getReg().view<Cmp::HollyWellEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, holywelldoor_cmp, holywell_door_pos_cmp] : holywelldoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), holywell_door_pos_cmp ) ) continue;

      // shrink entrance bounds slightly for better UX
      Cmp::RectBounds decreased_entrance_bounds( holywell_door_pos_cmp.position, holywell_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH );

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_entrance_collision: Player entering holywell from graveyard at position ({}, {})", pc_pos_cmp.position.x,
                   pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_HOLYWELL );
    }
  }
}

void HolyWellSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto holywelldoor_view = getReg().view<Cmp::HolyWellExit, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, holywelldoor_cmp, holywell_door_pos_cmp] : holywelldoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), holywell_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds( holywell_door_pos_cmp.position, holywell_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_exit_collision: Player exiting holywell to graveyard at position ({}, {})", pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_HOLYWELL );
    }
  }
}

} // namespace ProceduralMaze::Sys