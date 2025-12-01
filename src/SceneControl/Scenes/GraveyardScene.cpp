#include <Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )

    : m_sound_bank( sound_bank ),
      m_system_store( system_store ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void GraveyardScene::on_init()
{
  // initialize any entities or components that should exist before on_enter
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &render_game_system = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();
  render_game_system.init_tilemap();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.addPlayerEntity();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate();

  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5 );

  auto &exit_sys = m_system_store.find<Sys::SystemStore::Type::ExitSystem>();
  exit_sys.spawn_exit();
  render_game_system.init_views();

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.init_hazard_field();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.init_hazard_field();

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.stopFootstepsSound();

  auto &m_render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  m_render_game_sys.clear_tilemap();
}

void GraveyardScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_music( "title_music" ).stop();
  // only do this once every update, other it constantly restarts the music
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Music::Status::Playing ) { m_sound_bank.get_music( "game_music" ).play(); }

  auto &anim_sys = m_system_store.find<Sys::SystemStore::Type::AnimSystem>();
  anim_sys.update( dt );

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.update_hazard_field();
  sinkhole_sys.check_npc_hazard_field_collision();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.update_hazard_field();
  corruption_sys.check_npc_hazard_field_collision();

  auto &bomb_sys = m_system_store.find<Sys::SystemStore::Type::BombSystem>();
  bomb_sys.update();

  auto &exit_sys = m_system_store.find<Sys::SystemStore::Type::ExitSystem>();
  exit_sys.check_exit_collision();

  auto &loot_sys = m_system_store.find<Sys::SystemStore::Type::LootSystem>();
  loot_sys.check_loot_collision();

  auto &npc_sys = m_system_store.find<Sys::SystemStore::Type::NpcSystem>();
  npc_sys.update( dt );

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.check_player_wormhole_collision();

  auto &digging_sys = m_system_store.find<Sys::SystemStore::Type::DiggingSystem>();
  digging_sys.update();

  auto &footstep_sys = m_system_store.find<Sys::SystemStore::Type::FootstepSystem>();
  footstep_sys.update();

  // enable/disable collision detection depending on Cmp::System settings
  auto &player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  for ( auto [_ent, _sys] : m_reg.view<Cmp::System>().each() )
  {
    player_sys.update( dt, !_sys.collisions_enabled );
    if ( _sys.collisions_enabled )
    {
      sinkhole_sys.check_player_hazard_field_collision();
      corruption_sys.check_player_hazard_field_collision();
      npc_sys.check_player_to_npc_collision();
    }
  }

  auto &render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  auto &render_overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  render_game_sys.refresh_z_order_queue();
  render_game_sys.render_game( dt, render_overlay_sys );

  // defer this scenes input event processing until we  exit this function
  m_nav_event_dispatcher.enqueue( Events::ProcessGraveyardSceneInputEvent() );
}

entt::registry &GraveyardScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene