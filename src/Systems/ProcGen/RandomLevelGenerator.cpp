#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys::ProcGen
{

RandomLevelGenerator::RandomLevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                            Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void RandomLevelGenerator::gen_rectangle_gamearea( sf::Vector2u map_grid_size, Cmp::RectBounds &player_start_area, Sprites::SpriteMetaType wall_type,
                                                   SpawnArea spawnarea )
{

  unsigned int w = map_grid_size.x;
  unsigned int h = map_grid_size.y;

  for ( unsigned int x = 0; x < w; x++ )
  {
    for ( unsigned int y = 0; y < h; y++ )
    {
      sf::Vector2f new_pos( { x * Constants::kGridSquareSizePixelsF.x, y * Constants::kGridSquareSizePixelsF.y } );

      // condition for left, right, top, bottom borders
      bool isBorder = ( x == 0 ) || ( y == 0 ) || ( y == 1 ) || ( x == w - 1 ) || ( y == h - 1 ) || ( y == h - 2 );

      // auto [sprite_type, sprite_idx] = m_sprite_factory.get_random_type_and_texture_index( { wall_type } );
      if ( isBorder )
      {
        enum SpriteIdx {
          TOPLEFT = 0,
          TOPCENTER = 1,
          TOPRIGHT = 2,
          LEFT = 3,
          TOPFRONT = 4,
          RIGHT = 5,
          BOTTOMLEFT = 6,
          BOTTOMCENTER = 7,
          BOTTOMRIGHT = 8,
          BOTTOMFRONTLEFT = 9,
          BOTTOMFRONTCENTER = 10,
          BOTTOMFRONTRIGHT = 11,
        };

        // new_pos.y + grid_height = in front of player
        // new_pos.y - grid_height = behind the player
        float grid_height = Constants::kGridSquareSizePixelsF.y;

        // Top-left corner
        if ( x == 0 && y == 0 ) Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::TOPLEFT, new_pos.y + grid_height );

        // Top edge (excluding corners)
        else if ( x > 0 && x < w - 1 && y == 0 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::TOPCENTER, new_pos.y + grid_height );

        // Top front side (facing the player, excluding corners)
        else if ( x > 0 && x < w - 1 && y == 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::TOPFRONT, new_pos.y - 2 * grid_height, Factory::SolidWall::FALSE );

        // Top-right corner
        else if ( x == w - 1 && y == 0 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::TOPRIGHT, new_pos.y + grid_height );

        // Left edge (excluding corners)
        else if ( x == 0 && y > 0 && y < h - 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::LEFT, new_pos.y + grid_height );

        // Right edge (excluding corners)
        else if ( x == w - 1 && y > 0 && y < h - 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::RIGHT, new_pos.y + grid_height );

        // Bottom-left corner
        else if ( x == 0 && y == h - 2 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMLEFT, new_pos.y + grid_height, Factory::SolidWall::FALSE );

        // Bottom edge (excluding corners)
        else if ( x > 0 && x < w - 1 && y == h - 2 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMCENTER, new_pos.y + grid_height, Factory::SolidWall::FALSE );

        // Bottom-right corner
        else if ( x == w - 1 && y == h - 2 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMRIGHT, new_pos.y + grid_height, Factory::SolidWall::FALSE );

        // Bottom-left front side corner (facing the player)
        else if ( x == 0 && y == h - 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMFRONTLEFT, new_pos.y + grid_height );

        // Bottom front side (facing the player, excluding corners)
        else if ( x > 0 && x < w - 1 && y == h - 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMFRONTCENTER, new_pos.y + grid_height );

        // Bottom-right front side corner (facing the player)
        else if ( x == w - 1 && y == h - 1 )
          Factory::add_wall_entity( getReg(), new_pos, wall_type, SpriteIdx::BOTTOMFRONTRIGHT, new_pos.y + grid_height );
      }

      bool isInside = ( x > 0 ) && ( y > 1 ) && ( x < w - 1 ) && ( y < h - 1 );
      if ( isInside )
      {
        // create world position entity, mark spawn area if in player start area
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        if ( spawnarea == SpawnArea::TRUE )
        {
          if ( pos_cmp.findIntersection( player_start_area.getBounds() ) ) { Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f ); }
        }
        // track contiguous list of positions for proc gen
        m_data.push_back( entity );
      }
    }
  }
  SPDLOG_INFO( "Generated {} positions for random level.", m_data.size() );
}

