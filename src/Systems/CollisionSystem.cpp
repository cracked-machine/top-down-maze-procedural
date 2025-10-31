#include <Components/WeaponLevel.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Window.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/Destructable.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/LargeObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDamageDelay.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/ShrineCost.hpp>
#include <Components/Persistent/WaterBonus.hpp>
#include <Components/PlayerScore.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

CollisionSystem::CollisionSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                                  Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( reg, window, sprite_factory )
{
  // register the event sinks
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&CollisionSystem::on_player_action>( this );
  SPDLOG_DEBUG( "CollisionSystem initialized" );
}

void CollisionSystem::suspend()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
  for ( auto [_pc_entt, player] : player_collision_view.each() )
  {
    if ( player.m_bombdeploycooldowntimer.isRunning() ) player.m_bombdeploycooldowntimer.stop();
  }
}
void CollisionSystem::resume()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
  for ( auto [_pc_entt, player] : player_collision_view.each() )
  {
    if ( not player.m_bombdeploycooldowntimer.isRunning() ) player.m_bombdeploycooldowntimer.start();
  }
}

void CollisionSystem::check_bones_reanimation()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  auto npccontainer_collision_view = m_reg->view<Cmp::NpcContainer, Cmp::Position>();
  for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    for ( auto [npccontainer_entt, npccontainer_cmp, npccontainer_pos_cmp] : npccontainer_collision_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getGameView(), npccontainer_pos_cmp ) ) continue;

      auto &npc_activate_scale = get_persistent_component<Cmp::Persistent::NpcActivateScale>();
      // we just create a temporary RectBounds here instead of a component because we only need it for
      // this one comparison and it already contains the needed scaling logic
      auto npc_activate_bounds = Cmp::RectBounds( npccontainer_pos_cmp.position, kGridSquareSizePixelsF,
                                                  npc_activate_scale.get_value() );

      if ( pc_pos_cmp.findIntersection( npc_activate_bounds.getBounds() ) )
      {
        m_reg->emplace_or_replace<Cmp::Obstacle>( npccontainer_entt, "BONES", 0, false );
        getEventDispatcher().trigger( Events::NpcCreationEvent( npccontainer_entt, "NPCSKELE" ) );
      }
    }
  }
}

void CollisionSystem::check_player_to_npc_collision()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction>();
  auto npc_collision_view = m_reg->view<Cmp::NPC, Cmp::Position>();
  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, dir_cmp] : player_collision_view.each() )
  {
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_collision_view.each() )
    {
      if ( pc_pos_cmp.findIntersection( npc_pos_cmp ) )
      {
        auto &npc_damage_cooldown = get_persistent_component<Cmp::Persistent::NpcDamageDelay>();
        if ( npc_cmp.m_damage_cooldown.getElapsedTime().asSeconds() < npc_damage_cooldown.get_value() ) continue;

        auto &npc_damage = get_persistent_component<Cmp::Persistent::NpcDamage>();
        pc_cmp.health -= npc_damage.get_value();

        npc_cmp.m_damage_cooldown.restart();

        auto &npc_push_back = get_persistent_component<Cmp::Persistent::NpcPushBack>();

        // Find a valid pushback position by checking all 8 directions
        sf::Vector2f target_push_back_pos = findValidPushbackPosition( pc_pos_cmp.position, npc_pos_cmp.position, dir_cmp,
                                                                       npc_push_back.get_value() );

        // Update player position if we found a valid pushback position
        if ( target_push_back_pos != pc_pos_cmp.position ) { pc_pos_cmp.position = target_push_back_pos; }
      }

      if ( pc_cmp.health <= 0 )
      {
        pc_cmp.alive = false;
        return;
      }
    }
  }
}

sf::Vector2f CollisionSystem::findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos,
                                                         const sf::Vector2f &player_direction, float pushback_distance )
{
  // Define all 8 directions (N, NE, E, SE, S, SW, W, NW)
  std::vector<sf::Vector2f> directions = {
      { 0.0f, -1.0f }, // North
      { 1.0f, -1.0f }, // North-East
      { 1.0f, 0.0f },  // East
      { 1.0f, 1.0f },  // South-East
      { 0.0f, 1.0f },  // South
      { -1.0f, 1.0f }, // South-West
      { -1.0f, 0.0f }, // West
      { -1.0f, -1.0f } // North-West
  };

  // Priority order for direction selection
  std::vector<sf::Vector2f> preferred_directions;

  if ( player_direction != sf::Vector2f( 0.0f, 0.0f ) )
  {
    // Player is moving - prefer pushing opposite to movement direction
    sf::Vector2f opposite_dir = -player_direction.normalized();
    preferred_directions.push_back( opposite_dir );

    // Add perpendicular directions as secondary options
    sf::Vector2f perp1 = sf::Vector2f( -opposite_dir.y, opposite_dir.x );
    sf::Vector2f perp2 = sf::Vector2f( opposite_dir.y, -opposite_dir.x );
    preferred_directions.push_back( perp1 );
    preferred_directions.push_back( perp2 );
  }
  else
  {
    // Player is stationary - prefer pushing away from NPC
    sf::Vector2f away_from_npc = player_pos - npc_pos;
    if ( away_from_npc != sf::Vector2f( 0.0f, 0.0f ) ) { preferred_directions.push_back( away_from_npc.normalized() ); }
  }

  // Add all 8 directions to ensure we check everything
  for ( const auto &dir : directions )
  {
    preferred_directions.push_back( dir );
  }

  // Try each direction in priority order
  for ( const auto &push_dir : preferred_directions )
  {
    sf::FloatRect candidate_pos{ player_pos + push_dir.normalized() * pushback_distance, kGridSquareSizePixelsF };
    candidate_pos = snap_to_grid( candidate_pos );

    // Check if this position is valid and different from current position
    if ( candidate_pos.position != player_pos && is_valid_move( candidate_pos ) ) { return candidate_pos.position; }
  }

  // If no valid position found, return original position
  return player_pos;
}

void CollisionSystem::check_loot_collision()
{
  // Store both loot effects and the player velocities
  struct LootEffect
  {
    entt::entity loot_entity;
    Sprites::SpriteMetaType type;
    entt::entity player_entity;
  };

  std::vector<LootEffect> loot_effects;

  // First pass: detect collisions and gather effects to apply
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::WeaponLevel>();
  auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_weapon_level] : player_collision_view.each() )
  {
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp] : loot_collision_view.each() )
    {
      if ( not is_visible_in_view( RenderSystem::getGameView(), loot_pos_cmp ) ) continue;

      if ( pc_pos_cmp.findIntersection( loot_pos_cmp ) )
      {
        // Store effect to apply after collision detection
        loot_effects.push_back( { loot_entt, loot_cmp.m_type, pc_entt } );
      }
    }
  }

  // Second pass: apply effects and remove loots
  for ( const auto &effect : loot_effects )
  {
    if ( !m_reg->valid( effect.player_entity ) ) continue;

    auto &pc_cmp = m_reg->get<Cmp::PlayableCharacter>( effect.player_entity );
    auto &weapon_level_cmp = m_reg->get<Cmp::WeaponLevel>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
      pc_cmp.health = std::min( pc_cmp.health + health_bonus.get_value(), 100 );
    }
    else if ( effect.type == "EXTRA_BOMBS" )
    {
      auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
      if ( pc_cmp.bomb_inventory >= 0 ) pc_cmp.bomb_inventory += bomb_bonus.get_value();
    }
    else if ( effect.type == "LOWER_WATER" )
    {
      auto &water_bonus = get_persistent_component<Cmp::Persistent::WaterBonus>();
      for ( auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each() )
      {
        water_level.m_level = std::min( water_level.m_level + water_bonus.get_value(), static_cast<float>( kDisplaySize.y ) );
      }
    }

    else if ( effect.type == "INFINI_BOMBS" ) { pc_cmp.bomb_inventory = -1; }
    else if ( effect.type == "CHAIN_BOMBS" ) { pc_cmp.blast_radius = std::clamp( pc_cmp.blast_radius + 1, 0, 3 ); }
    else if ( effect.type == "WEAPON_BOOST" )
    {
      // increase weapon level by 50, up to max level 100
      weapon_level_cmp.m_level = std::clamp( weapon_level_cmp.m_level + 50.f, 0.f, 100.f );
    }

    // Remove the loot component
    m_reg->remove<Cmp::Loot>( effect.loot_entity );
    m_reg->emplace_or_replace<Cmp::Destructable>( effect.loot_entity );
  }
}

void CollisionSystem::check_player_large_obstacle_collision( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerScore>();
  auto large_obstacle_view = m_reg->view<Cmp::LargeObstacle>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_score_cmp] : player_view.each() )
  {
    // slightly larger hitbox for large obstacles because we want to trigger
    // collision when we get CLOSE to them
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, kGridSquareSizePixelsF, 1.5f );

    for ( auto [lo_entity, lo_cmp] : large_obstacle_view.each() )
    {
      // only spawn one npc per large obstacle but give proportionate score for the grave size
      uint8_t grave_activated_score = 0;

      if ( player_hitbox.findIntersection( lo_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with LargeObstacle at ({}, {})", lo_cmp.position.x, lo_cmp.position.y );

        // check for shrine collisions
        auto shrine_view = m_reg->view<Cmp::ShrineSprite, Cmp::Position>();
        for ( auto [shrine_entity, shrine_cmp, shrine_pos_cmp] : shrine_view.each() )
        {
          // is shrine part of the collided large obstacle?
          if ( not lo_cmp.findIntersection( shrine_pos_cmp ) ) continue;

          // already activated shrine? - skip
          auto anim_sprite_cmp = m_reg->try_get<Cmp::SpriteAnimation>( shrine_entity );
          if ( anim_sprite_cmp ) continue;

          // player doesn't have enough score? - skip
          auto &shrine_cost = get_persistent_component<Cmp::Persistent::ShrineCost>();
          if ( pc_score_cmp.get_score() < shrine_cost.get_value() ) continue;

          // Convert pixel size to grid size, then calculate threshold
          auto lo_grid_size = lo_cmp.size.componentWiseDiv( kGridSquareSizePixelsF );
          auto lo_count_threshold = ( lo_grid_size.x * lo_grid_size.y );

          if ( lo_cmp.get_activated_sprite_count() < lo_count_threshold )
          {
            // activate the next shrine sprite
            m_reg->emplace_or_replace<Cmp::SpriteAnimation>( shrine_entity, 0, 1 );
            lo_cmp.increment_activated_sprite_count();
            pc_score_cmp.decrement_score( shrine_cost.get_value() );

            // Did we just fully activate the shrine?
            if ( lo_cmp.get_activated_sprite_count() >= lo_count_threshold )
            {
              lo_cmp.set_powers_active();
              // unlock the door (internally checks if we activated all of the shrines)
              getEventDispatcher().trigger( Events::UnlockDoorEvent() );
            }
          }
        }

        // check for grave collisions
        auto grave_view = m_reg->view<Cmp::GraveSprite, Cmp::Position>();
        for ( auto [grave_entity, grave_cmp, grave_pos_cmp] : grave_view.each() )
        {
          // is grave part of the collided large obstacle?
          if ( not lo_cmp.findIntersection( grave_pos_cmp ) ) continue;

          // have we activated all the parts of the grave yet?
          auto &grave_ms = m_sprite_factory.get_multisprite_by_type( grave_cmp.getType() );
          auto activation_threshold = grave_ms.get_grid_size().width * grave_ms.get_grid_size().height;
          if ( lo_cmp.get_activated_sprite_count() < activation_threshold )
          {

            // switch to 2nd pair sprite indices - see Grave types in res/json/sprite_metadata.json
            // [ 0 ] --> becomes [ 2 ]
            // [ 1 ] --> becomes [ 3 ]
            // or
            // [ 0 ][ 1 ] --> becomes [ 4 ][ 5 ]
            // [ 2 ][ 3 ] --> becomes [ 6 ][ 7 ]
            auto current_index = grave_cmp.getTileIndex();
            grave_cmp.setTileIndex( current_index += 2 * grave_ms.get_grid_size().width );

            lo_cmp.increment_activated_sprite_count();
            grave_activated_score += 1;
          }
        }
      }

      // spawn npc event after processing all possible shrine/grave activations
      if ( grave_activated_score > 0 )
      {
        auto grave_activation_rng = Cmp::RandomInt( 1, 3 );
        auto consequence = grave_activation_rng.gen();
        switch ( consequence )
        {
          case 1:
            SPDLOG_INFO( "Spawning NPC from grave activation." );
            getEventDispatcher().trigger( Events::NpcCreationEvent( lo_entity, "NPCGHOST" ) );
            break;
          case 2:
            SPDLOG_INFO( "Dropping bomb from grave activation." );
            pc_cmp.bomb_inventory += 1;
            getEventDispatcher().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_BOMB ) );
            break;
          case 3:
            SPDLOG_INFO( "Increasing player score from grave activation." );
            pc_score_cmp.increment_score( grave_activated_score );
            break;
        }
      }
    }
  }
}

} // namespace ProceduralMaze::Sys