#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/RectBounds.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/Destructable.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/NpcDeathPosition.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ShrineSegment.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/LootContainerDestroyedEvent.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys
{

BombSystem::BombSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                        Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&Sys::BombSystem::on_player_action>( this );
  std::ignore = get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::BombSystem::onPause>( this );
  std::ignore = get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::BombSystem::onResume>( this );
  SPDLOG_DEBUG( "BombSystem initialized" );
}

void BombSystem::onPause()
{
  if ( m_sound_bank.get_effect( "bomb_fuse" ).getStatus() == sf::Sound::Status::Playing )
    m_sound_bank.get_effect( "bomb_fuse" ).pause();
  if ( m_sound_bank.get_effect( "bomb_detonate" ).getStatus() == sf::Sound::Status::Playing )
    m_sound_bank.get_effect( "bomb_detonate" ).pause();
  auto player_collision_view = getReg().view<Cmp::Armed>();
  for ( auto [_pc_entt, armed] : player_collision_view.each() )
  {
    if ( armed.m_fuse_delay_clock.isRunning() ) armed.m_fuse_delay_clock.stop();
    if ( armed.m_warning_delay_clock.isRunning() ) armed.m_warning_delay_clock.stop();
  }
}
void BombSystem::onResume()
{
  if ( m_sound_bank.get_effect( "bomb_fuse" ).getStatus() == sf::Sound::Status::Paused )
    m_sound_bank.get_effect( "bomb_fuse" ).play();
  if ( m_sound_bank.get_effect( "bomb_detonate" ).getStatus() == sf::Sound::Status::Paused )
    m_sound_bank.get_effect( "bomb_detonate" ).play();
  auto player_collision_view = getReg().view<Cmp::Armed>();
  for ( auto [_pc_entt, armed] : player_collision_view.each() )
  {
    if ( not armed.m_fuse_delay_clock.isRunning() ) armed.m_fuse_delay_clock.start();
    if ( not armed.m_warning_delay_clock.isRunning() ) armed.m_warning_delay_clock.start();
  }
}