void RandomLevelGenerator::gen_circular_gamearea( sf::Vector2u map_grid_size, Cmp::RectBounds &player_start_area )
{

  unsigned int w = map_grid_size.x;
  unsigned int h = map_grid_size.y;

  int cx = w / 2; // center in tiles
  int cy = h / 2;
  int r = std::min( w, h ) / 2; // radius in tiles

  int border = 2; // thickness in tiles

  int rOuter2 = r * r;
  int rInner = r - border;
  int rInner2 = rInner * rInner;

  for ( unsigned int x = 0; x < w; ++x )
  {
    for ( unsigned int y = 0; y < h; ++y )
    {
      int dx = static_cast<int>( x ) - cx;
      int dy = static_cast<int>( y ) - cy;
      int d2 = dx * dx + dy * dy;

      auto kGridSquareSizePixels = Constants::kGridSquareSizePixels;
      sf::Vector2f new_pos( x * kGridSquareSizePixels.x, y * kGridSquareSizePixels.y );

      if ( d2 <= rInner2 )
      {

        // Create world position entity
        // We don't add any "CRYPT.interior_sb" obstacles until CryptSystem.
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        // Mark this position as spawn area if in player start area
        if ( pos_cmp.findIntersection( player_start_area.getBounds() ) ) { Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f ); }

        // track the contiguous creation order entities so we can easily find its neighbours later
        m_data.push_back( entity );
      }
      else if ( d2 <= rOuter2 )
      {
        // but we do add "CRYPT.interior_sb" for walls
        Factory::add_wall_entity( getReg(), new_pos, "CRYPT.interior_sb", 0, Utils::get_player_position( getReg() ).position.y );
      }
      else
      {
        // outside circle (no tile)
      }
    }
  }
}

void RandomLevelGenerator::gen_cross_gamearea( sf::Vector2u map_grid_size, Cmp::RectBounds &player_start_area, int vertArmHalfWidth,
                                               int horizArmHalfWidth, int horizOffset )
{
  auto start_room_entity = getReg().create();
  getReg().emplace_or_replace<Cmp::CryptRoomStart>( start_room_entity, player_start_area.position(), player_start_area.size() );

  unsigned int w = map_grid_size.x; // in tiles
  unsigned int h = map_grid_size.y; // in tiles

  int cx = w / 2;
  int cy = h / 2;

  // arm half-length in tiles
  int vertHalfLength = h;
  int horizHalfLength = w;

  // helper: is (x,y) part of the cross?
  auto inCross = [&]( int x, int y ) -> bool
  {
    if ( x < 0 || y < 0 || x >= static_cast<int>( w ) || y >= static_cast<int>( h ) ) return false;

    int dx = x - cx;
    int dy = y - cy;

    // Vertical arm (centered on cx, cy)
    bool inVertical = std::abs( dx ) <= vertArmHalfWidth && std::abs( dy ) <= vertHalfLength;

    // Horizontal arm (centered on cy + horizOffset)
    bool inHorizontal = std::abs( ( dy - horizOffset ) ) <= horizArmHalfWidth && std::abs( dx ) <= horizHalfLength;

    return inVertical || inHorizontal;
  };

  for ( int x = 0; x < static_cast<int>( w ); ++x )
  {
    for ( int y = 0; y < static_cast<int>( h ); ++y )
    {

      auto kGridSquareSizePixels = Constants::kGridSquareSizePixels;
      sf::Vector2f new_pos( x * kGridSquareSizePixels.x, y * kGridSquareSizePixels.y );

      bool inside = inCross( x, y );
      if ( !inside ) continue; // Factory::createVoidPosition( getReg(), new_pos ); // outside cross, skip

      // 1-tile border: any 4-neighbor outside the cross
      bool isBorder = !inCross( x - 1, y ) || !inCross( x + 1, y ) || !inCross( x, y - 1 ) || !inCross( x, y + 1 );

      if ( isBorder ) { Factory::add_wall_entity( getReg(), new_pos, "CRYPT.interior_sb", 0, Utils::get_player_position( getReg() ).position.y ); }
      else
      {
        // create world position entity, mark spawn area if in player start area
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        if ( pos_cmp.findIntersection( player_start_area.getBounds() ) ) { Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f ); }

        // track the contiguous creation order entities so we can easily find its neighbours later
        m_data.push_back( entity );
      }
    }
  }
}

