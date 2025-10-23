#include <BombSystem.hpp>
#include <NpcDeathPosition.hpp>
#include <Persistent/ArmedOffDelay.hpp>
#include <Persistent/BombDamage.hpp>
#include <SpriteAnimation.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

BombSystem::BombSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&Sys::BombSystem::on_player_action>(
      this );
}

void BombSystem::suspend()
{
  auto player_collision_view = m_reg->view<Cmp::Armed>();
  for ( auto [_pc_entt, armed] : player_collision_view.each() )
  {
    if ( armed.m_fuse_delay_clock.isRunning() ) armed.m_fuse_delay_clock.stop();
    if ( armed.m_warning_delay_clock.isRunning() ) armed.m_warning_delay_clock.stop();
  }
}
void BombSystem::resume()
{
  auto player_collision_view = m_reg->view<Cmp::Armed>();
  for ( auto [_pc_entt, armed] : player_collision_view.each() )
  {
    if ( not armed.m_fuse_delay_clock.isRunning() ) armed.m_fuse_delay_clock.start();
    if ( not armed.m_warning_delay_clock.isRunning() ) armed.m_warning_delay_clock.start();
  }
}

void BombSystem::arm_occupied_location()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    if ( pc_cmp.has_active_bomb ) continue;     // skip if player already placed a bomb
    if ( pc_cmp.bomb_inventory == 0 ) continue; // skip if player has no bombs left, -1 is infini bombs

    auto obstacle_collision_view = m_reg->view<Cmp::Obstacle, Cmp::Position>( entt::exclude<typename Cmp::Armed> );
    for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos_cmp] : obstacle_collision_view.each() )
    {
      auto player_hitbox = get_hitbox( pc_pos_cmp );

      // reduce/center the player hitbox to avoid
      // arming a neighbouring location
      player_hitbox.size.x /= 2.f;
      player_hitbox.size.y /= 2.f;
      player_hitbox.position.x += 4.f;
      player_hitbox.position.y += 4.f;

      auto obstacle_hitbox = get_hitbox( obstacle_pos_cmp );

      // are we standing on this tile?
      if ( player_hitbox.findIntersection( obstacle_hitbox ) )
      {
        // has the bomb spamming cooldown expired?
        if ( pc_cmp.m_bombdeploycooldowntimer.getElapsedTime() >= pc_cmp.m_bombdeploydelay )
        {
          if ( m_fuse_sound_player.getStatus() != sf::Sound::Status::Playing ) m_fuse_sound_player.play();

          place_concentric_bomb_pattern( obstacle_entity, pc_cmp.blast_radius );

          pc_cmp.m_bombdeploycooldowntimer.restart();
          pc_cmp.has_active_bomb = true;
          pc_cmp.bomb_inventory = ( pc_cmp.bomb_inventory > 0 ) ? pc_cmp.bomb_inventory - 1 : pc_cmp.bomb_inventory;
        }
      }
    }
  }
}

void BombSystem::place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int blast_radius )
{
  // arm the current tile (center of the bomb)
  m_reg->emplace_or_replace<Cmp::Armed>( epicenter_entity, sf::seconds( 3 ), sf::Time::Zero, true, sf::Color::Blue,
                                         -1 );

  sf::Vector2i centerTile = getGridPosition( epicenter_entity ).value();

  int sequence_counter = 0;

  // First arm the center tile
  auto &fuse_delay = get_persistent_component<Cmp::Persistent::FuseDelay>();
  m_reg->emplace_or_replace<Cmp::Armed>( epicenter_entity, sf::seconds( fuse_delay() ), sf::Time::Zero, true,
                                         sf::Color::Transparent, sequence_counter++ );

  auto all_obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();

  // For each layer from 1 to BLAST_RADIUS
  for ( int layer = 1; layer <= blast_radius; layer++ )
  {
    std::vector<std::pair<entt::entity, sf::Vector2i>> layer_entities;

    // Collect all entities in this layer with their positions
    for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos_cmp] : all_obstacle_view.each() )
    {
      if ( obstacle_entity == epicenter_entity || m_reg->any_of<Cmp::Armed>( obstacle_entity ) ) continue;

      sf::Vector2i obstacleTile = getGridPosition( obstacle_entity ).value();
      int distanceFromCenter = getChebyshevDistance( obstacleTile, centerTile );

      if ( distanceFromCenter == layer ) { layer_entities.push_back( { obstacle_entity, obstacleTile } ); }
    }

    // Sort entities in clockwise order
    std::sort( layer_entities.begin(), layer_entities.end(), [centerTile]( const auto &a, const auto &b ) {
      // Calculate angles from center to points
      float angleA = std::atan2( a.second.y - centerTile.y, a.second.x - centerTile.x );
      float angleB = std::atan2( b.second.y - centerTile.y, b.second.x - centerTile.x );
      return angleA < angleB;
    } );

    // Arm each entity in the layer in clockwise order
    for ( const auto &[entity, pos] : layer_entities )
    {
      sf::Color color = sf::Color( 255, 10 + ( sequence_counter * 10 ) % 155, 255, 64 );
      auto &fuse_delay = get_persistent_component<Cmp::Persistent::FuseDelay>();
      auto &armed_on_delay = get_persistent_component<Cmp::Persistent::ArmedOnDelay>();
      auto &armed_off_delay = get_persistent_component<Cmp::Persistent::ArmedOffDelay>();
      auto new_fuse_delay = sf::seconds( fuse_delay() + ( sequence_counter * armed_on_delay() ) );
      auto new_warning_delay = sf::seconds( armed_off_delay() + ( sequence_counter * armed_off_delay() ) );
      m_reg->emplace_or_replace<Cmp::Armed>( entity, new_fuse_delay, new_warning_delay, false, color,
                                             sequence_counter );
      sequence_counter++;
    }
  }
}

void BombSystem::update()
{
  auto armed_view = m_reg->view<Cmp::Armed, Cmp::Obstacle, Cmp::Neighbours, Cmp::Position>();
  for ( auto [_entt, _armed_cmp, _obstacle_cmp, _neighbours_cmp, _ob_pos_comp] : armed_view.each() )
  {
    if ( _armed_cmp.getElapsedFuseTime() < _armed_cmp.m_fuse_delay ) continue;
    if ( _obstacle_cmp.m_enabled && _obstacle_cmp.m_integrity > 0.0f )
    {
      // the obstacle is now destroyed by the bomb
      _obstacle_cmp.m_integrity = 0.0f;
      _obstacle_cmp.m_enabled = false;

      // replace the broken pot neighbour entities with a random loot component/sprite
      if ( _obstacle_cmp.m_type == "POT" )
      {
        auto &sprite_factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
        auto [obstacle_type, random_obstacle_texture_index] = sprite_factory->get_random_type_and_texture_index(
            std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "LOWER_WATER" } );
        m_reg->emplace_or_replace<Cmp::Loot>( _entt, obstacle_type, random_obstacle_texture_index );
      }
    }

    // Check player explosion damage
    auto obstacle_explosion_zone = get_hitbox( _ob_pos_comp );
    auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
    for ( auto [player_entt, player, player_position] : player_view.each() )
    {
      auto player_bounding_box = get_hitbox( player_position );
      if ( player_bounding_box.findIntersection( obstacle_explosion_zone ) )
      {
        auto &bomb_damage = get_persistent_component<Cmp::Persistent::BombDamage>();
        player.health -= bomb_damage();
        if ( player.health <= 0 ) { player.alive = false; }
      }
      player.has_active_bomb = false;
    }

    // Check if NPC was killed by explosion
    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : m_reg->view<Cmp::NPC, Cmp::Position>().each() )
    {
      auto npc_bounding_box = get_hitbox( npc_pos_cmp );
      // notify npc system of death
      if ( npc_bounding_box.findIntersection( obstacle_explosion_zone ) )
      {
        m_reg->emplace_or_replace<Cmp::NpcDeathPosition>( npc_entt, npc_pos_cmp );
        m_reg->emplace_or_replace<Cmp::SpriteAnimation>( npc_entt );
        auto &npc_death_anim = m_reg->get<Cmp::SpriteAnimation>( npc_entt );
        npc_death_anim.m_current_frame = 0;
        SPDLOG_INFO( "NPC entity {} exploded at {},{}", static_cast<int>( npc_entt ), npc_pos_cmp.x, npc_pos_cmp.y );
        getEventDispatcher().trigger( Events::NpcDeathEvent( npc_entt ) );
      }
    }

    // if we got this far then the bomb detonated, we can destroy the armed component
    m_reg->erase<Cmp::Armed>( _entt );

    if ( m_fuse_sound_player.getStatus() == sf::Sound::Status::Playing ) m_fuse_sound_player.stop();
    if ( m_detonate_sound_player.getStatus() != sf::Sound::Status::Playing ) m_detonate_sound_player.play();
  }
}

} // namespace ProceduralMaze::Sys