void BombSystem::arm_occupied_location( [[maybe_unused]] const Events::PlayerActionEvent &event )
{
  auto player_collision_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( const auto [pc_entity, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    if ( event.action != Events::PlayerActionEvent::GameActions::GRAVE_BOMB && pc_cmp.has_active_bomb )
      continue; // skip if player already placed a bomb
    if ( event.action != Events::PlayerActionEvent::GameActions::GRAVE_BOMB && pc_cmp.bomb_inventory == 0 )
      continue; // skip if player has no bombs left, -1 is infini bombs

    // for booby trapped graves, first try to find a random nearby disabled destructable obstacle for candidate bomb epicenter
    entt::entity candidate_entity = entt::null;
    if ( event.action == Events::PlayerActionEvent::GameActions::GRAVE_BOMB )
    {
      auto search_area = Cmp::RectBounds( pc_pos_cmp.position, BaseSystem::kGridSquareSizePixelsF, 3.f );
      candidate_entity = get_random_nearby_disabled_obstacle( search_area.getBounds(), IncludePack<Cmp::Destructable>{},
                                                              ExcludePack<>{} );
      auto pos_cmp = getReg().try_get<Cmp::Position>( candidate_entity );
      if ( pos_cmp )
        SPDLOG_INFO( "Returned candidate entity: {}, pos: {},{}", static_cast<uint32_t>( candidate_entity ), pos_cmp->position.x,
                     pos_cmp->position.y );
    }
    // then use the candidate entity to place the booby trap bomb
    if ( candidate_entity != entt::null )
    {
      m_sound_bank.get_effect( "bomb_fuse" ).play();
      place_concentric_bomb_pattern( candidate_entity, pc_cmp.blast_radius );
    }
    // fallback to the normal bomb placement at player's current location
    else
    {
      auto destructable_view = getReg().view<Cmp::Destructable, Cmp::Position>(
          entt::exclude<typename Cmp::Armed, Cmp::ShrineSegment, Cmp::NPC> );
      for ( auto [destructable_entity, destructable_cmp, destructable_pos_cmp] : destructable_view.each() )
      {
        // make a copy and reduce/center the player hitbox to avoid arming a neighbouring location
        auto player_hitbox = sf::FloatRect( pc_pos_cmp );
        player_hitbox.size.x /= 2.f;
        player_hitbox.size.y /= 2.f;
        player_hitbox.position.x += 4.f;
        player_hitbox.position.y += 4.f;

        // are we standing on a destructable tile?
        if ( player_hitbox.findIntersection( destructable_pos_cmp ) )
        {

          SPDLOG_INFO( "Checking cooldown timer for bomb placement." );
          if ( pc_cmp.m_bombdeploycooldowntimer.getElapsedTime() < pc_cmp.m_bombdeploydelay ) continue;

          m_sound_bank.get_effect( "bomb_fuse" ).play();

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
  getReg().emplace_or_replace<Cmp::Armed>( epicenter_entity, sf::seconds( fuse_delay.get_value() ), sf::Time::Zero, true,
                                           sf::Color::Transparent, sequence_counter++, Cmp::Armed::EpiCenter::YES );

  // We dont detonate ReservedPositions so dont arm them in the first place
  // Also exclude NPCs since they're handled separately and may be missing Position component during death animation
  auto all_obstacle_view = getReg().view<Cmp::Destructable, Cmp::Position>(
      entt::exclude<Cmp::ShrineSegment, Cmp::GraveSegment, Cmp::NPC> );

  // For each layer from 1 to BLAST_RADIUS
  for ( int layer = 1; layer <= blast_radius; layer++ )
  {
    std::vector<std::pair<entt::entity, sf::Vector2i>> layer_entities;

    // Collect all entities in this layer with their positions
    for ( auto [destructable_entity, destructable_cmp, destructable_pos] : all_obstacle_view.each() )
    {
      if ( destructable_entity == epicenter_entity || getReg().any_of<Cmp::Armed>( destructable_entity ) ) continue;

      sf::Vector2i grid_position = getGridPosition( destructable_entity ).value();
      int distance_from_center = getChebyshevDistance( grid_position, centerTile );

      if ( distance_from_center == layer )
      {
        if ( getReg().any_of<Cmp::LootContainer>( destructable_entity ) )
        {
          SPDLOG_DEBUG( "Arming loot container entity {}", static_cast<int>( destructable_entity ) );
        }
        layer_entities.push_back( { destructable_entity, grid_position } );
      }
    }
    SPDLOG_DEBUG( "Layer {}: Found {} entities to arm", layer, layer_entities.size() );

    // Sort entities in clockwise order
    std::sort( layer_entities.begin(), layer_entities.end(),
               [centerTile]( const auto &a, const auto &b )
               {
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
      auto new_warning_delay = sf::seconds( armed_off_delay.get_value() + ( sequence_counter * armed_off_delay.get_value() ) );
      getReg().emplace_or_replace<Cmp::Armed>( entity, new_fuse_delay, new_warning_delay, false, color, sequence_counter );
      sequence_counter++;
    }
  }
}

void BombSystem::update()
{
  auto armed_view = getReg().view<Cmp::Armed, Cmp::Position>();
  for ( auto [armed_entt, armed_cmp, armed_pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.getElapsedFuseTime() < armed_cmp.m_fuse_delay ) continue;

    // detonate obstacles
    auto obst_cmp = getReg().try_get<Cmp::Obstacle>( armed_entt );
    if ( obst_cmp && obst_cmp->m_enabled && obst_cmp->m_integrity > 0.0f )
    {
      // the obstacle is now destroyed by the bomb
      obst_cmp->m_integrity = 0.0f;
      obst_cmp->m_enabled = false;
    }

    // detonate loot containers
    auto lootcontainer_cmp = getReg().try_get<Cmp::LootContainer>( armed_entt );
    if ( lootcontainer_cmp )
    {
      SPDLOG_INFO( "Triggering LootContainerDestroyedEvent for entity {}  ", static_cast<int>( armed_entt ) );
      get_systems_event_queue().trigger( Events::LootContainerDestroyedEvent( armed_entt ) );
    }

    // detonate npc containers
    auto npc_container_cmp = getReg().try_get<Cmp::NpcContainer>( armed_entt );
    if ( npc_container_cmp )
    {
      // assuming we could get close enough without the NPC spawning, the NPC container is now
      // destroyed by the bomb
      auto [npc_type, random_npc_texture_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "NPC_TYPE_1", "NPC_TYPE_2", "NPC_TYPE_3" } );
      getReg().remove<Cmp::NpcContainer>( armed_entt );
    }

    // Check player explosion damage
    auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position>();
    for ( auto [pc_entt, pc_cmp, pc_health_cmp, pc_mort_cmp, pc_pos_cmp] : player_view.each() )
    {
      if ( pc_pos_cmp.findIntersection( armed_pos_cmp ) )
      {
        auto &bomb_damage = get_persistent_component<Cmp::Persistent::BombDamage>();
        pc_health_cmp.health -= bomb_damage.get_value();
        if ( pc_health_cmp.health <= 0 ) { pc_mort_cmp.state = Cmp::PlayerMortality::State::EXPLODING; }
      }
      pc_cmp.has_active_bomb = false;
    }

    // Check if NPC was killed by explosion
    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : getReg().view<Cmp::NPC, Cmp::Position>().each() )
    {
      // notify npc system of death
      if ( npc_pos_cmp.findIntersection( armed_pos_cmp ) )
      {
        getReg().emplace_or_replace<Cmp::NpcDeathPosition>( npc_entt, npc_pos_cmp.position, npc_pos_cmp.size );
        getReg().emplace_or_replace<Cmp::SpriteAnimation>( npc_entt, 0, 0, true, "EXPLOSION", 0 );
        SPDLOG_INFO( "NPC entity {} exploded at {},{}", static_cast<int>( npc_entt ), npc_pos_cmp.position.x,
                     npc_pos_cmp.position.y );
        get_systems_event_queue().trigger( Events::NpcDeathEvent( npc_entt ) );
      }
    }

    // play sound effect if this armed component is epicenter
    if ( armed_cmp.m_epicenter == Cmp::Armed::EpiCenter::YES ) { m_sound_bank.get_effect( "bomb_detonate" ).play(); }

    // check if we have any epicenter armed components before stopping the fuse sound
    bool remaining_epicenter_bombs = false;
    for ( auto [armed_entity, armed_cmp] : getReg().view<Cmp::Armed>().each() )
    {
      if ( armed_cmp.m_epicenter == Cmp::Armed::EpiCenter::YES )
      {
        remaining_epicenter_bombs = true;
        break; // we dont care how many
      }
    }
    if ( not remaining_epicenter_bombs ) m_sound_bank.get_effect( "bomb_fuse" ).stop();

    // finally delete the armed component
    getReg().remove<Cmp::Armed>( armed_entt );
  }
}

} // namespace ProceduralMaze::Sys