void RandomLevelGenerator::gen_graveyard_exterior_obstacles()
{

  auto position_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {

    if ( Cmp::RandomInt{ 0, 1 }.gen() == 1 )
    {
      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "ROCK" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "ROCK" ).y;
      // Set the z-order value so that the rock obstacles are rendered above everything else
      Factory::createObstacle( getReg(), entity, pos_cmp, obst_type, rand_obst_tex_idx, ( zorder * 2.f ) );
    }
  }
}

void RandomLevelGenerator::gen_graveyard_exterior_multiblocks()
{
  auto grave_num_multiplier = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::GraveNumMultiplier>( getReg() );
  auto max_num_altars = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MaxNumAltars>( getReg() );
  auto max_num_crypts = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MaxNumCrypts>( getReg() );
  std::size_t max_number_holywells = 1;
  std::size_t max_number_ruins = 1;

  // GRAVES
  auto grave_meta_types = m_sprite_factory.get_all_sprite_types_by_pattern( "^GRAVE\\d+\\.closed$" );
  if ( grave_meta_types.empty() ) { SPDLOG_WARN( "No GRAVE multisprites found in SpriteFactory" ); }
  else
  {
    uint8_t max_num_graves = max_num_altars.get_value() * grave_num_multiplier.get_value();
    for ( std::size_t i = 0; i < max_num_graves; ++i )
    {
      auto [sprite_metatype, unused_index] = m_sprite_factory.get_random_type_and_texture_index( grave_meta_types );
      auto &multisprite = m_sprite_factory.get_multisprite_by_type( sprite_metatype );
      do_gen_graveyard_exterior_multiblock( multisprite, 0 );
    }
  }

  // ALTARS
  auto &altar_multisprite = m_sprite_factory.get_multisprite_by_type( "ALTAR.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( altar_multisprite, 0 );
  }

  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  auto &crypt_multisprite = m_sprite_factory.get_multisprite_by_type( "CRYPT.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( crypt_multisprite, 0 );
  }

  auto &holywell_multisprite = m_sprite_factory.get_multisprite_by_type( "HOLYWELL.exterior_building" );
  for ( std::size_t i = 0; i < max_number_holywells; ++i )
  {
    do_gen_graveyard_exterior_multiblock( holywell_multisprite, 0 );
  }

  auto &ruin_multisprite = m_sprite_factory.get_multisprite_by_type( "RUIN.exterior_building" );
  for ( std::size_t i = 0; i < max_number_ruins; ++i )
  {
    do_gen_graveyard_exterior_multiblock( ruin_multisprite, 0 );
  }
}

void RandomLevelGenerator::do_gen_graveyard_exterior_multiblock( const Sprites::MultiSprite &ms, unsigned long seed )
{
  auto [random_entity, random_origin_position] = find_spawn_location( ms, seed );
  if ( random_entity == entt::null )
  {
    SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
    return;
  }

  if ( ms.get_sprite_type().contains( "ALTAR" ) )
  {
    Factory::createMultiblock<Cmp::AltarMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type().contains( "GRAVE" ) )
  {
    Factory::createMultiblock<Cmp::GraveMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type() == "CRYPT.closed" )
  {
    Factory::createMultiblock<Cmp::CryptMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::CryptMultiBlock, Cmp::CryptSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type() == "HOLYWELL.exterior_building" )
  {
    Factory::createMultiblock<Cmp::HolyWellMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type() == "RUIN.exterior_building" )
  {
    Factory::createMultiblock<Cmp::RuinBuildingMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::RuinBuildingMultiBlock, Cmp::RuinSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported multisprite type: {}", ms.get_sprite_type() );
    return;
  }
}

