#include <Components/Persistent/EffectsVolume.hpp>
#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/Destructable.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/NpcDeathPosition.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/LootContainerDestroyedEvent.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys {

BombSystem::BombSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                        Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&Sys::BombSystem::on_player_action>(
      this );
  SPDLOG_DEBUG( "BombSystem initialized" );
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
  for ( const auto [pc_entity, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    if ( pc_cmp.has_active_bomb ) continue;     // skip if player already placed a bomb
    if ( pc_cmp.bomb_inventory == 0 ) continue; // skip if player has no bombs left, -1 is infini bombs

    auto destructable_view = m_reg->view<Cmp::Destructable, Cmp::Position>(
        entt::exclude<typename Cmp::Armed, Cmp::ShrineSprite, Cmp::NPC> );
    for ( auto [destructable_entity, destructable_cmp, destructable_pos_cmp] : destructable_view.each() )
    {
      // make a copy and reduce/center the player hitbox to avoid arming a neighbouring location
      auto player_hitbox = sf::FloatRect( pc_pos_cmp );
      player_hitbox.size.x /= 2.f;
      player_hitbox.size.y /= 2.f;
      player_hitbox.position.x += 4.f;
      player_hitbox.position.y += 4.f;

      // are we standing on this tile?
      if ( player_hitbox.findIntersection( destructable_pos_cmp ) )
      {
        // has the bomb spamming cooldown expired?
        if ( pc_cmp.m_bombdeploycooldowntimer.getElapsedTime() >= pc_cmp.m_bombdeploydelay )
        {
          auto &bomb_fuse_player = m_sound_bank.get_effect( "bomb_fuse" );
          if ( bomb_fuse_player.getStatus() != sf::Sound::Status::Playing ) bomb_fuse_player.play();

          place_concentric_bomb_pattern( destructable_entity, pc_cmp.blast_radius );

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
  sf::Vector2i centerTile = getGridPosition( epicenter_entity ).value();

  int sequence_counter = 0;

  // First arm the center tile
  auto &fuse_delay = get_persistent_component<Cmp::Persistent::FuseDelay>();
  m_reg->emplace_or_replace<Cmp::Armed>( epicenter_entity, sf::seconds( fuse_delay.get_value() ), sf::Time::Zero, true,
                                         sf::Color::Transparent, sequence_counter++ );

  // We dont detonate ReservedPositions so dont arm them in the first place
  // Also exclude NPCs since they're handled separately and may be missing Position component during death animation
  auto all_obstacle_view = m_reg->view<Cmp::Destructable, Cmp::Position>(
      entt::exclude<Cmp::ShrineSprite, Cmp::GraveSprite, Cmp::NPC> );

  // For each layer from 1 to BLAST_RADIUS
  for ( int layer = 1; layer <= blast_radius; layer++ )
  {
    std::vector<std::pair<entt::entity, sf::Vector2i>> layer_entities;

    // Collect all entities in this layer with their positions
    for ( auto [destructable_entity, destructable_cmp, destructable_pos] : all_obstacle_view.each() )
    {
      if ( destructable_entity == epicenter_entity || m_reg->any_of<Cmp::Armed>( destructable_entity ) ) continue;

      sf::Vector2i grid_position = getGridPosition( destructable_entity ).value();
      int distance_from_center = getChebyshevDistance( grid_position, centerTile );

      if ( distance_from_center == layer )
      {
        if ( m_reg->any_of<Cmp::LootContainer>( destructable_entity ) )
        {
          SPDLOG_DEBUG( "Arming loot container entity {}", static_cast<int>( destructable_entity ) );
        }
        layer_entities.push_back( { destructable_entity, grid_position } );
      }
    }
    SPDLOG_DEBUG( "Layer {}: Found {} entities to arm", layer, layer_entities.size() );

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
      auto new_fuse_delay = sf::seconds( fuse_delay.get_value() + ( sequence_counter * armed_on_delay.get_value() ) );
      auto new_warning_delay = sf::seconds( armed_off_delay.get_value() +
                                            ( sequence_counter * armed_off_delay.get_value() ) );
      m_reg->emplace_or_replace<Cmp::Armed>( entity, new_fuse_delay, new_warning_delay, false, color,
                                             sequence_counter );
      sequence_counter++;
    }
  }
}

void BombSystem::update()
{
  auto armed_view = m_reg->view<Cmp::Armed, Cmp::Position>();
  for ( auto [armed_entt, armed_cmp, armed_pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.getElapsedFuseTime() < armed_cmp.m_fuse_delay ) continue;

    // detonate obstacles
    auto obst_cmp = m_reg->try_get<Cmp::Obstacle>( armed_entt );
    if ( obst_cmp && obst_cmp->m_enabled && obst_cmp->m_integrity > 0.0f )
    {
      // the obstacle is now destroyed by the bomb
      obst_cmp->m_integrity = 0.0f;
      obst_cmp->m_enabled = false;
    }

    // detonate loot containers
    auto lootcontainer_cmp = m_reg->try_get<Cmp::LootContainer>( armed_entt );
    if ( lootcontainer_cmp )
    {
      SPDLOG_INFO( "Triggering LootContainerDestroyedEvent for entity {}  ", static_cast<int>( armed_entt ) );
      getEventDispatcher().trigger( Events::LootContainerDestroyedEvent( armed_entt ) );
      // // the loot container is now destroyed by the bomb, replace with a random loot component
      // auto [obstacle_type, random_obstacle_texture_index] = m_sprite_factory.get_random_type_and_texture_index(
      //     std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );
      // m_reg->remove<Cmp::LootContainer>( armed_entt );
      // m_reg->remove<Cmp::ReservedPosition>( armed_entt );
      // m_reg->emplace_or_replace<Cmp::Loot>( armed_entt, obstacle_type, random_obstacle_texture_index );
    }

    // detonate npc containers
    auto npc_container_cmp = m_reg->try_get<Cmp::NpcContainer>( armed_entt );
    if ( npc_container_cmp )
    {
      // assuming we could get close enough without the NPC spawning, the NPC container is now
      // destroyed by the bomb
      auto [npc_type, random_npc_texture_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "NPC_TYPE_1", "NPC_TYPE_2", "NPC_TYPE_3" } );
      m_reg->remove<Cmp::NpcContainer>( armed_entt );
    }

    // Check player explosion damage
    auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
    for ( auto [player_entt, player, player_position] : player_view.each() )
    {
      if ( player_position.findIntersection( armed_pos_cmp ) )
      {
        auto &bomb_damage = get_persistent_component<Cmp::Persistent::BombDamage>();
        player.health -= bomb_damage.get_value();
        if ( player.health <= 0 ) { player.alive = false; }
      }
      player.has_active_bomb = false;
    }

    // Check if NPC was killed by explosion
    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : m_reg->view<Cmp::NPC, Cmp::Position>().each() )
    {
      // notify npc system of death
      if ( npc_pos_cmp.findIntersection( armed_pos_cmp ) )
      {
        m_reg->emplace_or_replace<Cmp::NpcDeathPosition>( npc_entt, npc_pos_cmp.position );
        m_reg->emplace_or_replace<Cmp::SpriteAnimation>( npc_entt );
        auto &npc_death_anim = m_reg->get<Cmp::SpriteAnimation>( npc_entt );
        npc_death_anim.m_current_frame = 0;
        SPDLOG_INFO( "NPC entity {} exploded at {},{}", static_cast<int>( npc_entt ), npc_pos_cmp.position.x,
                     npc_pos_cmp.position.y );
        getEventDispatcher().trigger( Events::NpcDeathEvent( npc_entt ) );
      }
    }

    // if we got this far then the bomb detonated, we can remove the armed component
    m_reg->remove<Cmp::Armed>( armed_entt );
    auto &bomb_fuse_player = m_sound_bank.get_effect( "bomb_fuse" );
    if ( bomb_fuse_player.getStatus() == sf::Sound::Status::Playing ) bomb_fuse_player.stop();
    // dont play bomb detonate sound multiple times for concentric bombs
    auto &bomb_detonate_player = m_sound_bank.get_effect( "bomb_detonate" );
    if ( bomb_detonate_player.getStatus() != sf::Sound::Status::Playing ) bomb_detonate_player.play();
  }
}

} // namespace ProceduralMaze::Sys