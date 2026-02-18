#include <Audio/SoundBank.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/System.hpp>
#include <Factory/FloormapFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <Ruin/RuinHexagramMultiBlock.hpp>
#include <Ruin/RuinHexagramSegment.hpp>
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
#include <Systems/Threats/NpcSystem.hpp>

#include <Systems/SystemStore.hpp>
#include <Utils/Constants.hpp>

namespace ProceduralMaze::Scene
{

void RuinSceneUpperFloor::on_init()
{
  auto gridsize = Constants::kGridSizePxF;
  using SystemStoreType = Sys::SystemStore::Type;

  auto &m_persistent_sys = m_system_store.find<SystemStoreType::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, m_player_start_position );
  sf::Vector2f player_start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds( player_start_pos, gridsize, 1.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  auto &random_level_sys = m_system_store.find<SystemStoreType::RandomLevelGenerator>();
  random_level_sys.reset();
  random_level_sys.gen_rectangle_gamearea( RuinSceneUpperFloor::kMapGridSize, player_start_area, "RUIN.interior_wall",
                                           Sys::ProcGen::RandomLevelGenerator::SpawnArea::FALSE );

  // add two Cmp::NoPathFinding above the upper staircase landing to enforce perspective
  Factory::add_nopathfinding( m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 2 * gridsize.x ), gridsize.x } );
  Factory::add_nopathfinding( m_reg, { RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * gridsize.x ), gridsize.x } );

  const Sprites::MultiSprite &hexagram_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_hexagram3x3" );
  sf::Vector2f hexagram_pos( gridsize.x * 2, RuinSceneUpperFloor::kMapGridSizeF.y - hexagram_ms.getSpriteSizePixels().y - ( gridsize.y * 2 ) );
  Factory::add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( m_reg, hexagram_pos, hexagram_ms );

  // place the objective that was created when the player entered the RuinSceneLowerFloor scene
  auto ruin_objective_view = m_reg.view<Cmp::RuinObjectiveType>();
  for ( auto [ruin_obj_entt, ruin_obj_cmp] : ruin_objective_view.each() )
  {
    Factory::createCarryItem( m_reg, Cmp::Position( { hexagram_pos.x + gridsize.x, hexagram_pos.y + gridsize.y }, gridsize ), ruin_obj_cmp.m_type );
  }

  // spawn access hitbox just below horizontal centerpoint
  sf::Vector2f flooraccess_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 3 * gridsize.x ),
                                     RuinSceneUpperFloor::kMapGridSizeF.y - ( 3 * gridsize.y ) );
  sf::Vector2f flooraccess_size( ( 2 * gridsize.x ), gridsize.y );
  m_system_store.find<SystemStoreType::RuinSystem>().spawn_floor_access( flooraccess_position, flooraccess_size,
                                                                         Cmp::RuinFloorAccess::Direction::TO_LOWER );

  // add the straircase sprite for upper floor
  const Sprites::MultiSprite &stairs_upper_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_going_down" );
  sf::Vector2f stairs_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * gridsize.x ), ( 2 * gridsize.y ) );
  m_system_store.find<SystemStoreType::RuinSystem>().add_stairs<Cmp::RuinStairsUpperMultiBlock>( stairs_position, stairs_upper_ms );

  // add the straircase balustrade sprite for upper floor - make sure it is front of player
  const Sprites::MultiSprite &stairs_balustrade_ms = m_sprite_Factory.get_multisprite_by_type( "RUIN.interior_staircase_upper_balustrade" );
  sf::Vector2f balustrade_position( RuinSceneUpperFloor::kMapGridSizeF.x - ( 4 * gridsize.x ), ( 2 * gridsize.y ) );
  auto balustrade_zorder = Utils::get_player_position( m_reg ).position.y + gridsize.y;
  m_system_store.find<SystemStoreType::RuinSystem>().add_stairs<Cmp::RuinStairsBalustradeMultiBlock>( balustrade_position, stairs_balustrade_ms,
                                                                                                      balustrade_zorder );

  Factory::FloormapFactory::CreateFloormap( m_reg, m_floormap, RuinSceneUpperFloor::kMapGridSize, "res/json/ruin_upper_tilemap_config.json" );
}

void RuinSceneUpperFloor::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );

  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();

  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().init_views();

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::remove_player_lerp_cmp( m_reg );

  auto &player_pos = Utils::get_player_position( m_reg );
  player_pos.position = Utils::snap_to_grid( player_pos.position );
  SPDLOG_INFO( "Player entered RuinSceneUpperFloor at position ({}, {})", player_pos.position.x, player_pos.position.y );

  auto player_entt = Utils::get_player_entity( m_reg );
  m_reg.emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::UPPER );

  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().reset_floor_access_cooldown();
}

void RuinSceneUpperFloor::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
}

void RuinSceneUpperFloor::do_update( [[maybe_unused]] sf::Time dt )
{
  m_system_store.find<Sys::SystemStore::Type::AnimSystem>().update( dt );
  m_system_store.find<Sys::SystemStore::Type::NpcSystem>().update( dt );
  // m_system_store.find<Sys::SystemStore::Type::FootstepSystem>().update();
  m_system_store.find<Sys::SystemStore::Type::LootSystem>().check_loot_collision();
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_floor_access_collision( Cmp::RuinFloorAccess::Direction::TO_LOWER );
  m_system_store.find<Sys::SystemStore::Type::RuinSystem>().check_movement_slowdowns();

  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().update( dt, Sys::PlayerSystem::FootStepSfx::NONE );
  m_system_store.find<Sys::SystemStore::Type::PlayerSystem>().disable_damage_cooldown();

  auto &overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_floormap, Sys::RenderGameSystem::DarkMode::OFF,
                                                                               Sys::RenderGameSystem::WeatherMode::OFF );
}

entt::registry &RuinSceneUpperFloor::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene