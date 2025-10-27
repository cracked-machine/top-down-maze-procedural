#include <FloodSystem.hpp>

namespace ProceduralMaze::Sys {

FloodSystem::FloodSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window )
    : BaseSystem( reg, window )
{
}

void FloodSystem::add_flood_water_entity()
{
  SPDLOG_INFO( "Creating flood water entity" );
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::WaterLevel>( entity,
                                   kMapGridSize.y - 1 // initial level
  );
}

void FloodSystem::update()
{
  float frameTime = m_clock.restart().asSeconds();

  // Clamp frame time to prevent spiral of death
  frameTime = std::min( frameTime, 0.25f );

  m_accumulator += frameTime;

  // Process fixed timesteps
  while ( m_accumulator >= FIXED_TIMESTEP )
  {
    updateFlood( FIXED_TIMESTEP );
    m_accumulator -= FIXED_TIMESTEP;
  }
}

void FloodSystem::updateFlood( float dt )
{
  static float total_time = 0.0f;
  total_time += dt;

  // Cache views once - better performance since entities always exist
  auto water_view = m_reg->view<Cmp::WaterLevel>();
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction>();

  // abort if flood is paused
  for ( auto [_, sys] : m_reg->view<Cmp::System>().each() )
  {
    if ( not sys.pause_flood )
    {
      // Separate water level updates from collision checks for better
      // performance
      for ( auto [_, water_level] : water_view.each() )
      {
        auto &flood_speed = get_persistent_component<Cmp::Persistent::FloodSpeed>();
        water_level.m_level -= ( dt * flood_speed.get_value() );
      }
    }
  }

  // Check drowning - {0,0} is top-left so player drowns when water level is
  // BELOW player position
  for ( auto [_, water_level] : water_view.each() )
  {
    for ( auto [player_entity, pc_cmp, pos_cmp, dir_cmp] : player_view.each() )
    {
      if ( water_level.m_level <= pos_cmp.position.y ) // Water drowns player when water level is at or
                                                       // above player position
      {
        // Other systems will need to know which acceleration/deceleration/maxspeed to use
        pc_cmp.underwater = true;

        // Check if enough time has passed since last damage
        auto it = m_last_damage_time.find( player_entity );
        if ( it == m_last_damage_time.end() || ( total_time - it->second ) >= DAMAGE_COOLDOWN )
        {
          pc_cmp.health -= 5;
          SPDLOG_TRACE( "player health {}", player_char.health );
          m_last_damage_time[player_entity] = total_time;

          if ( pc_cmp.health <= 0 )
          {
            pc_cmp.alive = false;
            SPDLOG_TRACE( "Player has drowned!" );
          }
        }
      }
      else
      {
        // Player is out of water, remove from damage tracking
        m_last_damage_time.erase( player_entity );

        // Other systems will need to know which acceleration/deceleration/maxspeed to use
        pc_cmp.underwater = false;
      }
    }
  }
}
} // namespace ProceduralMaze::Sys