#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Player.hpp>

namespace ProceduralMaze::Scene
{

void CryptScene::on_init()
{

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  auto player_start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSizePxF, 3.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  // create the level contents
  auto &random_level_sys = m_sys.find<Sys::Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_cross_gamearea( CryptScene::kMapGridSize, player_start_area );
  Factory::gen_crypt_main_objective( m_reg, m_sprite_Factory, CryptScene::kMapGridSize );
  Factory::create_initial_crypt_rooms( m_reg, CryptScene::kMapGridSize );
  Factory::gen_crypt_initial_interior( m_reg, m_sprite_Factory );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, CryptScene::kMapGridSize, "res/json/crypt_tilemap_config.json" );

  // pass concrete spawn position to exit spawner
  m_sys.find<Sys::Store::Type::CryptSystem>().spawn_exit( sf::Vector2u{ CryptScene::kMapGridSize.x / 2, CryptScene::kMapGridSize.y - 1 } );
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

  auto player_view = m_reg.view<Cmp::PlayerCharacter, Cmp::Position>();
  for ( auto [player_entity, pc_cmp, pos_cmp] : player_view.each() )
  {
    pos_cmp.position = m_player_start_position;
  }

  if ( Utils::Player::get_player_mortality( m_reg ).state != Cmp::PlayerMortality::State::DEAD )
  {
    m_sys.find<Sys::Store::Type::CryptSystem>().createRoomBorders();

    // make sure player has been situated in start room first
    m_sys.find<Sys::Store::Type::CryptSystem>().shuffle_rooms_passages();
    m_sys.find<Sys::Store::Type::CryptSystem>().reset_maze();
    get_maze_timer().restart();
  }
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
  get_maze_timer().reset();

  // force the loading screen so that we hide any motion sickness inducing camera pan
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

void CryptScene::do_update( sf::Time dt )
{

  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::CryptSystem>().check_exit_collision();
  m_sys.find<Sys::Store::Type::CryptSystem>().update();
  m_sys.find<Sys::Store::Type::ShockwaveSystem>().checkShockwavePlayerCollision();
  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::DarkMode::ON );
}

entt::registry &CryptScene::registry() { return m_reg; }

sf::Clock CryptScene::s_maze_timer;

} // namespace ProceduralMaze::Scene