void RandomLevelGenerator::gen_crypt_initial_interior()
{
  SPDLOG_INFO( "Generating crypt interior obstacles." );
  auto position_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  // auto room_view = getReg().view<Cmp::CryptRoomClosed>();
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    // skip if inside a start/end/open room
    bool add_interior_wall = true;
    auto start_room_view = getReg().view<Cmp::CryptRoomStart>();
    for ( auto [start_room_entity, start_room_cmp] : start_room_view.each() )
    {
      if ( pos_cmp.findIntersection( start_room_cmp ) ) add_interior_wall = false;
    }
    auto end_room_view = getReg().view<Cmp::CryptRoomEnd>();
    for ( auto [end_room_entity, end_room_cmp] : end_room_view.each() )
    {
      if ( pos_cmp.findIntersection( end_room_cmp ) ) add_interior_wall = false;
    }
    auto open_room_view = getReg().view<Cmp::CryptRoomOpen>();
    for ( auto [open_room_entity, open_room_cmp] : open_room_view.each() )
    {
      if ( pos_cmp.findIntersection( open_room_cmp ) ) add_interior_wall = false;
    }

    if ( add_interior_wall )
    {
      auto [obst_type, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = m_sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      // Set the z-order value so that the obstacles are rendered above everything else
      Factory::createObstacle( getReg(), entity, pos_cmp, obst_type, 2, ( zorder * 2.f ) );
    }
  }
}

void RandomLevelGenerator::gen_crypt_main_objective( sf::Vector2u map_grid_size )
{
  auto map_grid_sizef = sf::Vector2f( static_cast<float>( map_grid_size.x ) * Constants::kGridSquareSizePixelsF.x,
                                      static_cast<float>( map_grid_size.y ) * Constants::kGridSquareSizePixelsF.y );
  auto kGridSquareSizePixelsF = Constants::kGridSquareSizePixelsF;
  // target position for the objective: always center top of the map
  const auto &ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_objective_closed" );

  float centered_x = ( map_grid_sizef.x / 2.f ) - ( ms.getSpriteSizePixels().x / 2.f ) + kGridSquareSizePixelsF.x;
  Cmp::Position objective_position( { centered_x, kGridSquareSizePixelsF.y * 2.f }, ms.getSpriteSizePixels() );

  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, objective_position.position, objective_position.size );

  SPDLOG_INFO( "Placing main crypt objective at position ({}, {})", objective_position.position.x, objective_position.position.y );
  Factory::createMultiblock<Cmp::CryptObjectiveMultiBlock>( getReg(), entity, objective_position, ms );
  Factory::createMultiblockSegments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( getReg(), entity, objective_position, ms );

  // while we're here, carve out a room for the objective sprite. These position/size modifiers are trial and error
  // whilst we decide on the final objective MB sprite dimensions

  auto end_room_entity = getReg().create();
  getReg().emplace_or_replace<Cmp::CryptRoomEnd>(
      end_room_entity, sf::Vector2f{ objective_position.position.x, objective_position.position.y },
      sf::Vector2f{ objective_position.size.x, objective_position.size.y + ( kGridSquareSizePixelsF.y * 2.f ) } );
}

void RandomLevelGenerator::gen_crypt_interior_multiblocks()
{

  for ( int i = 0; i < 30; i++ )
  {
    auto [ms_type, ms_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_mb1x3" } );
    auto &ms = m_sprite_factory.get_multisprite_by_type( ms_type );
    auto [random_entity, random_origin_position] = find_spawn_location( ms, 0 );
    if ( random_entity == entt::null )
    {
      SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
      return;
    }
    Factory::createMultiblock<Cmp::CryptInteriorMultiBlock>( getReg(), random_entity, random_origin_position, ms, ms_idx );
    Factory::createMultiblockSegments<Cmp::CryptInteriorMultiBlock, Cmp::CryptInteriorSegment>( getReg(), random_entity, random_origin_position, ms );
  }
}

void RandomLevelGenerator::gen_loot_containers( sf::Vector2u map_grid_size )
{
  auto num_loot_containers = map_grid_size.x * map_grid_size.y / 120; // one loot container per N grid squares

  for ( std::size_t i = 0; i < num_loot_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    float zorder = m_sprite_factory.get_sprite_size_by_type( "POT" ).y;

    Factory::createLootContainer( getReg(), random_entity, random_origin_position, "POT", 0, zorder );
  }
}

void RandomLevelGenerator::gen_npc_containers( sf::Vector2u map_grid_size )
{
  auto num_npc_containers = map_grid_size.x * map_grid_size.y / 120; // one NPC container per N grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [npc_type, rand_npc_tex_idx] =
      m_sprite_factory.get_random_type_and_texture_index( {
        "BONES"
      } );
    // clang-format on

    Factory::createNpcContainer( getReg(), random_entity, random_origin_position, npc_type, rand_npc_tex_idx, 0.f );
  }
}

void RandomLevelGenerator::gen_plants( sf::Vector2u map_grid_size )
{
  auto num_plants = map_grid_size.x * map_grid_size.y / 200;

  for ( std::size_t i = 0; i < num_plants; ++i )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // select a random number within the range of possible flora CarryItems
    auto [rand_plant_type, rnd_plant_idx] = m_sprite_factory.get_random_type_and_texture_index(
        { "CARRYITEM.plant1", "CARRYITEM.plant2", "CARRYITEM.plant3", "CARRYITEM.plant4", "CARRYITEM.plant5", "CARRYITEM.plant6", "CARRYITEM.plant7",
          "CARRYITEM.plant8", "CARRYITEM.plant9", "CARRYITEM.plant10", "CARRYITEM.plant11", "CARRYITEM.plant12" } );

    Factory::createPlantObstacle( getReg(), random_pos, rand_plant_type, 0.f );
    SPDLOG_INFO( "Created plant at {},{}", random_pos.position.x, random_pos.position.y );
  }
}

std::pair<entt::entity, Cmp::Position> RandomLevelGenerator::find_spawn_location( const Sprites::MultiSprite &ms, unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        getReg(), Utils::Rnd::IncludePack<>{}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::PlayerCharacter>{}, current_seed );

    auto lo_sprite_size = m_sprite_factory.get_sprite_size_by_type( ms.get_sprite_type() );
    Cmp::RectBounds new_lo_hitbox( random_pos.position, lo_sprite_size, 1.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : getReg().view<Cmp::GraveSegment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for altar collisions
      for ( auto [entity, altar_cmp, altar_pos_cmp] : getReg().view<Cmp::AltarSegment, Cmp::Position>().each() )
      {
        if ( altar_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for crypt collisions
      for ( auto [entity, crypt_cmp, crypt_pos_cmp] : getReg().view<Cmp::CryptSegment, Cmp::Position>().each() )
      {
        if ( crypt_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, holywell_cmp, holywell_pos_cmp] : getReg().view<Cmp::HolyWellSegment, Cmp::Position>().each() )
      {
        if ( holywell_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, ruin_cmp, ruin_pos_cmp] : getReg().view<Cmp::RuinSegment, Cmp::Position>().each() )
      {
        if ( ruin_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : getReg().view<Cmp::CryptObjectiveSegment, Cmp::Position>().each() )
      {
        if ( crypt_obj_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for reserved position collisions
      for ( auto [entity, reserved_cmp, reserved_pos_cmp] : getReg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
      {
        if ( reserved_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for spawn area collisions
      for ( auto [entity, spawn_cmp, spawn_pos_cmp] : getReg().view<Cmp::SpawnArea, Cmp::Position>().each() )
      {
        if ( spawn_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for playable character collisions
      for ( auto [entity, player_cmp, player_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
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
      auto new_entt = getReg().create();
      return { new_entt, getReg().emplace_or_replace<Cmp::Position>( new_entt, random_pos.position, random_pos.size ) };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

} // namespace ProceduralMaze::Sys::ProcGen