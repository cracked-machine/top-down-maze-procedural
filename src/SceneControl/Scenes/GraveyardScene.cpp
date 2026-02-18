#include <Audio/SoundBank.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Systems/Threats/WormholeSystem.hpp>
#include <Utils/Constants.hpp>

namespace ProceduralMaze::Scene
{

void GraveyardScene::on_init()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto &render_game_system = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  auto player_start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 5.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  Factory::CreatePlayer( m_reg );

  // create the level contents
  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_circular_gamearea( GraveyardScene::kMapGridSize, player_start_area );
  random_level_sys.gen_graveyard_exterior_multiblocks();
  Factory::gen_loot_containers( m_reg, m_sprite_factory, GraveyardScene::kMapGridSize );
  Factory::gen_npc_containers( m_reg, m_sprite_factory, GraveyardScene::kMapGridSize );
  Factory::gen_random_plants( m_reg, m_sprite_factory, GraveyardScene::kMapGridSize );
  random_level_sys.gen_graveyard_exterior_obstacles();

  // now use cellular automata on the exterior obstacles
  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5, GraveyardScene::kMapGridSize, Sys::ProcGen::RandomLevelGenerator::SceneType::GRAVEYARD_EXTERIOR );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, GraveyardScene::kMapGridSize, "res/json/graveyard_tilemap_config.json" );

  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().spawn_exit();

  render_game_system.init_views();

  m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>().init_hazard_field();
  m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>().init_hazard_field();
  m_system_store.find<Sys::SystemStore::Type::WormholeSystem>().spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );

  Factory::createCarryItem( m_reg, Cmp::Position( m_player_start_position + sf::Vector2f{ 16.f, 16.f }, Constants::kGridSquareSizePixelsF ),
                            "CARRYITEM.shovel" );

  Factory::createCarryItem( m_reg, Cmp::Position( m_player_start_position - sf::Vector2f{ 16.f, 16.f }, Constants::kGridSquareSizePixelsF ),
                            "CARRYITEM.axe" );

  Factory::createCarryItem( m_reg, Cmp::Position( m_player_start_position - sf::Vector2f{ 32.f, 16.f }, Constants::kGridSquareSizePixelsF ),
                            "CARRYITEM.scryingball" );
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "title_music" ).stop();
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_music( "game_music" ).play(); }

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::remove_player_lerp_cmp( m_reg );

  // Respawn player back in the graveyard: either at the last position when they left, or fallback to their start position
  auto &player_pos = Utils::get_player_position( m_reg );
  auto player_last_graveyard_pos = Utils::get_player_last_graveyard_position( m_reg );
  if ( player_last_graveyard_pos ) { player_pos.position = player_last_graveyard_pos->position; }
  else { player_pos.position = m_player_start_position; }
  SPDLOG_INFO( "Player entered graveyard at position ({}, {})", player_pos.position.x, player_pos.position.y );

  m_scene_exit_cooldown.restart();
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.stopFootstepsSound();

  Factory::FloormapFactory::ClearFloormap( m_floormap );
}

void GraveyardScene::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::BombSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().check_exit_collision();
  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().check_player_can_unlock_exit();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::WormholeSystem>().check_player_wormhole_collision();
  m_system_store.find<Sys::SystemStore::Type::DiggingSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();

  if ( m_scene_exit_cooldown.getElapsedTime() >= m_scene_exit_cooldown_time )
  {
    m_system_store.find<Sys::SystemStore::Type::CryptSystem>().check_entrance_collision();
  }
  m_system_store.find<Sys::SystemStore::Type::CryptSystem>().unlock_crypt_door();
  m_system_store.find<Sys::SystemStore::Type::AltarSystem>().check_player_collision();
  m_system_store.find<Sys::SystemStore::Type::HolyWellSystem>().check_entrance_collision();
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().update();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );

  // clang-format off
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( 
    dt,
    m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>(), m_floormap,
    Sys::RenderGameSystem::DarkMode::OFF
  );
  // clang-format on
}

entt::registry &GraveyardScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene