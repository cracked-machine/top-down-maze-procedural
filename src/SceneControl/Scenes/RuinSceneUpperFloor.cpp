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

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  sf::Vector2f player_start_pos = PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, gridsize, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  auto &random_level_sys = m_sys.find<Store::Type::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneUpperFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // add two Cmp::NoPathFinding above the upper staircase landing to enforce perspective
  Factory::add_nopathfinding( m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 2 * gridsize.x ), gridsize.x } );
  Factory::add_nopathfinding( m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * gridsize.x ), gridsize.x } );

  const Sprites::MultiSprite &hexagram_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_hexagram3x3" );
  sf::Vector2f hexagram_pos( gridsize.x * 2, RuinSceneUpperFloor::kMapGridSizeF.y - hexagram_ms.getSpriteSizePixels().y - ( gridsize.y * 3 ) );
  Factory::add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( m_reg, hexagram_pos, hexagram_ms );

  // place the objective that was created when the player entered the RuinSceneLowerFloor scene
  if ( not m_sys.find<Store::Type::RuinSystem>().is_player_carrying_witches_jar() )
  {
    auto ruin_objective_view = m_reg.view<Cmp::RuinObjectiveType>();
    for ( auto [ruin_obj_entt, ruin_obj_cmp] : ruin_objective_view.each() )
    {
      Factory::create_carry_item( m_reg, Cmp::Position( { hexagram_pos.x + gridsize.x, hexagram_pos.y + gridsize.y }, gridsize ),
                                  ruin_obj_cmp.m_type );
    }
  }

  // spawn access hitbox just below horizontal centerpoint
  sf::Vector2f flooraccess_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * gridsize.x ),
                                     RuinSceneUpperFloor::kMapGridSizeF.y - ( 3 * gridsize.y ) );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_sys.find<Store::Type::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size, Cmp::RuinFloorAccess::Direction::TO_LOWER );

  // add the straircase sprite for upper floor
  const Sprites::MultiSprite &stairs_upper_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_going_down" );
  sf::Vector2f stairs_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * gridsize.x ), ( 2 * gridsize.y ) );
  m_sys.find<Store::Type::RuinSystem>().add_stairs<Cmp::RuinStairsUpperMultiBlock>( stairs_position, stairs_upper_ms );

  // add the straircase balustrade sprite for upper floor - make sure it is front of player
  const Sprites::MultiSprite &stairs_balustrade_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_upper_balustrade" );
  sf::Vector2f balustrade_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * gridsize.x ), ( 2 * gridsize.y ) );
  auto balustrade_zorder = Utils::Player::get_position( m_reg ).position.y + gridsize.y;
  m_sys.find<Store::Type::RuinSystem>().add_stairs<Cmp::RuinStairsBalustradeMultiBlock>( balustrade_position, stairs_balustrade_ms,
                                                                                         balustrade_zorder );

  Factory::FloormapFactory::create_floormap( m_reg, m_floormap, RuinSceneUpperFloor::kMapGridSize, "res/json/ruin_upper_tilemap_config.json" );

  // create a spatial grid of the game area
  m_spatialgrid_ptr = std::make_shared<PathFinding::SpatialHashGrid>();
  auto view = m_reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> );
  for ( auto entity : view )
  {
    const auto &pos = view.get<Cmp::Position>( entity );
    m_spatialgrid_ptr->insert( entity, pos );
  }
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_spatialgrid_ptr );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_spatialgrid_ptr );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_spatialgrid_ptr );
}

void RuinSceneUpperFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  bool player_is_cursed = m_sys.find<Sys::Store::Type::RuinSystem>().is_player_carrying_witches_jar();
  if ( not player_is_cursed )
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

  auto &player_pos = Utils::Player::get_position( m_reg );
  player_pos.position = Utils::snap_to_grid( player_pos.position );
  SPDLOG_INFO( "Player entered RuinSceneUpperFloor at position ({}, {})", player_pos.position.x, player_pos.position.y );

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

void RuinSceneUpperFloor::do_update( [[maybe_unused]] sf::Time dt )
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

  bool player_curse_active = m_sys.find<Store::Type::RuinSystem>().check_activate_player_curse( RuinSceneUpperFloor::kMapGridSizeF );

  m_sys.find<Store::Type::RuinSystem>().update_shadow_hand_pos( RuinSceneUpperFloor::kMapGridSizeF );
  m_sys.find<Store::Type::RuinSystem>().check_player_shadow_hand_collision();

  auto &overlay_sys = m_sys.find<Store::Type::RenderOverlaySystem>();
  m_sys.find<Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, DarkMode::OFF, WeatherMode::OFF,
                                                           ( player_curse_active ? CursedMode::ON : CursedMode::OFF ) );
}

entt::registry &RuinSceneUpperFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene