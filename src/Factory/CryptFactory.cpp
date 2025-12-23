#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Factory/CryptFactory.hpp>

namespace ProceduralMaze::Factory
{

void create_initial_crypt_rooms( entt::registry &reg, sf::Vector2u map_grid_size )
{
  const auto &grid_square_size = Constants::kGridSquareSizePixelsF;
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

} // namespace ProceduralMaze::Factory