#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/System.hpp>
#include <Constants.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <Player.hpp>
#include <SceneControl/Events/ProcessShopSceneInputEvent.hpp>
#include <SceneControl/Scenes/ShopScene.hpp>
#include <Shop/ShopInventory.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/ShopSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>

namespace ProceduralMaze::Scene
{

void ShopScene::on_init()
{
  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_scene_config = std::make_shared<SceneConfig>();
  m_scene_config->load( "res/json/shop_scene_config.json" );

  m_inventory_config = m_sys.find<Sys::Store::Type::ShopSystem>().load_config( "res/json/shop_scene_config.json" );
  auto shop_inventory_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::ShopInventory>( shop_inventory_entt, m_inventory_config );
  m_sys.find<Sys::Store::Type::ShopSystem>().create_inventory( shop_inventory_entt );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene configuration (json) data
  auto [_, player_start_pos_px] = m_scene_config->get_player_start_position();
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );
  SPDLOG_INFO( "player_start_pos_px: {},{}", player_start_pos_px.x, player_start_pos_px.y );

  auto [map_size_grid, map_size_pixel] = m_scene_config->get_map_size();

  // create the empty game area
  sf::Vector2f player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_position, Constants::kGridSizePxF, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );
  auto &random_level_sys = m_sys.find<Sys::Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( map_size_grid, player_start_area, "HOLYWELL.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  auto [exit_pos_grid, _] = m_scene_config->get_exit_position();
  m_sys.find<Sys::Store::Type::ShopSystem>().spawn_exit( exit_pos_grid );

  // add the shopkeeper NPC
  for ( auto [_, sprite_pos_pixel] : m_scene_config->get_npc_position( "NPC.dr_knox" ) )
  {
    auto npc_entt = m_reg.create();
    m_reg.emplace_or_replace<Cmp::Position>( npc_entt, sprite_pos_pixel, Constants::kGridSizePxF );
    Factory::create_npc( m_reg, npc_entt, "NPC.dr_knox" );
  }

  m_floormap.create( random_level_sys.get_void_sm(), m_scene_config );

  // create a navmesh for pathfinding in the scene
  m_pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    m_pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void ShopScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  m_sound_bank.get_music( "game_music" ).stop();

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

  auto &player_pos = Utils::Player::get_position( m_reg );
  player_pos.position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  SPDLOG_INFO( "player_start_pos_px: {},{}", player_pos.position.x, player_pos.position.y );
}

void ShopScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();

  // Hide the sudden position update/camera pan behind a forced loading screen.
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void ShopScene::do_update( [[maybe_unused]] sf::Time dt )
{
  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Sys::Store::Type::ShopSystem>().check_exit_collision();

  for ( auto [_, sprite_pos_pixel] : m_scene_config->get_npc_position( "NPC.dr_knox" ) )
  {
    if ( m_sys.find<Sys::Store::Type::ShopSystem>().check_shopkeeper_collision( sprite_pos_pixel ) and not is_overlay_open() ) { open_overlay(); }
    else if ( not m_sys.find<Sys::Store::Type::ShopSystem>().check_shopkeeper_collision( sprite_pos_pixel ) ) { close_overlay(); }
  }

  auto player_pos = Utils::Player::get_position( m_reg );
  SPDLOG_INFO( "player_start_pos_px: {},{}", player_pos.position.x, player_pos.position.y );
  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::DarkMode::OFF, Sys::WeatherMode::OFF );
}

void ShopScene::open_overlay()
{
  auto inventory_view = m_reg.view<Cmp::ShopInventory>().each();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view )
  {
    inventory_cmp.is_enabled = true;
  }
  m_overlay_open = true;
}

void ShopScene::close_overlay()
{
  auto inventory_view = m_reg.view<Cmp::ShopInventory>().each();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view )
  {
    inventory_cmp.is_enabled = false;
  }
  m_overlay_open = false;
}

entt::registry &ShopScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene