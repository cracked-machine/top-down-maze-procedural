#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Scene
{

void GraveyardScene::on_init()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );

  auto &render_game_system = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();

  Factory::CreatePlayer( m_reg );

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate( Sys::ProcGen::RandomLevelGenerator::AreaShape::RECTANGLE, GraveyardScene::kMapGridSize,
                             true, true, true );

  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5, GraveyardScene::kMapGridSize );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, GraveyardScene::kMapGridSize );

  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().spawn_exit();

  render_game_system.init_views();

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.init_hazard_field();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.init_hazard_field();

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "title_music" ).stop();
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Sound::Status::Playing )
  {
    m_sound_bank.get_music( "game_music" ).play();
  }

  auto player_view = m_reg.view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( auto [player_entity, pc_cmp, pos_cmp] : player_view.each() )
  {
    pos_cmp.position = m_player_start_position;
    SPDLOG_INFO( "Player entered graveyard at position ({}, {})", pos_cmp.position.x, pos_cmp.position.y );

    // Clear any ongoing lerp to prevent position interpolation
    if ( m_reg.any_of<Cmp::LerpPosition>( player_entity ) )
    {
      m_reg.remove<Cmp::LerpPosition>( player_entity );
      SPDLOG_INFO( "Cleared LerpPosition component to prevent position interpolation" );
    }
  }

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
  auto check_position = [this]( [[maybe_unused]] const std::string &system_name )
  {
    auto player_view = m_reg.view<Cmp::PlayableCharacter, Cmp::Position>();
    for ( auto [player_entity, pc_cmp, pos_cmp] : player_view.each() )
    {
      static sf::Vector2f last_position = pos_cmp.position;
      if ( pos_cmp.position != last_position )
      {
        SPDLOG_DEBUG( "Player position changed after {}: ({}, {}) -> ({}, {})", system_name, last_position.x,
                      last_position.y, pos_cmp.position.x, pos_cmp.position.y );
        last_position = pos_cmp.position;
      }
    }
  };

  check_position( "START" );

  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  check_position( "AnimSystem" );

  m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>().update();
  check_position( "SinkHoleHazardSystem" );

  m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>().update();
  check_position( "CorruptionHazardSystem" );

  m_system_store.find<Sys::SystemStore::Type::BombSystem>().update();
  check_position( "BombSystem" );

  m_system_store.find<Sys::SystemStore::Type::ExitSystem>().check_exit_collision();
  check_position( "ExitSystem" );

  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  check_position( "LootSystem" );

  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  check_position( "NpcSystem" );

  m_system_store.find<Sys::SystemStore::Type::WormholeSystem>().check_player_wormhole_collision();
  check_position( "WormholeSystem" );

  m_system_store.find<Sys::SystemStore::Type::DiggingSystem>().update();
  check_position( "DiggingSystem" );

  m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  check_position( "FootstepSystem" );

  if ( m_scene_exit_cooldown.getElapsedTime() >= m_scene_exit_cooldown_time )
  {
    m_system_store.find<Sys::SystemStore::Type::CryptSystem>().check_entrance_collision();
  }
  check_position( "CryptSystem" );

  // Note: this enqueues 'Events::SceneManagerEvent::Type::GAME_OVER' if player is dead
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt );
  check_position( "PlayerSystem" );

  // clang-format off
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( 
    dt,
    m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>(), m_floormap
  );
  // clang-format on
  check_position( "RenderGameSystem" );
}

entt::registry &GraveyardScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene