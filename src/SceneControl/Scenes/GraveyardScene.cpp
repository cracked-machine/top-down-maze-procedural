#include <Audio/SoundBank.hpp>
#include <Components/Altar/MultiBlock.hpp>
#include <Components/Direction.hpp>
#include <Components/Grave/MultiBlock.hpp>
#include <Components/Inventory/Grimoire.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Particle/Flame.hpp>
#include <Components/Particle/SpriteBase.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/GraveyardProcGenBirthThreshold.hpp>
#include <Components/Persistent/GraveyardProcGenInitChance.hpp>
#include <Components/Persistent/GraveyardProcGenMaxIterations.hpp>
#include <Components/Persistent/GraveyardProcGenSurvivalThreshold.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/RuinProcGenMaxIterations.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Player/LastGraveyardPosition.hpp>
#include <Components/Player/LevelDepth.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/BuildingMultiBlock.hpp>
#include <Components/SceneSettings/CollisionDetection.hpp>
#include <Components/SceneSettings/CurrentScene.hpp>
#include <Components/SceneSettings/Footsteps.hpp>
#include <Components/SceneSettings/Shaders.hpp>
#include <Components/SceneSettings/ShowDebugStats.hpp>
#include <Components/SceneSettings/ShowNavmesh.hpp>
#include <Components/SceneSettings/ShowPathFinding.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/PathfindingFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/ShaderFactory.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/SceneData.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/NightStaticShader.hpp>
#include <Systems/ActionSystem.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/ArrowSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/GrimoireSystem.hpp>
#include <Systems/HealingSpringSystem.hpp>
#include <Systems/InventorySystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/DLASystem.hpp>
#include <Systems/ProcGen/LevelGenerator.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/RuinSystem.hpp>
#include <Systems/ShaderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Systems/Stores/SystemStore.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Systems/Threats/LightningSystem.hpp>
#include <Systems/Threats/WatchmanSystem.hpp>
#include <Systems/Threats/WispSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Profiling.hpp>

#include <cmath>

namespace Game::Scene
{

void GraveyardScene::on_init()
{
  SPDLOG_INFO( "Init {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  auto scene_settings_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::SceneSettings::CurrentScene>( scene_settings_entt, Cmp::SceneSettings::SceneId::GRAVEYARD );
  m_reg.emplace_or_replace<Cmp::SceneSettings::CollisionDetection>( scene_settings_entt, true );
  m_reg.emplace_or_replace<Cmp::SceneSettings::ShowPathFinding>( scene_settings_entt, false );
  m_reg.emplace_or_replace<Cmp::SceneSettings::ShowNavmesh>( scene_settings_entt, false );
  m_reg.emplace_or_replace<Cmp::SceneSettings::ShowDebugStats>( scene_settings_entt, false );
  m_reg.emplace_or_replace<Cmp::SceneSettings::Shaders>( scene_settings_entt, true );
  m_reg.emplace_or_replace<Cmp::SceneSettings::Footsteps>( scene_settings_entt, true );

  m_sys.find<Sys::Store::Type::ItemStore>().init_store();
  m_sys.find<Sys::Store::Type::NpcStore>().init_store();

  auto grimoire_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::Grimoire>( grimoire_entt );

  m_scene_data = std::make_shared<SceneData>( "res/scenes/graveyard.json" );

  // player start position
  auto [_, player_start_pos_px] = m_scene_data->get_player_start_position();
  Sys::PersistSystem::add<Cmp::Persist::PlayerStartPosition>( m_reg, player_start_pos_px );
  auto player_start_position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg );
  auto player_start_area = Cmp::RectBounds::scaled( player_start_position, Constants::kGridSizePxF, 5.f, Cmp::RectBounds::ScaleAxis::XY );

  // create the player, optionally increment the level counter
  auto player_view = m_reg.view<Cmp::Player::Character>();
  if ( player_view.size() == 0 ) { Factory::Player::create_player( m_reg ); }
  else
  {
    auto &level_depth_cmp = Utils::Player::get_level_depth( m_reg );
    level_depth_cmp.increment_count( 1 );
    level_depth_cmp.display_timer.restart();
  }

  auto [map_size_grid, map_size_pixel] = m_scene_data->map_size();
  SPDLOG_INFO( "m_scene_map_data {},{} {},{}", map_size_grid.x, map_size_grid.y, map_size_pixel.x, map_size_pixel.y );

  Factory::Shader::add_water( m_sys.find<Sys::Store::Type::ShaderSystem>(), map_size_pixel );
  // Factory::Shader::add_mist( m_sys.find<Sys::Store::Type::ShaderSystem>(), map_size_pixel );
  Factory::Shader::add_night_static( m_sys.find<Sys::Store::Type::ShaderSystem>(), map_size_pixel );
  Factory::Shader::add_fear_distortion( m_sys.find<Sys::Store::Type::ShaderSystem>(),
                                        Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( m_reg ) );

  // create the level contents
  auto &level_gen = m_sys.find<Sys::Store::Type::LevelGenerator>();
  m_reserved_sm = Factory::Pathfinding::create_reserved_navmesh( m_reg );
  level_gen.reset();
  level_gen.build_scene_from_data( *m_scene_data, m_reserved_sm );
  m_sys.find<Sys::Store::Type::ExitSystem>().create_exit();
  level_gen.gen_graveyard_exterior_multiblocks( m_reserved_sm );
  Factory::Loot::gen_loot_containers( m_reg, m_sprite_factory, map_size_grid, m_reserved_sm );
  Factory::Npc::gen_npc_containers( m_reg, m_sprite_factory, map_size_grid, m_reserved_sm );
  level_gen.gen_random_plants( map_size_grid, m_reserved_sm );

  auto init_chance = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenInitChance>( m_reg );
  level_gen.add_graveyard_exterior_obstacles( init_chance.get_value(), m_reserved_sm );

  auto &cellauto_parser = m_sys.find<Sys::Store::Type::CellAutomataSystem>();
  auto max_iterations = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenMaxIterations>( m_reg );
  auto birth_threshold = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenBirthThreshold>( m_reg );
  auto survival_threshold = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenSurvivalThreshold>( m_reg );
  cellauto_parser.iterate( max_iterations.get_value(), birth_threshold.get_value(), survival_threshold.get_value(),
                           Sys::ProcGen::LevelGenerator::SceneType::GRAVEYARD_EXTERIOR, level_gen.get_obstacle_sm(), m_reserved_sm );

  level_gen.decorate_graveyard_exterior_obstacles();

  // create navmeshes for pathfinding
  m_generic_npc_navmesh = Factory::Pathfinding::create_npc_navmesh( m_reg );
  m_ghost_navmesh = Factory::Pathfinding::create_ghost_navmesh( m_reg );
  m_player_navmesh = Factory::Pathfinding::create_player_navmesh( m_reg );
  m_open_navmesh = Factory::Pathfinding::create_open_navmesh( m_reg );

  // spatial index for RenderGameSystem's Cmp::Position z-order pass - kept up to date by a periodic
  // rebuild in do_update() rather than precise per-mutation sync, see kRenderPositionGridRebuildInterval
  m_render_position_sm = Factory::Pathfinding::create_render_position_grid( m_reg );

  reinit_navmesh();

  // create floor background
  Sprites::Containers::VertexFloor floortiles;
  floortiles.create( level_gen.get_void_sm(), m_scene_data );

  m_sys.find<Sys::Store::Type::RenderGameSystem>().init_world_view();

  auto new_sinkhole_pos = m_sys.find<Sys::Store::Type::SinkHoleHazardSystem>().init_hazard_field();
  if ( new_sinkhole_pos != sf::Vector2f{ 0, 0 } ) { floortiles.remove( new_sinkhole_pos ); }

  auto floor_entity = m_reg.create();
  m_reg.emplace<Sprites::Containers::VertexFloor>( floor_entity, floortiles );
  m_reg.emplace<Cmp::ZOrderValue>( floor_entity, -16.f );

  m_sys.find<Sys::Store::Type::CorruptionHazardSystem>().init_hazard_field();
  m_sys.find<Sys::Store::Type::WormholeSystem>().spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );

  // add flame particle sprites for any candle items in the new game world. Use the Candle item UUID to initialise the ParticleSprite.
  for ( auto [worlditem_entt, worlditem_cmp, worlditem_pos_cmp, worlditem_uuid_cmp] : m_reg.view<Cmp::WorldItem, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( worlditem_cmp.item_type != "item.candle" ) continue;
    Factory::Particle::add_flame( m_reg, "graveyard.candle.particle.flame", worlditem_uuid_cmp,
                                  { worlditem_pos_cmp.getCenter().x, worlditem_pos_cmp.getCenter().y - Cmp::Particle::Flame::kVerticalOffset },
                                  Utils::Player::get_position( m_reg ).y() - 1, Cmp::Particle::kWorldScalePreset );
  }
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering {}", get_name() );
  reinit_navmesh();

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  m_sound_bank.get_music( "title_music" ).stop();
  if ( m_sound_bank.get_music( "graveyard_music" ).getStatus() != sf::Sound::Status::Playing )
  {
    m_sound_bank.get_music( "graveyard_music" ).play();
    m_sound_bank.get_music( "graveyard_music" ).setLooping( true );
  }

  // prevent residual lerp movements from previous scene causing havoc in the new one
  Utils::Player::remove_lerp_cmp( m_reg );

  // Respawn player back in the graveyard: either at the last position when they left, or fallback to their start position
  auto &player_pos = Utils::Player::get_position( m_reg );
  auto *player_last_graveyard_pos = Utils::Player::get_last_graveyard_pos( m_reg );
  if ( player_last_graveyard_pos != nullptr )
  {
    player_pos.position = player_last_graveyard_pos->position;
    SPDLOG_INFO( "Player re-entered graveyard at position ({}, {})", player_pos.position.x, player_pos.position.y );
  }
  else if ( m_just_spawned )
  {
    player_pos.position = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( m_reg ) );
    SPDLOG_INFO( "Player entered graveyard at position ({}, {})", player_pos.position.x, player_pos.position.y );
    m_just_spawned = false;
  }

  // prevent the player from wandering off before the scene has loaded
  auto &player_dir = Utils::Player::get_direction( m_reg );
  player_dir = Cmp::Direction{ { 0.f, 0.f } };

  // check if the player inventory has a candle, if so light it up!
  Factory::Particle::add_flame_for_player_inventory_slot( m_reg );

  m_scene_exit_cooldown.restart();
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "graveyard_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  m_sys.find<Sys::Store::Type::FootstepSystem>().stop_footsteps_sound();

  for ( auto [floor_entt, floor_cmp] : m_reg.view<Sprites::Containers::VertexFloor>().each() )
  {
    floor_cmp.clear();
  }

  m_sound_bank.get_music( "graveyard_music" ).stop();
}

void GraveyardScene::do_update( sf::Time dt )
{
  ZoneScoped;

  // keep m_render_position_sm up to date with static entities created/destroyed since it was last
  // built - see its doc comment for why this is a periodic rebuild rather than precise sync.
  // Repopulates the existing grid object in place (never reassigns m_render_position_sm) so systems
  // like InventorySystem that hold a weak_ptr to it from a one-time init() call don't go stale.
  if ( m_render_position_grid_rebuild_clock.getElapsedTime() >= kRenderPositionGridRebuildInterval )
  {
    PROFILED( Factory::Pathfinding::populate_render_position_grid( m_reg, *m_render_position_sm ) );
    m_render_position_grid_rebuild_clock.restart();
  }

  PROFILED( m_sys.find<Sys::Store::Type::AnimSystem>().update( dt ) );

  {
    ZoneScopedN( "SinkHoleHazardSystem::update" );
    if ( auto pos = m_sys.find<Sys::Store::Type::SinkHoleHazardSystem>().update(); pos != sf::Vector2f{ 0, 0 } )
    {
      for ( auto [floor_entt, floortiles] : m_reg.view<Sprites::Containers::VertexFloor>().each() )
      {
        floortiles.remove( pos );
      }
    }
  }

  PROFILED( m_sys.find<Sys::Store::Type::CorruptionHazardSystem>().update() );
  PROFILED( m_sys.find<Sys::Store::Type::BombSystem>().update() );
  PROFILED( m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision() );
  PROFILED( m_sys.find<Sys::Store::Type::NpcSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::WispSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::WispSystem>().spawn_wisp() );
  PROFILED( m_sys.find<Sys::Store::Type::WatchmanSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::WormholeSystem>().check_player_wormhole_collision() );
  PROFILED( m_sys.find<Sys::Store::Type::ActionSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::FootstepSystem>().update( dt ) );

  PROFILED( m_sys.find<Sys::Store::Type::CryptSystem>().update_exit_zorder() );
  if ( m_scene_exit_cooldown.getElapsedTime() >= m_scene_exit_cooldown_time )
  {
    PROFILED( m_sys.find<Sys::Store::Type::CryptSystem>().check_entrance_collision() );
  }

  PROFILED( m_sys.find<Sys::Store::Type::CryptSystem>().unlock_crypt_door() );
  PROFILED( m_sys.find<Sys::Store::Type::AltarSystem>().update() );
  PROFILED( m_sys.find<Sys::Store::Type::HealingSpringSystem>().update_building_zorder() );
  PROFILED( m_sys.find<Sys::Store::Type::HealingSpringSystem>().check_entrance_collision() );
  PROFILED( m_sys.find<Sys::Store::Type::RuinSystem>().update_exit_zorder() );
  PROFILED( m_sys.find<Sys::Store::Type::RuinSystem>().check_entrance_collision() );
  PROFILED( m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::LightningSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::GrimoireSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::ExitSystem>().update_exit_zorder() );
  PROFILED( m_sys.find<Sys::Store::Type::ExitSystem>().check_exit_collision() );
  PROFILED( m_sys.find<Sys::Store::Type::ArrowSystem>().update( dt ) );
  PROFILED( m_sys.find<Sys::Store::Type::InventorySystem>().update( dt ) );

  PROFILED( m_sys.find<Sys::Store::Type::ParticleSystem>().check_collsion( { "graveyard.plant.particle.flame" } ) );
  PROFILED( m_sys.find<Sys::Store::Type::ParticleSystem>().update( dt ) );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  PROFILED( m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys, m_generic_npc_navmesh, m_render_position_sm ) );
}

void GraveyardScene::reinit_navmesh()
{
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_generic_npc_navmesh, m_open_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::WispSystem>().init( m_open_navmesh );
  m_sys.find<Sys::Store::Type::BombSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::ActionSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::InventorySystem>().init( m_generic_npc_navmesh, m_player_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_open_navmesh );
  m_sys.find<Sys::Store::Type::WormholeSystem>().init( m_generic_npc_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_generic_npc_navmesh );
}

entt::registry &GraveyardScene::registry() { return m_reg; }

} // namespace Game::Scene