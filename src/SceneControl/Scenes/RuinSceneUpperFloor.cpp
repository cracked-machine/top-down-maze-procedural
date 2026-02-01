#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/WallFactory.hpp>
#include <SceneControl/Scenes/RuinSceneUpperFloor.hpp>

#include <Audio/SoundBank.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>

#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

#include <Systems/SystemStore.hpp>
#include <Utils/Constants.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneUpperFloor::on_init()
{
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneUpperFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // add two Cmp::NoPathFinding above the upper staircase landing to enforce perspective
  Factory::add_nopathfinding(
      m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 2 * Constants::kGridSquareSizePixelsF.x ), Constants::kGridSquareSizePixelsF.x } );
  Factory::add_nopathfinding(
      m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * Constants::kGridSquareSizePixelsF.x ), Constants::kGridSquareSizePixelsF.x } );

  // place the objective that was picked in RuinSceneLowerFloor scene
  auto ruin_objective_view = m_reg.view<Cmp::RuinObjectiveType>();
  SPDLOG_INFO( "ruin_objective_view: {}", ruin_objective_view->size() );
  for ( auto [ruin_obj_entt, ruin_obj_cmp] : ruin_objective_view.each() )
  {
    Factory::createCarryItem( m_reg, Cmp::Position( Utils::snap_to_grid( { 32.f, 32.f } ), Constants::kGridSquareSizePixelsF ), ruin_obj_cmp.m_type );
  }

  // spawn access hitbox just below horizontal centerpoint
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().spawn_floor_access(
      Utils::snap_to_grid( { RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * Constants::kGridSquareSizePixelsF.x ),
                             RuinSceneUpperFloor::kMapGridSizeF.y - ( 2 * Constants::kGridSquareSizePixelsF.y ) } ),
      { ( 2 * Constants::kGridSquareSizePixelsF.x ), Constants::kGridSquareSizePixelsF.y }, Cmp::RuinFloorAccess::Direction::TO_LOWER );

  // add the straircase sprite for upper floor
  const Sprites::MultiSprite &stairs_upper_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_going_down" );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().spawn_staircase_multiblock<Cmp::RuinStairsUpperMultiBlock>(
      { RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * Constants::kGridSquareSizePixelsF.x ), ( 2 * Constants::kGridSquareSizePixelsF.y ) },
      stairs_upper_ms );

  // add the straircase balustrade sprite for upper floor - make sure it is front of player
  const Sprites::MultiSprite &stairs_balustrade_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_upper_balustrade" );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().spawn_staircase_multiblock<Cmp::RuinStairsBalustradeMultiBlock>(
      { RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * Constants::kGridSquareSizePixelsF.x ), ( 2 * Constants::kGridSquareSizePixelsF.y ) },
      stairs_balustrade_ms, Utils::get_player_position( m_reg ).position.y + Constants::kGridSquareSizePixelsF.y );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, RuinSceneUpperFloor::kMapGridSize, "res/json/ruin_upper_tilemap_config.json" );
}

void RuinSceneUpperFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::remove_player_lerp_cmp( m_reg );

  auto &player_pos = Utils::get_player_position( m_reg );
  player_pos.position = Utils::snap_to_grid( player_pos.position );
  SPDLOG_INFO( "Player entered RuinSceneUpperFloor at position ({}, {})", player_pos.position.x, player_pos.position.y );

  auto player_entt = Utils::get_player_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::UPPER );

  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneUpperFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
}

void RuinSceneUpperFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  // m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_LOWER );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_starcase_multiblock_collision();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt, Sys::PlayerSystem::FootStepSfx::NONE );
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().disable_damage_cooldown();

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::OFF,
                                                                               Sys::RenderGameSystem::WeatherMode::OFF );
}

entt::registry &RuinSceneUpperFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene