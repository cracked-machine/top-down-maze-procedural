#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scenes/RuinSceneLowerFloor.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneLowerFloor::on_init()
{
  auto gridsize = Constants::kGridSizePxF;
  using SystemStoreType = Sys::SystemStore::Type;

  auto &m_persistent_sys = m_system_store.find<SystemStoreType::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_door_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, gridsize, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  // select the objective type that will be spawned in the RuinSceneUpperFloor scene
  auto selected_objective_ms_type = m_sprite_factory.get_random_type( { "CARRYITEM.boots", "CARRYITEM.witchesjar", "CARRYITEM.preservedcat" },
                                                                      { 1, 1, 1 } );
  auto ruin_objective_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::RuinObjectiveType>( ruin_objective_entt, selected_objective_ms_type );

  // generate the scene boundaries
  auto &random_level_sys = m_system_store.find<SystemStoreType::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneLowerFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // pass concrete spawn position to exit spawner
  m_system_store.find<SystemStoreType::HolyWellSystem>().spawn_exit(
      sf::Vector2u{ RuinSceneLowerFloor::kMapGridSize.x / 3, RuinSceneLowerFloor::kMapGridSize.y - 1 } );

  // spawn access hitbox just above horizontal centerpoint
  sf::Vector2f flooraccess_position( RuinSceneLowerFloor::kMapGridSizeF.x - ( 3 * gridsize.x ), 2 * gridsize.y );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_system_store.find<SystemStoreType::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size,
                                                                         Cmp::RuinFloorAccess::Direction::TO_UPPER );

  // add the straircase sprite for lower floor
  const Sprites::MultiSprite &stairs_ms = m_sprite_factory.get_multisprite_by_type( "RUIN.interior_staircase_going_up" );
  sf::Vector2f stairs_position( RuinSceneLowerFloor::kMapGridSizeF.x - ( 4 * gridsize.x ), gridsize.y );
  m_system_store.find<SystemStoreType::RuinSystem>().add_stairs<Cmp::RuinStairsLowerMultiBlock>( stairs_position, stairs_ms );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, RuinSceneLowerFloor::kMapGridSize, "res/json/ruin_lower_tilemap_config.json" );

  sf::Vector2f bc_area_position( 0, 0 );
  sf::Vector2f bc_area_size( RuinSceneLowerFloor::kMapGridSizeF.x - 48, RuinSceneLowerFloor::kMapGridSizeF.y - 32 );
  m_system_store.find<SystemStoreType::RuinSystem>().gen_lowerfloor_bookcases( sf::FloatRect( bc_area_position, bc_area_size ) );

  sf::Vector2f cobweb_area_position( 0, 0 );
  sf::Vector2f cobweb_area_size( RuinSceneLowerFloor::kMapGridSizeF.x - 48, RuinSceneLowerFloor::kMapGridSizeF.y - 32 );
  m_system_store.find<SystemStoreType::RuinSystem>().add_lowerfloor_cobwebs( sf::FloatRect( cobweb_area_position, cobweb_area_size ) );

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::Player::remove_player_lerp_cmp( m_reg );

  auto &player_pos = Utils::Player::get_player_position( m_reg );
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

  auto player_entt = Utils::Player::get_player_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::LOWER );

  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneLowerFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  // m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().check_exit_collision();
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_UPPER );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_movement_slowdowns();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt, Sys::PlayerSystem::FootStepSfx::NONE );
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().disable_damage_cooldown();

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::OFF,
                                                                               Sys::RenderGameSystem::WeatherMode::OFF );
}

entt::registry &RuinSceneLowerFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene