#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/System.hpp>
#include <SceneControl/Scenes/RuinSceneLowerFloor.hpp>

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
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneLowerFloor::on_init()
{
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_door_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  // select the objective type that will be spawned in the RuinSceneUpperFloor scene
  auto selected_objective_ms_type = m_sprite_factory.get_random_type( { "CARRYITEM.boots", "CARRYITEM.witchesjar", "CARRYITEM.preservedcat" },
                                                                      { 1, 1, 1 } );
  auto ruin_objective_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::RuinObjectiveType>( ruin_objective_entt, selected_objective_ms_type );

  // generate the scene boundaries
  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneLowerFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // pass concrete spawn position to exit spawner
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().spawn_exit(
      sf::Vector2u{ RuinSceneLowerFloor::kMapGridSize.x / 2, RuinSceneLowerFloor::kMapGridSize.y - 1 } );

  // spawn access hitbox just above horizontal centerpoint
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().spawn_floor_access(
      Utils::snap_to_grid(
          { RuinSceneLowerFloor::kMapGridSizeF.x - ( 3 * Constants::kGridSquareSizePixelsF.x ), 2 * Constants::kGridSquareSizePixelsF.y } ),
      { ( 2 * Constants::kGridSquareSizePixelsF.x ), Constants::kGridSquareSizePixelsF.y }, Cmp::RuinFloorAccess::Direction::TO_UPPER );

  // add the straircase sprite for lower floor
  const Sprites::MultiSprite &stairs_ms = m_sprite_factory.get_multisprite_by_type( "RUIN.interior_staircase_going_up" );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().spawn_staircase_multiblock<Cmp::RuinStairsLowerMultiBlock>(
      { RuinSceneLowerFloor::kMapGridSizeF.x - ( 4 * Constants::kGridSquareSizePixelsF.x ), Constants::kGridSquareSizePixelsF.y }, stairs_ms );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, RuinSceneLowerFloor::kMapGridSize, "res/json/ruin_lower_tilemap_config.json" );
}

void RuinSceneLowerFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::remove_player_lerp_cmp( m_reg );

  auto &player_pos = Utils::get_player_position( m_reg );
  switch ( m_entry_mode )
  {
    case EntryMode::FROM_DOOR: {
      SPDLOG_INFO( "Player entering from door" );
      player_pos.position = Utils::snap_to_grid( m_player_door_position );
      break;
    }
    case EntryMode::FROM_UPPER_FLOOR: {
      SPDLOG_INFO( "Player entering from upper floor" );
      player_pos.position = Utils::snap_to_grid( player_pos.position );
      break;
    }
  }
  SPDLOG_INFO( "Player entered RuinSceneLowerFloor at position ({}, {})", player_pos.position.x, player_pos.position.y );

  auto player_entt = Utils::get_player_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::LOWER );

  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneLowerFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
}

void RuinSceneLowerFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  // m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().check_exit_collision();
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_UPPER );

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt, Sys::PlayerSystem::FootStepSfx::NONE );

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::OFF,
                                                                               Sys::RenderGameSystem::WeatherMode::OFF );
}

entt::registry &RuinSceneLowerFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene