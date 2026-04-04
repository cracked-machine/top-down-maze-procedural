#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Ruin/RuinHexagramMultiBlock.hpp>
#include <Components/Ruin/RuinHexagramSegment.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scenes/RuinSceneUpperFloor.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/CryptSystem.hpp>
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

namespace ProceduralMaze::Scene
{

void RuinSceneUpperFloor::on_init()
{
  using namespace Sys;
  auto gridsize = Constants::kGridSizePxF;

  auto &m_persistent_sys = m_sys.find<Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_scene_map_data = std::make_shared<SceneData>( "res/scenes/ruinupper.json" );

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace<Cmp::System>( sys_cmp_entt );

  // initialise the persistent player start position from the scene data
  auto [_, player_start_pos_px] = m_scene_map_data->get_player_start_position();
  PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );

  auto [map_size_grid, map_size_pixel] = m_scene_map_data->map_size();

  // generate the empty game area
  sf::Vector2f player_start_pos = PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_pos, gridsize, 1.f, Cmp::RectBounds::ScaleAxis::XY );
  auto &random_level_sys = m_sys.find<Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_game_area( *m_scene_map_data );

  // add access hitbox just below horizontal centerpoint
  sf::Vector2f flooraccess_position( map_size_pixel.x - ( 3 * gridsize.x ), map_size_pixel.y - ( 3 * gridsize.y ) );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_sys.find<Store::Type::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size, Cmp::RuinFloorAccess::Direction::TO_LOWER );

  m_floormap.create( random_level_sys.get_void_sm(), m_scene_map_data );

  // create a navmesh for pathfinding in the scene
  m_pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    m_pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_pathfinding_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_pathfinding_navmesh );
}

void RuinSceneUpperFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( m_reg );
  if ( inventory_type != "CARRYITEM.witchesjar" )
  {
    if ( m_sound_bank.get_music( "ruin_creaking_rope" ).getStatus() != sf::Sound::Status::Playing )
    {
      m_sound_bank.get_music( "ruin_creaking_rope" ).play();
      m_sound_bank.get_music( "ruin_creaking_rope" ).setLooping( true );
    }
    if ( m_sound_bank.get_music( "ruin_music" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_music( "ruin_music" ).play(); }
  }
  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_views();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::Player::remove_lerp_cmp( m_reg );

  // prevent the player from wandering off before the scene has loaded
  auto &player_dir = Utils::Player::get_direction( m_reg );
  player_dir = Cmp::Direction{ { 0.f, 0.f } };

  auto &player_pos = Utils::Player::get_position( m_reg );
  player_pos.position = Utils::snap_to_grid( player_pos.position );

  auto player_entt = Utils::Player::get_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::UPPER );

  m_sys.find<Sys::Store::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneUpperFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_sound_bank.get_music( "ruin_creaking_rope" ).stop();
  m_sound_bank.get_music( "ruin_music" ).stop();
  m_reg.clear();
}

void RuinSceneUpperFloor::do_update( sf::Time dt )
{
  using namespace Sys;
  m_sys.find<Store::Type::AnimSystem>().update( dt );
  m_sys.find<Store::Type::NpcSystem>().update( dt );
  // m_sys.find<Store::Type::FootstepSystem>().update();
  m_sys.find<Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Store::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_LOWER );
  m_sys.find<Store::Type::RuinSystem>().check_movement_slowdowns();

  m_sys.find<Store::Type::PlayerSystem>().update( dt, PlayerSystem::FootStepSfx::NONE );
  m_sys.find<Store::Type::PlayerSystem>().disable_damage_cooldown();

  auto [_, map_size_pixel] = m_scene_map_data->map_size();
  bool player_curse_active = m_sys.find<Store::Type::RuinSystem>().check_activate_player_curse( map_size_pixel );

  m_sys.find<Store::Type::RuinSystem>().update_shadow_hand_pos( map_size_pixel );
  m_sys.find<Store::Type::RuinSystem>().check_player_shadow_hand_collision();

  auto &overlay_sys = m_sys.find<Store::Type::RenderOverlaySystem>();
  m_sys.find<Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, DarkMode::OFF, WeatherMode::OFF,
                                                           ( player_curse_active ? CursedMode::ON : CursedMode::OFF ) );
}

entt::registry &RuinSceneUpperFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene