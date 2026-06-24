#include <Audio/SoundBank.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/PathfindingFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/ShaderFactory.hpp>
#include <Inventory/PlayerInventorySlot.hpp>
#include <Inventory/WorldItem.hpp>
#include <Persistent/CryptShuffleTimeout.hpp>
#include <Player/PlayerNoPath.hpp>
#include <SceneControl/SceneData.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/LevelGenerator.hpp>
#include <Systems/ProcGen/PassageSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Stores/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Player.hpp>

namespace Game::Scene
{

void CryptScene::on_init()
{

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  m_scene_data = std::make_shared<SceneData>( "res/scenes/crypt.json" );
  SPDLOG_INFO( "get_floor_image: {}", m_scene_data->floor_tileset_image().string() );
  SPDLOG_INFO( "levelgen_tilelayer size: {}", m_scene_data->levelgen_tilelayer().size() );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene configuration (json) data
  auto [_, player_start_pos_px] = m_scene_data->get_player_start_position();
  SPDLOG_INFO( "player start position {},{}", player_start_pos_px.x, player_start_pos_px.y );
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );

  auto [map_size_grid, map_size_pixel] = m_scene_data->map_size();

  Factory::Shader::add_night_static( m_sys.find<Sys::Store::Type::ShaderSystem>(), map_size_pixel );

  // create the empty game area
  auto player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_position, Constants::kGridSizePxF, 3.f, Cmp::RectBounds::ScaleAxis::XY );
  auto &random_level_sys = m_sys.find<Sys::Store::Type::LevelGenerator>();
  random_level_sys.reset();
  random_level_sys.build_scene_from_data( *m_scene_data );
  m_sys.find<Sys::Store::Type::PassageSystem>().init_scene_data( m_scene_data );

  // intialise the game area
  auto start_room_entity = m_reg.create();
  m_reg.emplace_or_replace<Cmp::CryptRoomStart>( start_room_entity, player_start_area.position(), player_start_area.size() );
  m_sys.find<Sys::Store::Type::CryptSystem>().create_end_room( map_size_grid );
  m_sys.find<Sys::Store::Type::CryptSystem>().create_initial_closed_rooms( map_size_grid );
  m_sys.find<Sys::Store::Type::CryptSystem>().cache_all_room_connections();
  m_sys.find<Sys::Store::Type::CryptSystem>().gen_crypt_initial_interior();

  // create navmeshes for pathfinding
  m_npc_navmesh = Pathfinding::Factory::create_npc_navmesh( m_reg );
  m_open_navmesh = Pathfinding::Factory::create_open_navmesh( m_reg );
  reinit_navmesh();

  Sprites::Containers::VertexFloor floortiles;
  floortiles.create( random_level_sys.get_void_sm(), m_scene_data );
  auto floor_entity = m_reg.create();
  m_reg.emplace<Sprites::Containers::VertexFloor>( floor_entity, floortiles );
  m_reg.emplace<Cmp::ZOrderValue>( floor_entity, -16.f );

  Factory::add_inventory( m_reg, "item.candle" );
  Particle::Factory::add_flame_for_player_inventory_slot( m_reg );
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_world_view();

  if ( m_just_spawned )
  {
    auto &player_pos = Utils::Player::get_position( m_reg );
    player_pos.position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
    m_just_spawned = false;
    m_sys.find<Sys::Store::Type::CryptSystem>().setup();
    const auto kCryptShuffleTimeout = Sys::PersistSystem::get<Cmp::Persist::CryptShuffleTimeout>( m_reg ).get_value();
    Crypt::Factory::create_crypt_shuffle_timer( m_reg, kCryptShuffleTimeout );
  }

  // prevent the player from wandering off before the scene has loaded
  auto &player_dir = Utils::Player::get_direction( m_reg );
  player_dir = Cmp::Direction{ { 0.f, 0.f } };
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
  Crypt::Factory::destroy_crypt_shuffle_timer( m_reg );

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void CryptScene::do_update( sf::Time dt )
{

  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::CryptSystem>().update( dt );
  m_sys.find<Sys::Store::Type::ShockwaveSystem>().check_shockwave_player_collision();
  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );
  m_sys.find<Sys::Store::Type::PassageSystem>().update( dt );

  // Block shockwave particles from travelling through any entity that has Cmp::PlayerNoPath component
  // Don't use Cmp::NpcNoPathFinding because it blocks over lavapits.
  for ( auto [ob_entt, ob_cmp, pos_cmp] : m_reg.view<Cmp::PlayerNoPath, Cmp::Position>().each() )
  {
    m_sys.find<Sys::Store::Type::ParticleSystem>().check_collsion( pos_cmp );
  }
  m_sys.find<Sys::Store::Type::ParticleSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys );
}

void CryptScene::reinit_navmesh()
{
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_npc_navmesh, m_open_navmesh );
  m_sys.find<Sys::Store::Type::PassageSystem>().init_nav_mesh( m_npc_navmesh );
  m_sys.find<Sys::Store::Type::CryptSystem>().init( m_npc_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_npc_navmesh, m_player_navmesh, m_open_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_npc_navmesh );
}

entt::registry &CryptScene::registry() { return m_reg; }

} // namespace Game::Scene