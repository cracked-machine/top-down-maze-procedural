#include <Audio/SoundBank.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scenes/RuinSceneLowerFloor.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneLowerFloor::on_init()
{
  auto gridsize = Constants::kGridSizePxF;
  using SystemStoreType = Sys::Store::Type;

  auto &m_persistent_sys = m_sys.find<SystemStoreType::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_door_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, gridsize, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  // select the objective type that will be spawned in the RuinSceneUpperFloor scene
  auto selected_objective_ms_type = m_sprite_factory.get_random_type( { "CARRYITEM.witchesjar" } );
  auto ruin_objective_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::RuinObjectiveType>( ruin_objective_entt, selected_objective_ms_type );

  // generate the scene boundaries
  auto &random_level_sys = m_sys.find<SystemStoreType::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneLowerFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // pass concrete spawn position to exit spawner
  m_sys.find<SystemStoreType::HolyWellSystem>().spawn_exit(
      sf::Vector2u{ RuinSceneLowerFloor::kMapGridSize.x / 3, RuinSceneLowerFloor::kMapGridSize.y - 1 } );

  // spawn access hitbox just above horizontal centerpoint
  sf::Vector2f flooraccess_position( kMapGridSizeF.x - ( 3 * gridsize.x ), 2 * gridsize.y );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_sys.find<SystemStoreType::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size, Cmp::RuinFloorAccess::Direction::TO_UPPER );

  // add the straircase sprite for lower floor
  const Sprites::MultiSprite &stairs_ms = m_sprite_factory.get_multisprite_by_type( "RUIN.interior_staircase_going_up" );
  sf::Vector2f stairs_position( kMapGridSizeF.x - ( 4 * gridsize.x ), gridsize.y );
  m_sys.find<SystemStoreType::RuinSystem>().add_stairs<Cmp::RuinStairsLowerMultiBlock>( stairs_position, stairs_ms );

  // Make sure bookcaseses cant block access to the staircase
  Factory::add_reservedposition( m_reg, { kMapGridSizeF.x - ( 5 * gridsize.x ), kMapGridSizeF.y - ( 2 * gridsize.x ) } );
  Factory::add_reservedposition( m_reg, { kMapGridSizeF.x - ( 5 * gridsize.x ), kMapGridSizeF.y - ( 3 * gridsize.x ) } );

  Factory::FloormapFactory::create_floormap( m_reg, m_floormap, RuinSceneLowerFloor::kMapGridSize, "res/json/ruin_lower_tilemap_config.json" );

  sf::Vector2f bc_area_position( 0, 0 );
  sf::Vector2f bc_area_size( kMapGridSizeF.x - 48, kMapGridSizeF.y - 16 );
  m_sys.find<SystemStoreType::RuinSystem>().gen_lowerfloor_bookcases( sf::FloatRect( bc_area_position, bc_area_size ) );

  sf::Vector2f cobweb_area_position( 0, 0 );
  sf::Vector2f cobweb_area_size( kMapGridSizeF.x - 48, kMapGridSizeF.y - 32 );
  m_sys.find<SystemStoreType::RuinSystem>().add_lowerfloor_cobwebs( 200, sf::FloatRect( cobweb_area_position, cobweb_area_size ) );

  m_sys.find<Sys::Store::Type::RuinSystem>().reset_player_curse();

  // create a spatial grid of the game area
  auto view = m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> );
  for ( auto entity : view )
  {
    const auto &pos = view.get<Cmp::Position>( entity );
    m_spatial_grid.insert( entity, pos );
  }

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "game_music" ).stop();
  if ( not m_sys.find<Sys::Store::Type::RuinSystem>().is_player_carrying_witches_jar() )
  {
    if ( m_sound_bank.get_music( "ruin_creaking_rope" ).getStatus() != sf::Sound::Status::Playing )
    {
      m_sound_bank.get_music( "ruin_creaking_rope" ).play();
      m_sound_bank.get_music( "ruin_creaking_rope" ).setLooping( true );
    }
    if ( m_sound_bank.get_music( "ruin_music" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_music( "ruin_music" ).play(); }
  }

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

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

  m_sys.find<Sys::Store::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneLowerFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_sound_bank.get_music( "ruin_creaking_rope" ).stop();
  m_sound_bank.get_music( "ruin_music" ).stop();

  m_reg.clear();

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void RuinSceneLowerFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  using namespace Sys;
  m_sys.find<Store::Type::AnimSystem>().update( dt );
  m_sys.find<Store::Type::NpcSystem>().update( dt, &m_spatial_grid );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Store::Type::LootSystem>().check_loot_collision();

  m_sys.find<Store::Type::RuinSystem>().update( &m_spatial_grid );
  m_sys.find<Store::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_UPPER );
  m_sys.find<Store::Type::RuinSystem>().check_movement_slowdowns();
  m_sys.find<Store::Type::RuinSystem>().creaking_rope_update();

  m_sys.find<Store::Type::PlayerSystem>().update( dt, &m_spatial_grid, Sys::PlayerSystem::FootStepSfx::NONE );
  m_sys.find<Store::Type::PlayerSystem>().disable_damage_cooldown();

  bool is_player_cursed = m_sys.find<Sys::Store::Type::RuinSystem>().check_activate_player_curse( kMapGridSizeF );
  if ( is_player_cursed ) { m_sys.find<Store::Type::RuinSystem>().check_create_witch( m_reg, sf::FloatRect( { 0, 0 }, kMapGridSizeF ) ); }

  // `check_exit_collision()` may reset the player curse so it must be called after `check_activate_player_curse()`
  // or we incorrectly re-trigger the curse effects before we can leave this function and exit the scene.
  // i.e. the witch scream triggers again as we leave the scene
  m_sys.find<Store::Type::RuinSystem>().check_exit_collision();

  auto &overlay_sys = m_sys.find<Store::Type::RenderOverlaySystem>();
  m_sys.find<Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, m_spatial_grid, DarkMode::OFF, WeatherMode::OFF,
                                                           ( is_player_cursed ? CursedMode::ON : CursedMode::OFF ) );
}

entt::registry &RuinSceneLowerFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene