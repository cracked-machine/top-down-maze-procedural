#include <Audio/SoundBank.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <Player/PlayerCharacter.hpp>
#include <Player/PlayerLevelDepth.hpp>
#include <ReservedPosition.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/SceneData.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Systems/Threats/WormholeSystem.hpp>
#include <Utils.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>
#include <memory>

namespace ProceduralMaze::Scene
{

void GraveyardScene::on_init()
{
  SPDLOG_INFO( "Init {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto &render_game_system = m_sys.find<Sys::Store::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // create the level contents
  m_scene_map_data = std::make_shared<SceneData>( "res/scenes/graveyard.json" );
  auto [_, player_start_pos_px] = m_scene_map_data->get_player_start_position();
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );
  auto player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_position, Constants::kGridSizePxF, 5.f, Cmp::RectBounds::ScaleAxis::XY );

  auto player_view = m_reg.view<Cmp::PlayerCharacter>();
  if ( player_view.size() == 0 ) { Factory::create_player( m_reg ); }
  else
  {
    auto &level_depth_cmp = Utils::Player::get_level_depth( m_reg );
    level_depth_cmp.increment_count( 1 );
    level_depth_cmp.display_timer.restart();
  }

  auto [map_size_grid, map_size_pixel] = m_scene_map_data->map_size();
  SPDLOG_INFO( "m_scene_map_data {},{} {},{}", map_size_grid.x, map_size_grid.y, map_size_pixel.x, map_size_pixel.y );

  auto &random_level_sys = m_sys.find<Sys::Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_game_area( *m_scene_map_data );

  random_level_sys.gen_graveyard_exterior_multiblocks();
  Factory::gen_loot_containers( m_reg, m_sprite_factory, map_size_grid );
  Factory::gen_npc_containers( m_reg, m_sprite_factory, map_size_grid );
  random_level_sys.gen_random_plants( map_size_grid );
  random_level_sys.gen_graveyard_exterior_obstacles();

  // now use cellular automata on the exterior obstacles
  auto &cellauto_parser = m_sys.find<Sys::Store::Type::CellAutomataSystem>();
  cellauto_parser.iterate( 5, Sys::ProcGen::RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR, random_level_sys.get_obstacle_sm() );

  // create a navmesh for pathfinding in the scene
  m_pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    m_pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  reinit_navmesh();

  // create floor background
  m_floormap.create( random_level_sys.get_void_sm(), m_scene_map_data );

  m_sys.find<Sys::Store::Type::ExitSystem>().spawn_exit();

  render_game_system.init_views();

  m_sys.find<Sys::Store::Type::SinkHoleHazardSystem>().init_hazard_field();
  m_sys.find<Sys::Store::Type::CorruptionHazardSystem>().init_hazard_field();
  m_sys.find<Sys::Store::Type::WormholeSystem>().spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  reinit_navmesh();

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "title_music" ).stop();
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Sound::Status::Playing )
  {
    m_sound_bank.get_music( "game_music" ).play();
    m_sound_bank.get_music( "game_music" ).setLooping( true );
  }

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::Player::remove_lerp_cmp( m_reg );

  // Respawn player back in the graveyard: either at the last position when they left, or fallback to their start position
  auto &player_pos = Utils::Player::get_position( m_reg );
  auto player_last_graveyard_pos = Utils::Player::get_last_graveyard_pos( m_reg );
  if ( player_last_graveyard_pos ) { player_pos.position = player_last_graveyard_pos->position; }
  else { player_pos.position = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg ) ); }
  SPDLOG_INFO( "Player entered graveyard at position ({}, {})", player_pos.position.x, player_pos.position.y );

  m_scene_exit_cooldown.restart();
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_sys.find<Sys::Store::Type::PlayerSystem>();
  m_player_sys.stopFootstepsSound();

  Factory::FloormapFactory::clear_floormap( m_floormap );

  m_sound_bank.get_music( "game_music" ).stop();
}

void GraveyardScene::do_update( sf::Time dt )
{
  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::SinkHoleHazardSystem>().update();
  m_sys.find<Sys::Store::Type::CorruptionHazardSystem>().update();
  m_sys.find<Sys::Store::Type::BombSystem>().update();
  m_sys.find<Sys::Store::Type::ExitSystem>().check_exit_collision();
  m_sys.find<Sys::Store::Type::ExitSystem>().check_player_can_unlock_exit();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::WormholeSystem>().check_player_wormhole_collision();
  m_sys.find<Sys::Store::Type::DiggingSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();

  if ( m_scene_exit_cooldown.getElapsedTime() >= m_scene_exit_cooldown_time )
  {
    m_sys.find<Sys::Store::Type::CryptSystem>().check_entrance_collision();
  }
  m_sys.find<Sys::Store::Type::CryptSystem>().unlock_crypt_door();
  m_sys.find<Sys::Store::Type::AltarSystem>().check_player_collision();
  m_sys.find<Sys::Store::Type::HolyWellSystem>().check_entrance_collision();
  m_sys.find<Sys::Store::Type::RuinSystem>().check_entrance_collision();

  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::DarkMode::OFF );
}

void GraveyardScene::reinit_navmesh()
{
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::BombSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::DiggingSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );
}

entt::registry &GraveyardScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene