#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Direction.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Inventory/Grimoire.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/GraveyardProcGenBirthThreshold.hpp>
#include <Components/Persistent/GraveyardProcGenInitChance.hpp>
#include <Components/Persistent/GraveyardProcGenMaxIterations.hpp>
#include <Components/Persistent/GraveyardProcGenSurvivalThreshold.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/RuinProcGenMaxIterations.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerLevelDepth.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinBuildingMultiBlock.hpp>
#include <Components/SceneSettings/CollisionDetection.hpp>
#include <Components/SceneSettings/CurrentScene.hpp>
#include <Components/SceneSettings/Shaders.hpp>
#include <Components/SceneSettings/ShowDebugStats.hpp>
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
#include <Systems/Threats/WormholeSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>

#include <cmath>

namespace Game::Scene
{

void GraveyardScene::on_init()
{
  SPDLOG_INFO( "Init {}", get_name() );

  auto &m_persistent_sys = m_sys.find<Sys::Store::Type::PersistSystem>();
  m_persistent_sys.initialize_component_registry();
  m_persistent_sys.load_state();

  auto sys_cmp_entt = m_reg.create();
  m_reg.emplace_or_replace<Cmp::SceneSettings::CurrentScene>( sys_cmp_entt, Cmp::SceneSettings::SceneId::GRAVEYARD );
  m_reg.emplace_or_replace<Cmp::SceneSettings::CollisionDetection>( sys_cmp_entt, true );
  m_reg.emplace_or_replace<Cmp::SceneSettings::ShowPathFinding>( sys_cmp_entt, false );
  m_reg.emplace_or_replace<Cmp::SceneSettings::ShowDebugStats>( sys_cmp_entt, false );
  m_reg.emplace_or_replace<Cmp::SceneSettings::Shaders>( sys_cmp_entt, true );

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
  auto player_view = m_reg.view<Cmp::PlayerCharacter>();
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

  // create the level contents
  auto &level_gen = m_sys.find<Sys::Store::Type::LevelGenerator>();
  level_gen.reset();
  level_gen.build_scene_from_data( *m_scene_data );
  m_sys.find<Sys::Store::Type::ExitSystem>().create_exit();
  level_gen.gen_graveyard_exterior_multiblocks();
  m_reserved_navmash = Factory::Pathfinding::create_reserved_navmesh( m_reg );
  Factory::Loot::gen_loot_containers( m_reg, m_sprite_factory, map_size_grid, m_reserved_navmash );
  Factory::Npc::gen_npc_containers( m_reg, m_sprite_factory, map_size_grid, m_reserved_navmash );
  level_gen.gen_random_plants( map_size_grid, m_reserved_navmash );

  auto init_chance = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenInitChance>( m_reg );
  level_gen.add_graveyard_exterior_obstacles( init_chance.get_value(), m_reserved_navmash );

  auto &cellauto_parser = m_sys.find<Sys::Store::Type::CellAutomataSystem>();
  auto max_iterations = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenMaxIterations>( m_reg );
  auto birth_threshold = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenBirthThreshold>( m_reg );
  auto survival_threshold = Sys::PersistSystem::get<Cmp::Persist::GraveyardProcGenSurvivalThreshold>( m_reg );
  cellauto_parser.iterate( max_iterations.get_value(), birth_threshold.get_value(), survival_threshold.get_value(),
                           Sys::ProcGen::LevelGenerator::SceneType::GRAVEYARD_EXTERIOR, level_gen.get_obstacle_sm(), m_reserved_navmash );

  level_gen.decorate_graveyard_exterior_obstacles();

  // auto max_iterations = Sys::PersistSystem::get<Cmp::Persist::RuinProcGenMaxIterations>( m_reg );
  // auto &dla_sys = m_sys.find<Sys::Store::Type::DiffusionLtdAggrSystem>();
  // dla_sys.iterate( sf::FloatRect( { 0.f, 0.f }, map_size_pixel ), max_iterations.get_value(), level_gen.get_obstacle_sm(),
  //                  Sys::ProcGen::DLASystem::SpawnShape::Ellipse );

  // create navmeshes for pathfinding
  m_generic_npc_navmesh = Factory::Pathfinding::create_npc_navmesh( m_reg );
  m_ghost_navmesh = Factory::Pathfinding::create_ghost_navmesh( m_reg );
  m_player_navmesh = Factory::Pathfinding::create_player_navmesh( m_reg );
  m_open_navmesh = Factory::Pathfinding::create_open_navmesh( m_reg );
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

  // auto &particle_system = m_sys.find<Sys::Store::Type::ParticleSystem>();
  // Factory::Particle::add_test( m_reg, particle_system, "GraveyardParticleTest" );
  // Factory::Particle::add_flame( m_reg, particle_system, "player.candle", Utils::Player::get_position( m_reg ).getCenter() );
  // Factory::Particle::add_flame( m_reg, particle_system, "player.candle", sf::Vector2f{ 900.0, 900.0 } );
  // Factory::Particle::add_smoke( m_reg, particle_system, "GraveyardParticleTest" );
  // Factory::Particle::add_shockwave( m_reg, particle_system, "GraveyardParticleTest" );
  // auto uuid = Cmp::UUID::generate();
  // Factory::Particle::add_shockwave( m_reg, "GraveyardParticleTest", uuid, Utils::Player::get_position( m_reg ).getCenter(), 50000 );

  // add flame particle sprites for any candle items in the new game world. Use the Candle item UUID to initialise the ParticleSprite.
  for ( auto [candle_entt, candle_cmp, candle_pos, uuid_cmp] : m_reg.view<Cmp::WorldItem, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( not candle_cmp.sprite_type.contains( "candle" ) ) continue;
    Factory::Particle::add_flame( m_reg, "particle.candle", uuid_cmp, candle_pos.getCenter(), Utils::Player::get_position( m_reg ).y() - 1 );
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
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Sound::Status::Playing )
  {
    m_sound_bank.get_music( "game_music" ).play();
    m_sound_bank.get_music( "game_music" ).setLooping( true );
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

  m_scene_exit_cooldown.restart();
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting {}", get_name() );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_sys.find<Sys::Store::Type::PlayerSystem>();
  m_player_sys.stop_footsteps_sound();

  for ( auto [floor_entt, floor_cmp] : m_reg.view<Sprites::Containers::VertexFloor>().each() )
  {
    floor_cmp.clear();
  }

  m_sound_bank.get_music( "game_music" ).stop();
}

void GraveyardScene::do_update( sf::Time dt )
{
  m_sys.find<Sys::Store::Type::AnimSystem>().update( dt );

  if ( auto pos = m_sys.find<Sys::Store::Type::SinkHoleHazardSystem>().update(); pos != sf::Vector2f{ 0, 0 } )
  {
    for ( auto [floor_entt, floortiles] : m_reg.view<Sprites::Containers::VertexFloor>().each() )
    {
      floortiles.remove( pos );
    }
  }

  m_sys.find<Sys::Store::Type::CorruptionHazardSystem>().update();
  m_sys.find<Sys::Store::Type::BombSystem>().update();
  m_sys.find<Sys::Store::Type::LootSystem>().check_loot_collision();
  m_sys.find<Sys::Store::Type::NpcSystem>().update( dt );
  m_sys.find<Sys::Store::Type::NpcSystem>().spawn_wisp();
  m_sys.find<Sys::Store::Type::WormholeSystem>().check_player_wormhole_collision();
  m_sys.find<Sys::Store::Type::ActionSystem>().update( dt );
  m_sys.find<Sys::Store::Type::FootstepSystem>().update();

  m_sys.find<Sys::Store::Type::CryptSystem>().update_exit_zorder();
  if ( m_scene_exit_cooldown.getElapsedTime() >= m_scene_exit_cooldown_time )
  {
    m_sys.find<Sys::Store::Type::CryptSystem>().check_entrance_collision();
  }

  m_sys.find<Sys::Store::Type::CryptSystem>().unlock_crypt_door();
  m_sys.find<Sys::Store::Type::AltarSystem>().check_player_collision();
  m_sys.find<Sys::Store::Type::HealingSpringSystem>().update_building_zorder();
  m_sys.find<Sys::Store::Type::HealingSpringSystem>().check_entrance_collision();
  m_sys.find<Sys::Store::Type::RuinSystem>().update_exit_zorder();
  m_sys.find<Sys::Store::Type::RuinSystem>().check_entrance_collision();
  m_sys.find<Sys::Store::Type::PlayerSystem>().update( dt );
  m_sys.find<Sys::Store::Type::LightningSystem>().update( dt );
  m_sys.find<Sys::Store::Type::GrimoireSystem>().update( dt );
  m_sys.find<Sys::Store::Type::ExitSystem>().update_exit_zorder();
  m_sys.find<Sys::Store::Type::ExitSystem>().check_exit_collision();
  m_sys.find<Sys::Store::Type::ArrowSystem>().update( dt );

  for ( auto [ob_entt, ob_cmp, pos_cmp, uuid_cmp] : m_reg.view<Cmp::Obstacle, Cmp::Position, Cmp::UUID>().each() )
  {
    // If this is the obstacle being currently dug then skip particle collision detection
    if ( m_reg.any_of<Cmp::SelectedPosition>( ob_entt ) ) continue;
    m_sys.find<Sys::Store::Type::ParticleSystem>().check_collsion( pos_cmp );
  }

  m_sys.find<Sys::Store::Type::ParticleSystem>().update( dt );

  auto &overlay_sys = m_sys.find<Sys::Store::Type::RenderOverlaySystem>();
  m_sys.find<Sys::Store::Type::RenderGameSystem>().render_game( dt, overlay_sys );
}

void GraveyardScene::reinit_navmesh()
{
  m_sys.find<Sys::Store::Type::NpcSystem>().init( m_generic_npc_navmesh, m_open_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::BombSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::ActionSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_ghost_navmesh );
  m_sys.find<Sys::Store::Type::PlayerSystem>().init( m_generic_npc_navmesh, m_player_navmesh, m_open_navmesh );
  m_sys.find<Sys::Store::Type::WormholeSystem>().init( m_generic_npc_navmesh );
  m_sys.find<Sys::Store::Type::RenderOverlaySystem>().init( m_generic_npc_navmesh );
}

entt::registry &GraveyardScene::registry() { return m_reg; }

} // namespace Game::Scene