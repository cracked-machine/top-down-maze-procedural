#include <Components/CryptSegment.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Exit.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/NPC.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <optional>
#include <spdlog/spdlog.h>

#include <Audio/SoundBank.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/Armed.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys
{
using entt::exclude;

BombSystem::BombSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&Sys::BombSystem::on_player_action>( this );
  std::ignore = get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::BombSystem::onPause>( this );
  std::ignore = get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::BombSystem::onResume>( this );
  SPDLOG_DEBUG( "BombSystem initialized" );
}

void BombSystem::onPause()
{
  if ( m_sound_bank.get_effect( "bomb_fuse" ).getStatus() == sf::Sound::Status::Playing ) m_sound_bank.get_effect( "bomb_fuse" ).pause();
  if ( m_sound_bank.get_effect( "bomb_detonate" ).getStatus() == sf::Sound::Status::Playing ) m_sound_bank.get_effect( "bomb_detonate" ).pause();
  auto armed_view = getReg().view<Cmp::Armed>();
  for ( auto [entt, armed_cmp] : armed_view.each() )
  {
    if ( armed_cmp.m_fuse_delay_clock.isRunning() ) armed_cmp.m_fuse_delay_clock.stop();
    if ( armed_cmp.m_warning_delay_clock.isRunning() ) armed_cmp.m_warning_delay_clock.stop();
  }
}
void BombSystem::onResume()
{
  if ( m_sound_bank.get_effect( "bomb_fuse" ).getStatus() == sf::Sound::Status::Paused ) m_sound_bank.get_effect( "bomb_fuse" ).play();
  if ( m_sound_bank.get_effect( "bomb_detonate" ).getStatus() == sf::Sound::Status::Paused ) m_sound_bank.get_effect( "bomb_detonate" ).play();
  auto armed_view = getReg().view<Cmp::Armed>();
  for ( auto [entt, armed_cmp] : armed_view.each() )
  {
    if ( not armed_cmp.m_fuse_delay_clock.isRunning() ) armed_cmp.m_fuse_delay_clock.start();
    if ( not armed_cmp.m_warning_delay_clock.isRunning() ) armed_cmp.m_warning_delay_clock.start();
  }
}

void BombSystem::arm_occupied_location( [[maybe_unused]] const Events::PlayerActionEvent &event )
{

  auto [inventory_entt, inventory_type] = Utils::get_player_inventory_type( getReg() );
  if ( event.action == Events::PlayerActionEvent::GameActions::DROP_BOMB )
  {
    if ( inventory_type != "CARRYITEM.bomb" ) return;
  }

  auto player_collision_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( const auto [pc_entity, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    // for booby trapped graves, first try to find a random nearby disabled destructable obstacle for candidate bomb epicenter
    entt::entity candidate_entity = entt::null;
    if ( event.action == Events::PlayerActionEvent::GameActions::GRAVE_BOMB )
    {
      auto search_area = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 3.f );
      candidate_entity = Utils::Rnd::get_random_nearby_disabled_obstacle( getReg(), search_area.getBounds(), Utils::Rnd::IncludePack<Cmp::Armable>{},
                                                                          Utils::Rnd::ExcludePack<Cmp::Exit>{} );
      auto pos_cmp = getReg().try_get<Cmp::Position>( candidate_entity );
      if ( pos_cmp )
        SPDLOG_DEBUG( "Returned candidate entity: {}, pos: {},{}", static_cast<uint32_t>( candidate_entity ), pos_cmp->position.x,
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
      auto destructable_view = getReg().view<Cmp::Armable, Cmp::Position>();
      for ( auto [destructable_entity, destructable_cmp, destructable_pos_cmp] : destructable_view.each() )
      {

        auto [inventory_entt, inventory_type] = Utils::get_player_inventory_type( getReg() );
        if ( inventory_type != "CARRYITEM.bomb" ) return;

        // make a copy and reduce/center the player hitbox to avoid arming a neighbouring location
        auto player_hitbox = sf::FloatRect( pc_pos_cmp );
        player_hitbox.size.x /= 2.f;
        player_hitbox.size.y /= 2.f;
        player_hitbox.position.x += 4.f;
        player_hitbox.position.y += 4.f;

        // are we standing on a destructable tile?
        if ( player_hitbox.findIntersection( destructable_pos_cmp ) )
        {
          m_sound_bank.get_effect( "bomb_fuse" ).play();

          auto armed_epicenter_entity = getReg().create();
          getReg().emplace<Cmp::Position>( armed_epicenter_entity, destructable_pos_cmp.position, destructable_pos_cmp.size );
          place_concentric_bomb_pattern( armed_epicenter_entity, pc_cmp.blast_radius );
          Factory::destroyInventory( getReg(), "CARRYITEM.bomb" );
          // remove the used bomb carry item from the player inventory - Factory::createArmed drops a new bomb
        }
      }
    }
  }
}

void BombSystem::place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int blast_radius )
{
  sf::Vector2i centerTile = Utils::getGridPosition<int>( getReg(), epicenter_entity ).value();
  int sequence_counter = 0;
  Factory::createArmed( getReg(), epicenter_entity, Cmp::Armed::EpiCenter::YES, sequence_counter++, centerTile.y - 64 );

  // We dont detonate ReservedPositions so dont arm them in the first place
  // Also exclude NPCs since they're handled separately and may be missing Position component during
  // death animation
  auto all_obstacle_view = getReg().view<Cmp::Armable, Cmp::Position>( exclude<Cmp::NPC, Cmp::Exit> );

  // For each layer from 1 to BLAST_RADIUS
  for ( int layer = 1; layer <= blast_radius; layer++ )
  {
    std::vector<std::pair<entt::entity, sf::Vector2i>> layer_entities;

    // Collect all entities in this layer with their positions
    for ( auto [destructable_entity, destructable_cmp, destructable_pos] : all_obstacle_view.each() )
    {
      if ( destructable_entity == epicenter_entity || getReg().any_of<Cmp::Armed>( destructable_entity ) ) continue;

      sf::Vector2i grid_position = Utils::getGridPosition<int>( getReg(), destructable_entity ).value();
      int distance_from_center = Utils::Maths::getChebyshevDistance( grid_position, centerTile );

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
      Factory::createArmed( getReg(), entity, Cmp::Armed::EpiCenter::NO, sequence_counter++, centerTile.y - 64 );
    }
  }
}

void BombSystem::update()
{
  auto armed_view = getReg().view<Cmp::Armed, Cmp::Position>();
  for ( auto [armed_entt, armed_cmp, armed_pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.getElapsedFuseTime() < armed_cmp.m_fuse_delay ) continue;

    // detonate obstacles - remove all traces of obstacle
    auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [obst_entity, obst_cmp, obst_pos_cmp] : obstacle_view.each() )
    {
      if ( not obst_pos_cmp.findIntersection( armed_pos_cmp ) ) continue;
      Factory::destroyObstacle( getReg(), obst_entity );
    }

    // detonate loot containers - component removal is handled by LootSystem
    auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position>();
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp] : loot_container_view.each() )
    {
      if ( not loot_pos_cmp.findIntersection( armed_pos_cmp ) ) continue;

      if ( loot_entt != entt::null ) { m_sound_bank.get_effect( "break_pot" ).play(); }

      Factory::destroyLootContainer( getReg(), loot_entt );
    }

    // detonate npc containers - these are activated by proximity so just destroy them
    auto npc_container_view = getReg().view<Cmp::NpcContainer, Cmp::Position>();
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_container_view.each() )
    {
      if ( not npc_pos_cmp.findIntersection( armed_pos_cmp ) ) continue;
      Factory::destroyNpcContainer( getReg(), npc_entity );
    }

    // detonate nearby carryitems - cruel but fair
    auto carryitem_view = getReg().view<Cmp::CarryItem, Cmp::Position>();
    for ( auto [carryitem_entity, carryitem_cmp, carryitem_pos_cmp] : carryitem_view.each() )
    {
      if ( not carryitem_pos_cmp.findIntersection( armed_pos_cmp ) ) continue;
      if ( carryitem_cmp.type == "CARRYITEM.pickaxe" or carryitem_cmp.type == "CARRYITEM.axe" or carryitem_cmp.type == "CARRYITEM.shovel" )
      {
        Utils::reduce_player_inventory_wear_level( getReg(), Sys::PersistSystem::get_persist_cmp<Cmp::Persist::BombDamage>( getReg() ).get_value() );
      }
      else
      {
        if ( getReg().valid( carryitem_entity ) ) { getReg().destroy( carryitem_entity ); }
      }
    }

    // Check player explosion damage
    auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position>();
    for ( auto [pc_entt, pc_cmp, pc_health_cmp, pc_mort_cmp, pc_pos_cmp] : player_view.each() )
    {
      if ( pc_pos_cmp.findIntersection( armed_pos_cmp ) )
      {
        auto &bomb_damage = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::BombDamage>( getReg() );
        pc_health_cmp.health -= bomb_damage.get_value();
        if ( pc_health_cmp.health <= 0 )
        {
          get_systems_event_queue().enqueue(
              Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::EXPLODING, Utils::get_player_position( getReg() ) ) );
        }
      }
    }

    // Check if NPC was killed by explosion
    for ( auto [npc_entt, npc_cmp, npc_pos_cmp, npc_anim_cmp] : getReg().view<Cmp::NPC, Cmp::Position, Cmp::SpriteAnimation>().each() )
    {
      if ( npc_anim_cmp.m_sprite_type.contains( "NPCGHOST" ) ) continue;
      // notify npc system of death
      if ( npc_pos_cmp.findIntersection( armed_pos_cmp ) )
      {
        Factory::createNpcExplosion( getReg(), npc_pos_cmp );

        SPDLOG_INFO( "NPC entity {} exploded at {},{}", static_cast<int>( npc_entt ), npc_pos_cmp.position.x, npc_pos_cmp.position.y );
        Factory::destroyNPC( getReg(), npc_entt );

        auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
            std::vector<std::string>{ "EXTRA_HEALTH", "CHAIN_BOMBS", "WEAPON_BOOST" } );

        Cmp::RandomInt do_drop( 0, 2 ); // 1 in 3 chance of no drop
        if ( do_drop.gen() == 0 )
        {
          // clang-format off
          auto dropped_loot_entt = Factory::createLootDrop( 
            getReg(), 
            Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),                                        
            sf::FloatRect{ npc_pos_cmp.position, npc_pos_cmp.size }, 
            Factory::IncludePack<>{},
            Factory::ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition>{} );
          // clang-format on

          if ( dropped_loot_entt != entt::null )
          {
            SPDLOG_INFO( "Dropped RELIC_DROP loot at NPC death position." );
            m_sound_bank.get_effect( "drop_relic" ).play();
          }
        }
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
    Factory::destroyArmed( getReg(), armed_entt );

    // Replace the armed position with a detonated sprite for visual effect - make sure its z-order
    // is furthest back
    Factory::createDetonated( getReg(), armed_entt, armed_pos_cmp );
  }
}

} // namespace ProceduralMaze::Sys