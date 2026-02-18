#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptPassageSpikeTrap.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomLavaPit.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Random.hpp>

namespace ProceduralMaze::Factory
{

void create_initial_crypt_rooms( entt::registry &reg, sf::Vector2u map_grid_size )
{
  const auto &grid_square_size = Constants::kGridSizePxF;
  const int min_room_width = 3;
  const int min_room_height = 3;
  const int max_room_width = 8;
  const int max_room_height = 8;
  const int max_distance_between_rooms = 2;
  const int max_attempts = 5000;

  int room_count = 0;
  int current_attempt = 0;
  while ( room_count < 20 )
  {

    int room_width = Cmp::RandomInt{ min_room_width, max_room_width }.gen();
    int room_height = Cmp::RandomInt{ min_room_height, max_room_height }.gen();
    auto [entt, pos] = Utils::Rnd::get_random_position( reg, Utils::Rnd::IncludePack<Cmp::Neighbours>{}, {}, 0 );
    Cmp::CryptRoomClosed new_room( pos.position, { room_width * grid_square_size.x, room_height * grid_square_size.y } );
    SPDLOG_DEBUG( "Generated new room at ({}, {}) size ({}, {})", new_room.position.x, new_room.position.y, new_room.size.x, new_room.size.y );

    auto is_min_distance_ok = [&]( const Cmp::CryptRoomClosed &existing_room, const Cmp::CryptRoomClosed &new_room ) -> bool
    {
      float distance_x = std::max( 0.f, std::max( existing_room.position.x - ( new_room.position.x + new_room.size.x ),
                                                  new_room.position.x - ( existing_room.position.x + existing_room.size.x ) ) );
      float distance_y = std::max( 0.f, std::max( existing_room.position.y - ( new_room.position.y + new_room.size.y ),
                                                  new_room.position.y - ( existing_room.position.y + existing_room.size.y ) ) );
      float distance = std::sqrt( distance_x * distance_x + distance_y * distance_y );
      return distance >= static_cast<float>( max_distance_between_rooms ) * grid_square_size.x;
    };

    auto check_collision = [&]( const auto &existing_object ) -> bool
    { return new_room.findIntersection( existing_object ) || !is_min_distance_ok( existing_object, new_room ); };

    bool overlaps_existing = false;

    // make sure new_room area does not fall outside map_grid_size
    if ( !Utils::isInBounds( new_room.position, new_room.size, map_grid_size ) ) { overlaps_existing = true; }

    // check for intersection with existing rooms
    if ( !overlaps_existing )
    {
      auto room_view = reg.view<Cmp::CryptRoomClosed>();
      for ( auto [existing_entity, existing_room] : room_view.each() )
      {
        if ( check_collision( existing_room ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with walls
    if ( !overlaps_existing )
    {
      auto wall_view = reg.view<Cmp::Wall, Cmp::Position>();
      for ( auto [wall_entity, wall_cmp, wall_pos_cmp] : wall_view.each() )
      {
        if ( check_collision( wall_pos_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with start room
    if ( !overlaps_existing )
    {
      auto start_room_view = reg.view<Cmp::CryptRoomStart>();
      for ( auto [start_room_entity, start_room_cmp] : start_room_view.each() )
      {
        if ( check_collision( start_room_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // check for intersection with end room
    if ( !overlaps_existing )
    {
      auto end_room_view = reg.view<Cmp::CryptRoomEnd>();
      for ( auto [end_room_entity, end_room_cmp] : end_room_view.each() )
      {
        if ( check_collision( end_room_cmp ) )
        {
          overlaps_existing = true;
          break;
        }
      }
    }

    // now check the result
    if ( !overlaps_existing )
    {
      auto entity = reg.create();
      reg.emplace<Cmp::CryptRoomClosed>( entity, std::move( new_room ) );

      room_count++;
      SPDLOG_INFO( "Added new crypt room entity {}, total rooms: {}", entt::to_integral( entity ), room_count );
    }
    else
    {
      SPDLOG_DEBUG( "New room overlaps existing room, discarded. ({}/{})", current_attempt, max_attempts );
      current_attempt++;
    }
    if ( current_attempt >= max_attempts )
    {
      SPDLOG_WARN( "Max attempts reached when generating crypt rooms, stopping." );
      return;
    }
  }

  SPDLOG_INFO( "Total rooms: {}", room_count );
  // Currently no special logic needed; placeholder for future use
}

entt::entity CreateCryptLever( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder )
{
  auto entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entt, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::CryptLever>( entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type, sprite_idx );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  return entt;
}

void DestroyCryptLever( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::CryptLever>( entt ) ) reg.remove<Cmp::CryptLever>( entt );
  if ( reg.all_of<Cmp::Position>( entt ) ) reg.remove<Cmp::Position>( entt );
  if ( reg.all_of<Cmp::SpriteAnimation>( entt ) ) reg.remove<Cmp::SpriteAnimation>( entt );
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) reg.remove<Cmp::ZOrderValue>( entt );
  if ( reg.valid( entt ) ) { reg.destroy( entt ); }
}

entt::entity CreateCryptChest( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder )
{
  auto entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entt, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::CryptChest>( entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, false, sprite_type, sprite_idx );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  return entt;
}

void DestroyCryptChest( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::CryptChest>( entt ) ) reg.remove<Cmp::CryptLever>( entt );
  if ( reg.all_of<Cmp::Position>( entt ) ) reg.remove<Cmp::Position>( entt );
  if ( reg.all_of<Cmp::SpriteAnimation>( entt ) ) reg.remove<Cmp::SpriteAnimation>( entt );
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) reg.remove<Cmp::ZOrderValue>( entt );
  if ( reg.valid( entt ) ) { reg.destroy( entt ); }
}

void createCryptLavaPit( entt::registry &reg, const Cmp::CryptRoomOpen &room )
{
  // add the lava pit area
  sf::Vector2f adjusted_position = { room.position.x + Constants::kGridSizePxF.x, room.position.y + Constants::kGridSizePxF.y };
  sf::Vector2f adjusted_size = { room.size.x - ( Constants::kGridSizePxF.x * 2 ), room.size.y - ( Constants::kGridSizePxF.y * 2 ) };
  sf::FloatRect lava_pit_bounds( adjusted_position, adjusted_size );

  auto lava_pit_entt = reg.create();
  reg.emplace<Cmp::CryptRoomLavaPit>( lava_pit_entt, lava_pit_bounds );

  // add the inidividual lava cells
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( reg.all_of<Cmp::FootStepTimer>( pos_entt ) ) continue;      // dont add lava to footstep positions
    if ( not lava_pit_bounds.findIntersection( pos_cmp ) ) continue; // only add lava to this lava pit
    auto lava_cell_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( lava_cell_entt, pos_cmp.position, pos_cmp.size );
    reg.emplace_or_replace<Cmp::NpcNoPathFinding>( lava_cell_entt );
    reg.emplace_or_replace<Cmp::CryptRoomLavaPitCell>( lava_cell_entt, pos_cmp.position, pos_cmp.size );
    reg.emplace_or_replace<Cmp::SpriteAnimation>( lava_cell_entt, 0, 0, true, "CRYPT.interior_lava", 0 );
    // reg.emplace<Cmp::ZOrderValue>( lava_cell_entt, pos_cmp.size.y );
  }
}

void destroyCryptLavaPit( entt::registry &reg, entt::entity entt )
{
  auto lava_pit_cmp = reg.try_get<Cmp::CryptRoomLavaPit>( entt );
  if ( lava_pit_cmp )
  {
    for ( auto [lava_Cell_entt, lava_cell_cmp] : reg.view<Cmp::CryptRoomLavaPitCell>().each() )
    {
      if ( not lava_cell_cmp.findIntersection( *lava_pit_cmp ) ) continue;
      reg.destroy( lava_Cell_entt );
    }
  }
  reg.destroy( entt );
}

void addSpikeTrap( entt::registry &reg, const entt::entity entt, const int passage_id )
{
  auto pblock_cmp = reg.try_get<Cmp::CryptPassageBlock>( entt );
  if ( not pblock_cmp ) return;
  sf::Vector2f position = *pblock_cmp;

  auto spike_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( spike_entt, position, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( spike_entt, 0, 0, false, "CRYPT.interior_spiketrap", 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( spike_entt, position.y - 16.f ); // always behind player
  reg.emplace_or_replace<Cmp::CryptPassageSpikeTrap>( spike_entt, position, passage_id );
}

void gen_crypt_initial_interior( entt::registry &reg, Sprites::SpriteFactory &sprite_factory )
{
  SPDLOG_INFO( "Generating crypt interior obstacles." );
  auto position_view = reg.view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  // auto room_view = reg.view<Cmp::CryptRoomClosed>();
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    // skip if inside a start/end/open room
    bool add_interior_wall = true;
    auto start_room_view = reg.view<Cmp::CryptRoomStart>();
    for ( auto [start_room_entity, start_room_cmp] : start_room_view.each() )
    {
      if ( pos_cmp.findIntersection( start_room_cmp ) ) add_interior_wall = false;
    }
    auto end_room_view = reg.view<Cmp::CryptRoomEnd>();
    for ( auto [end_room_entity, end_room_cmp] : end_room_view.each() )
    {
      if ( pos_cmp.findIntersection( end_room_cmp ) ) add_interior_wall = false;
    }
    auto open_room_view = reg.view<Cmp::CryptRoomOpen>();
    for ( auto [open_room_entity, open_room_cmp] : open_room_view.each() )
    {
      if ( pos_cmp.findIntersection( open_room_cmp ) ) add_interior_wall = false;
    }

    if ( add_interior_wall )
    {
      auto [obst_type, rand_obst_tex_idx] = sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_sb" } );
      float zorder = sprite_factory.get_sprite_size_by_type( "CRYPT.interior_sb" ).y;
      // Set the z-order value so that the obstacles are rendered above everything else
      Factory::createObstacle( reg, entity, pos_cmp, obst_type, 2, ( zorder * 2.f ) );
    }
  }
}

void gen_crypt_main_objective( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size )
{
  auto map_grid_sizef = sf::Vector2f( static_cast<float>( map_grid_size.x ) * Constants::kGridSizePxF.x,
                                      static_cast<float>( map_grid_size.y ) * Constants::kGridSizePxF.y );
  auto kGridSizePxF = Constants::kGridSizePxF;
  // target position for the objective: always center top of the map
  const auto &ms = sprite_factory.get_multisprite_by_type( "CRYPT.interior_objective_closed" );

  float centered_x = ( map_grid_sizef.x / 2.f ) - ( ms.getSpriteSizePixels().x / 2.f ) + kGridSizePxF.x;
  Cmp::Position objective_position( { centered_x, kGridSizePxF.y * 2.f }, ms.getSpriteSizePixels() );

  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, objective_position.position, objective_position.size );

  SPDLOG_INFO( "Placing main crypt objective at position ({}, {})", objective_position.position.x, objective_position.position.y );
  Factory::createMultiblock<Cmp::CryptObjectiveMultiBlock>( reg, entity, objective_position, ms );
  Factory::createMultiblockSegments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( reg, entity, objective_position, ms );

  // while we're here, carve out a room for the objective sprite. These position/size modifiers are trial and error
  // whilst we decide on the final objective MB sprite dimensions

  auto end_room_entity = reg.create();
  reg.emplace_or_replace<Cmp::CryptRoomEnd>( end_room_entity, sf::Vector2f{ objective_position.position.x, objective_position.position.y },
                                             sf::Vector2f{ objective_position.size.x, objective_position.size.y + ( kGridSizePxF.y * 2.f ) } );
}

} // namespace ProceduralMaze::Factory