#include <Components/Altar/MultiBlock.hpp>
#include <Components/Altar/Segment.hpp>
#include <Components/Crypt/BuildingMultiBlock.hpp>
#include <Components/Crypt/BuildingSegment.hpp>
#include <Components/Crypt/InteriorMultiBlock.hpp>
#include <Components/Crypt/InteriorSegment.hpp>
#include <Components/Crypt/ObjectiveMultiBlock.hpp>
#include <Components/Crypt/ObjectiveSegment.hpp>
#include <Components/Crypt/RoomClosed.hpp>
#include <Components/Crypt/RoomEnd.hpp>
#include <Components/Crypt/RoomOpen.hpp>
#include <Components/Crypt/RoomStart.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/MultiBlock.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Grave/Segment.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/Moveable.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/BuildingSegment.hpp>
#include <Components/Ruin/Cobweb.hpp>
#include <Components/Ruin/GateSegment.hpp>
#include <Components/Ruin/HexagramMultiBlock.hpp>
#include <Components/Ruin/HexagramSegment.hpp>
#include <Components/Ruin/RuneMarking.hpp>
#include <Components/Ruin/StairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/StairsGateMultiBlock.hpp>
#include <Components/Ruin/StairsLowerMultiBlock.hpp>
#include <Components/Ruin/StairsUpperMultiBlock.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/Spring/HealingSpringBuildingMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingSegment.hpp>
#include <Components/Spring/HealingSpringMultiBlock.hpp>
#include <Components/Spring/HealingSpringSegment.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <SceneControl/SceneData.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/ProcGen/LevelGenerator.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>
#include <optional>
#include <ranges>
#include <spdlog/spdlog.h>

namespace Game::Sys::ProcGen
{

LevelGenerator::LevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_obstacle_sm( std::make_unique<PathFinding::SpatialHashGrid>() ),
      m_void_sm( std::make_unique<PathFinding::SpatialHashGrid>() ),
      m_non_obstacle_sm( std::make_unique<PathFinding::SpatialHashGrid>() )
{
}

PathFinding::SpatialHashGrid &LevelGenerator::get_obstacle_sm() { return *m_obstacle_sm; }
PathFinding::SpatialHashGrid &LevelGenerator::get_void_sm() { return *m_void_sm; }
PathFinding::SpatialHashGrid &LevelGenerator::get_non_obstacle_sm() { return *m_non_obstacle_sm; }

void LevelGenerator::build_scene_from_data( const Scene::SceneData &scene_data, const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  auto [map_size_grid, map_size_pixel] = scene_data.map_size();
  auto w = map_size_grid.x;

  // Walls
  const Sprites::SpriteSheet &wall_ms = m_sprite_factory.get_spritesheet_by_type( scene_data.wall_tileset().name );
  for ( const auto [i, tile] : std::views::enumerate( scene_data.wall_tilelayer() ) )
  {
    auto row = i / w; // increments every 'w' tiles
    auto col = i % w; // wraps back to zero every 'w' tiles
    sf::Vector2f new_pos( static_cast<float>( col ) * Constants::kGridSizePxF.x, static_cast<float>( row ) * Constants::kGridSizePxF.y );
    if ( tile >= scene_data.wall_tileset().first_gid )
    {
      // get the relative index using the offset from the json data
      auto entt = Factory::Wall::add_wall_entity( reg(), new_pos, wall_ms, tile - scene_data.wall_tileset().first_gid );
      reserved_sm->insert( entt, Cmp::Position( new_pos, Constants::kGridSizePxF ) );
    }
  }

  // Levelgen layer
  for ( const auto [i, tile] : std::views::enumerate( scene_data.levelgen_tilelayer() ) )
  {
    auto col = i % w; // wraps back to zero every 'w' tiles
    auto row = i / w; // increments every 'w' tiles
    sf::Vector2f new_pos( static_cast<float>( col ) * Constants::kGridSizePxF.x, static_cast<float>( row ) * Constants::kGridSizePxF.y );
    if ( tile == scene_data.void_tile_id() )
    {
      Cmp::Position new_pos_cmp( new_pos, Constants::kGridSizePxF );
      auto entt = Factory::Obstacle::create_void_pos( reg(), new_pos_cmp );
      m_void_sm->insert( entt, new_pos_cmp );
    }
    else if ( tile == scene_data.open_tile_id() ) { Factory::Obstacle::create_world_pos( reg(), new_pos ); }
    else if ( tile == scene_data.spawn_tile_id() )
    {
      auto entity = Factory::Obstacle::create_world_pos( reg(), new_pos );
      Factory::Player::add_spawn_area( reg(), entity, m_sprite_factory, new_pos.y - 16.0f );
      reserved_sm->insert( entity, Cmp::Position( new_pos, Constants::kGridSizePxF ) );
    }
    else if ( tile == scene_data.exit_tile_id() )
    {
      auto entt = Factory::Crypt::create_crypt_exit( reg(), new_pos );
      reserved_sm->insert( entt, Cmp::Position( new_pos, Constants::kGridSizePxF ) );
    }
    else if ( tile == scene_data.reserved_tile_id() )
    {
      auto entt = Factory::Wall::add_reservedposition( reg(), new_pos );
      reserved_sm->insert( entt, Cmp::Position( new_pos, Constants::kGridSizePxF ) );
    }
  }

  // Solid Layer
  for ( const auto &solid : scene_data.solid_objectlayer() )
  {
    Factory::Wall::add_solid_player( reg(), solid );
    Factory::Wall::add_solid_npc( reg(), solid );
    Factory::Wall::add_no_move_dest( reg(), solid );
  }

  // Multiblock layer
  for ( const auto &[ms_type, pos] : scene_data.multiblock_objectlayer() )
  {
    // allow "npc." markers in the multiblock layer
    if ( ms_type.contains( "npc." ) )
    {
      auto npc_entt = reg().create();
      reg().emplace_or_replace<Cmp::Position>( npc_entt, pos, Constants::kGridSizePxF );
      Factory::Npc::create_npc( m_reg, npc_entt, ms_type );
      continue;
    }

    // Item markers share this layer but are named by item id (e.g. "item.axe"), which isn't
    // itself a valid sprite key - resolve the item's actual sprite type before lookup.
    const Sprites::SpriteMetaType sprite_lookup_type = ms_type.contains( "item." ) ? Sys::ItemStore::instance().get_item( ms_type ).sprite_type
                                                                                   : ms_type;
    const auto &ms = m_sprite_factory.get_spritesheet_by_type( sprite_lookup_type );
    if ( ms_type == "sprite.graveyard.healingspring" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( reg(), pos,
                                                                                                                                      ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.crypt.objective.closed" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Crypt::ObjectiveMultiBlock, Cmp::Crypt::ObjectiveSegment>( reg(),
                                                                                                                                            pos, ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.crypt.altar.inactive" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Altar::MultiBlock, Cmp::Altar::Segment>( reg(), pos, ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.ruin.stairs.up" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::StairsLowerMultiBlock, Cmp::Ruin::StairsSegment>( reg(), pos,
                                                                                                                                         ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.ruin.stairs.down" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::StairsUpperMultiBlock, Cmp::Ruin::StairsSegment>( reg(), pos,
                                                                                                                                         ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.ruin.stairs.balustrade" )
    {
      auto [mb_entt,
            _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::StairsBalustradeMultiBlock, Cmp::Ruin::StairsSegment>( reg(), pos, ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.ruin.stairs.gate" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::StairsGateMultiBlock, Cmp::Ruin::GateSegment>( reg(), pos,
                                                                                                                                      ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type == "sprite.ruin.hex" )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::HexagramMultiBlock, Cmp::Ruin::HexagramSegment>( m_reg, pos,
                                                                                                                                        ms );
      reserved_sm->insert( mb_entt, Cmp::Position( pos, Constants::kGridSizePxF ) );
    }
    else if ( ms_type.contains( "item.plant" ) ) { gen_plant( ms_type, pos, reserved_sm ); }
    else if ( ms_type.contains( "item." ) )
    {
      // prevent infinite respawns in the RuinSceneUpperFloor
      auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( reg() );
      if ( inventory_type == "item.witchesjar" ) continue;

      // make sure we mark the *world* entt as reserved
      auto world_pos_entt = Utils::get_world_pos_entt( m_reg, Cmp::Position( pos, ms.get_sprite_size() ) );
      if ( world_pos_entt != entt::null )
      {
        reg().emplace_or_replace<Cmp::ReservedPosition>( world_pos_entt );
        get_systems_event_queue().trigger( Events::CreateItemEvent( Cmp::Position( pos, ms.get_sprite_size() ), ms_type ) );
      }
    }
  }
}

void LevelGenerator::add_graveyard_exterior_obstacles( float init_chance, const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  auto position_view = reg().view<Cmp::Position>( entt::exclude<Cmp::Player::Character, Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    if ( Cmp::RandomFloat{ 0.f, 1.f }.gen() < init_chance )
    {
      if ( Factory::Obstacle::add_obstacle( reg(), entity, reserved_sm ) )
      {
        m_obstacle_sm->insert( entity, pos_cmp );
        reserved_sm->insert( entity, pos_cmp );
      }
    }
  }
}

void LevelGenerator::decorate_graveyard_exterior_obstacles()
{
  const Sprites::SpriteSheet &ss_main = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.wall.int.main" );
  const Sprites::SpriteSheet &ss_cap = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.wall.int.cap" );

  auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::Player::Character, Cmp::ReservedPosition> );
  for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
  {
    auto uuid = Cmp::UUID::generate();
    auto [_, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.graveyard.wall.int.main" } );

    Factory::Obstacle::decorate_obstacle( reg(), obstacle_entt, obstacle_pos_cmp, ss_main, rand_obst_tex_idx );
    reg().emplace_or_replace<Cmp::UUID>( obstacle_entt, uuid );

    auto cap_entt = reg().create();
    Cmp::Position cap_position( { obstacle_pos_cmp.x(), obstacle_pos_cmp.y() - obstacle_pos_cmp.size.y }, obstacle_pos_cmp.size );
    reg().emplace_or_replace<Cmp::Position>( cap_entt, cap_position );
    Factory::Obstacle::decorate_obstacle( reg(), cap_entt, cap_position, ss_cap, rand_obst_tex_idx, obstacle_pos_cmp.y() + 1, false );
    reg().emplace_or_replace<Cmp::ReservedPosition>( cap_entt );
    reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
    Factory::Obstacle::add_obstacle_cap( reg(), cap_entt );
  }
}

void LevelGenerator::add_ruin_interior_obstacles( float init_chance, const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  auto position_view = reg().view<Cmp::Position>( entt::exclude<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Exit> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    if ( reserved_sm->at( pos_cmp ).empty() )
    {
      if ( Cmp::RandomFloat{ 0.f, 1.f }.gen() < init_chance )
      {
        Factory::Obstacle::add_obstacle( reg(), entity );
        m_obstacle_sm->insert( entity, pos_cmp );
        reserved_sm->insert( entity, pos_cmp );
      }
    }
  }
}

void LevelGenerator::decorate_ruin_interior_obstacles()
{
  const Sprites::SpriteSheet &ss_main = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.wall.int.main" );
  const Sprites::SpriteSheet &ss_cap = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.wall.int.cap" );
  constexpr int ss_main_idx = 0;
  constexpr int ss_cap_idx = 0;

  auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::Player::Character, Cmp::ReservedPosition> );
  for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
  {
    auto uuid = Cmp::UUID::generate();
    Factory::Obstacle::decorate_obstacle( reg(), obstacle_entt, obstacle_pos_cmp, ss_main, ss_main_idx );
    reg().emplace_or_replace<Cmp::UUID>( obstacle_entt, uuid );
    reg().emplace_or_replace<Cmp::Moveable>( obstacle_entt );

    auto cap_entt = reg().create();
    Cmp::Position cap_position( { obstacle_pos_cmp.x(), obstacle_pos_cmp.y() - obstacle_pos_cmp.size.y }, obstacle_pos_cmp.size );
    reg().emplace_or_replace<Cmp::Position>( cap_entt, cap_position );
    Factory::Obstacle::decorate_obstacle( reg(), cap_entt, cap_position, ss_cap, ss_cap_idx, obstacle_pos_cmp.y(), false );
    reg().emplace_or_replace<Cmp::ReservedPosition>( cap_entt );
    reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
    Factory::Obstacle::add_obstacle_cap( reg(), cap_entt );
  }
}

void LevelGenerator::add_ruin_rune_markers()
{
  for ( auto _ : std::views::iota( 0, 5 ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( reg(), {}, Utils::Rnd::ExcludePack<Cmp::ReservedPosition, Cmp::Player::Character>{} );
    reg().emplace_or_replace<Cmp::ReservedPosition>( rnd_entt );
    auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.ruin.runemarking.inactive" } );
    float zorder = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.runemarking.inactive" ).get_zorder( 0 );
    auto rune_entt = Factory::Ruin::create_rune_marker( reg(), rnd_pos, zorder, idx );

    SPDLOG_INFO( "Added rune to {},{}", rnd_pos.x(), rnd_pos.y() );

    m_non_obstacle_sm->insert( rune_entt, rnd_pos );
  }
}

void LevelGenerator::add_lowerfloor_cobwebs( int num_cobwebs, sf::FloatRect scene_dimensions )
{
  auto has_collision = [&]( const Cmp::RectBounds &pos )
  {
    if ( Utils::Collision::check_cmp<Cmp::Ruin::StairsLowerMultiBlock>( reg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::Ruin::Cobweb>( reg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::Exit>( reg(), pos ) ) { return true; }

    // ensure is inside scene
    if ( not Cmp::RectBounds::scaled( pos.position(), pos.size(), 1.5f ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  constexpr auto gridsize = Constants::kGridSizePxF;
  int max_cobwebs = num_cobwebs;
  for ( auto _ : std::views::iota( 0, max_cobwebs ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( reg(), {}, Utils::Rnd::ExcludePack<Cmp::ReservedPosition, Cmp::Player::Character>{} );
    if ( rnd_entt == entt::null ) continue;

    if ( has_collision( Cmp::RectBounds::scaled( { rnd_pos.position }, gridsize, 1 ) ) ) continue;
    auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.ruin.cobweb" } );
    Factory::Ruin::create_cobweb( reg(), rnd_entt, rnd_pos.position, m_sprite_factory.get_spritesheet_by_type( ms ), idx );
    m_non_obstacle_sm->insert( rnd_entt, rnd_pos );
  }
}

void LevelGenerator::gen_graveyard_exterior_multiblocks( const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  auto grave_num_multiplier = Sys::PersistSystem::get<Cmp::Persist::GraveNumMultiplier>( reg() );
  auto max_num_altars = Sys::PersistSystem::get<Cmp::Persist::MaxNumAltars>( reg() );
  auto max_num_crypts = Sys::PersistSystem::get<Cmp::Persist::MaxNumCrypts>( reg() );
  std::size_t max_number_healing_springs = 1;
  std::size_t max_number_ruins = 1;

  // shared spawn-location lookup for every multiblock placed below
  auto find_spawn_pos = [&]( const Sprites::SpriteSheet &ms ) -> std::optional<Cmp::Position>
  {
    auto [random_entity, random_origin_position] = find_spawn_location( ms, 0 );
    if ( random_entity == entt::null )
    {
      SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
      return std::nullopt;
    }
    return random_origin_position;
  };

  // GRAVES
  auto grave_meta_types = m_sprite_factory.get_all_sprite_types_by_pattern( R"(graves\.\w+\.closed$)" );
  if ( grave_meta_types.size() < 2 ) { SPDLOG_WARN( "No GRAVE spritesheets found in SpriteFactory" ); }
  else
  {
    SPDLOG_DEBUG( "Found {}, {}", grave_meta_types[0], grave_meta_types[1] );
    auto max_num_graves = static_cast<size_t>( max_num_altars.get_value() * grave_num_multiplier.get_value() );
    for ( std::size_t i = 0; i < max_num_graves; ++i )
    {
      auto [sprite_metatype, index] = m_sprite_factory.get_random_type_and_texture_index( grave_meta_types );
      SPDLOG_DEBUG( "Selected {}, {}", sprite_metatype, unused_index );
      const auto &spritesheet = m_sprite_factory.get_spritesheet_by_type( sprite_metatype );
      if ( auto pos = find_spawn_pos( spritesheet ) )
      {
        auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Grave::MultiBlock, Cmp::Grave::Segment>( reg(), pos->position,
                                                                                                                            spritesheet, index );
        reserved_sm->insert( mb_entt, pos.value() );
      }
    }
  }

  // ALTARS
  const auto &altar_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.altar.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    if ( auto pos = find_spawn_pos( altar_spritesheet ) )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Altar::MultiBlock, Cmp::Altar::Segment>( reg(), pos->position,
                                                                                                                          altar_spritesheet );
      reserved_sm->insert( mb_entt, pos.value() );
    }
  }

  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  const auto &crypt_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.crypt.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    if ( auto pos = find_spawn_pos( crypt_spritesheet ) )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Crypt::BuildingMultiBlock, Cmp::Crypt::BuildingSegment>(
          reg(), pos->position, crypt_spritesheet );
      reserved_sm->insert( mb_entt, pos.value() );
      SPDLOG_INFO( "Added {} to {},{}", crypt_spritesheet.get_sprite_type(), pos->position.x, pos->position.y );
    }
  }

  const auto &healingspring_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.building.healingspring" );
  for ( std::size_t i = 0; i < max_number_healing_springs; ++i )
  {
    if ( auto pos = find_spawn_pos( healingspring_spritesheet ) )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>(
          reg(), pos->position, healingspring_spritesheet );
      reserved_sm->insert( mb_entt, pos.value() );
      SPDLOG_INFO( "Added {} to {},{}", healingspring_spritesheet.get_sprite_type(), pos->position.x, pos->position.y );
    }
  }

  const auto &ruin_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.ruin" );
  for ( std::size_t i = 0; i < max_number_ruins; ++i )
  {
    if ( auto pos = find_spawn_pos( ruin_spritesheet ) )
    {
      auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::Ruin::BuildingMultiBlock, Cmp::Ruin::BuildingSegment>(
          reg(), pos->position, ruin_spritesheet );
      reserved_sm->insert( mb_entt, pos.value() );
      SPDLOG_INFO( "Added {} to {},{}", ruin_spritesheet.get_sprite_type(), pos->position.x, pos->position.y );
    }
  }
}

std::pair<entt::entity, Cmp::Position> LevelGenerator::find_spawn_location( const Sprites::SpriteSheet &ms, unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        reg(), Utils::Rnd::IncludePack<>{}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::Player::Character>{}, current_seed );

    auto lo_sprite_size = m_sprite_factory.get_sprite_size_by_type( ms.get_sprite_type() );
    auto new_lo_hitbox = Cmp::RectBounds::scaled( random_pos.position, lo_sprite_size, 1.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : reg().view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : reg().view<Cmp::Grave::Segment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for altar collisions
      for ( auto [entity, altar_cmp, altar_pos_cmp] : reg().view<Cmp::Altar::Segment, Cmp::Position>().each() )
      {
        if ( altar_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for crypt collisions
      for ( auto [entity, crypt_cmp, crypt_pos_cmp] : reg().view<Cmp::Crypt::BuildingSegment, Cmp::Position>().each() )
      {
        if ( crypt_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, spring_cmp, spring_pos_cmp] : reg().view<Cmp::HealingSpringBuildingSegment, Cmp::Position>().each() )
      {
        if ( spring_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, ruin_cmp, ruin_pos_cmp] : reg().view<Cmp::Ruin::BuildingSegment, Cmp::Position>().each() )
      {
        if ( ruin_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : reg().view<Cmp::Crypt::ObjectiveSegment, Cmp::Position>().each() )
      {
        if ( crypt_obj_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for reserved position collisions
      for ( auto [entity, reserved_cmp, reserved_pos_cmp] : reg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
      {
        if ( reserved_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for spawn area collisions
      for ( auto [entity, spawn_cmp, spawn_pos_cmp] : reg().view<Cmp::SpawnArea, Cmp::Position>().each() )
      {
        if ( spawn_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for playable character collisions
      for ( auto [entity, player_cmp, player_pos_cmp] : reg().view<Cmp::Player::Character, Cmp::Position>().each() )
      {
        if ( player_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      return true;
    };

    if ( is_valid() )
    {
      if ( current_seed != seed && seed > 0 )
      {
        SPDLOG_WARN( "Large Obstacle spawn: original seed {} was invalid, used seed {} instead (attempt {})", seed, current_seed, attempts + 1 );
      }
      auto new_entt = reg().create();
      return { new_entt, reg().emplace_or_replace<Cmp::Position>( new_entt, random_pos.position, random_pos.size ) };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

bool LevelGenerator::gen_plant( const std::string &plant_type, sf::Vector2f pos, const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  const auto &plant_ss = m_sprite_factory.get_spritesheet_by_type( "sprite." + plant_type );

  // Plants can span more than one grid row (e.g. 1x2), so every cell in the footprint - not
  // just the origin - must be a real, unreserved world tile. Otherwise a plant can overlap a
  // reserved tile it doesn't share an entity with, e.g. the player spawn.
  auto plant_grid_size = plant_ss.get_grid_size();
  bool footprint_clear = true;
  for ( int gy = 0; gy < plant_grid_size.y && footprint_clear; ++gy )
  {
    for ( int gx = 0; gx < plant_grid_size.x; ++gx )
    {
      sf::Vector2f pos_offset( static_cast<float>( gx ) * Constants::kGridSizePxF.x, static_cast<float>( gy ) * Constants::kGridSizePxF.y );
      Cmp::Position cell_pos( pos + pos_offset, Constants::kGridSizePxF );
      if ( Utils::get_world_pos_entt( reg(), cell_pos ) == entt::null || not reserved_sm->at( cell_pos ).empty() )
      {
        footprint_clear = false;
        break;
      }
    }
  }

  if ( footprint_clear )
  {
    // now create the plant at a new entt
    auto [mb_entt, _] = Factory::Multiblock::add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( reg(), pos, plant_ss );

    // Add the worlditem now so we don't have to look it up later when digging up the plant
    reg().emplace_or_replace<Cmp::WorldItem>( mb_entt, Sys::ItemStore::instance().get_item( plant_type ) );

    // Protect every cell covered by this plant: segment entities carry the UUID and
    // are at each tile position. Insert them so subsequent placement calls see them
    // as reserved via the O(1) navmesh check.
    auto *mb_uuid = reg().try_get<Cmp::UUID>( mb_entt );
    if ( mb_uuid )
    {
      for ( auto [seg_entt, seg_cmp, seg_pos, seg_uuid] : reg().view<Cmp::PlantSegment, Cmp::Position, Cmp::UUID>().each() )
      {
        if ( seg_uuid == *mb_uuid ) reserved_sm->insert( seg_entt, seg_pos );
      }
    }
    return true;
  }
  return false;
}

std::vector<entt::entity> LevelGenerator::gen_random_plants( sf::Vector2u map_grid_size, const PathFinding::SpatialHashGridSharedPtr &reserved_sm )
{
  std::vector<entt::entity> assigned_entts;

  auto num_plants = map_grid_size.x * map_grid_size.y / 200;

  for ( std::size_t i = 0; i < num_plants; ++i )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        reg(), {}, Utils::Rnd::ExcludePack<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    std::vector<std::string> plant_item_type_list{ "item.plant1", "item.plant2", "item.plant3", "item.plant4",  "item.plant5",  "item.plant6",
                                                   "item.plant7", "item.plant8", "item.plant9", "item.plant10", "item.plant11", "item.plant12" };
    auto chosen_plant_item_type = plant_item_type_list.at( Cmp::RandomInt( 0, static_cast<int>( plant_item_type_list.size() - 1 ) ).gen() );
    if ( gen_plant( chosen_plant_item_type, random_pos.position, reserved_sm ) ) { assigned_entts.push_back( random_entity ); }
  }
  return assigned_entts;
}

void LevelGenerator::reset()
{
  m_obstacle_sm->clear();
  m_void_sm->clear();
  m_non_obstacle_sm->clear();
}

} // namespace Game::Sys::ProcGen