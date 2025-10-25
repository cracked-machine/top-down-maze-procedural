#include <CollisionSystem.hpp>
#include <CorruptionCell.hpp>
#include <HazardFieldCell.hpp>
#include <LargeObstacle.hpp>
#include <MultiSprite.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <Persistent/WaterBonus.hpp>
#include <PlayerScore.hpp>
#include <RectBounds.hpp>
#include <RenderSystem.hpp>
#include <ReservedPosition.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Window.hpp>
#include <SelectedPosition.hpp>
#include <SinkholeCell.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

CollisionSystem::CollisionSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  // register the event sinks
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&CollisionSystem::on_player_action>(
      this );
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
  auto obstacle_collision_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [_pc_entt, _pc, _pc_pos] : player_collision_view.each() )
  {
    for ( auto [_obstacle_entt, _obstacle, _obstacle_pos] : obstacle_collision_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getWindow().getView(), _obstacle_pos ) ) continue;

      if ( _obstacle.m_type != "BONES" || not _obstacle.m_enabled ) continue;

      auto &npc_activate_scale = get_persistent_component<Cmp::Persistent::NpcActivateScale>();
      // we just create a temporary RectBounds here instead of a component because we only need it for
      // this one comparison and it already contains the needed scaling logic
      auto npc_activate_bounds = Cmp::RectBounds(
          _obstacle_pos.position, sf::Vector2f{ BaseSystem::kGridSquareSizePixels }, npc_activate_scale.get_value() );

      if ( _pc_pos.findIntersection( npc_activate_bounds.getBounds() ) )
      {
        // dont really care what obstacle this becomes as long as its disabled.
        m_reg->emplace_or_replace<Cmp::Obstacle>( _obstacle_entt, "BONES", 0, false );
        getEventDispatcher().trigger( Events::NpcCreationEvent( _obstacle_pos.position ) );
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
        sf::Vector2f target_push_back_pos = findValidPushbackPosition( pc_pos_cmp.position, npc_pos_cmp.position,
                                                                       dir_cmp, npc_push_back.get_value() );

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
    sf::FloatRect candidate_pos{ player_pos + push_dir.normalized() * pushback_distance,
                                 sf::Vector2f{ BaseSystem::kGridSquareSizePixels } };
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
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

  for ( auto [_pc_entt, _pc, _pc_pos] : player_collision_view.each() )
  {

    for ( auto [_loot_entt, _loot, _loot_pos] : loot_collision_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getWindow().getView(), _loot_pos ) ) continue;

      if ( _pc_pos.findIntersection( _loot_pos ) )
      {
        // Store effect to apply after collision detection
        loot_effects.push_back( { _loot_entt, _loot.m_type, _pc_entt } );
      }
    }
  }

  // Second pass: apply effects and remove loots
  for ( const auto &effect : loot_effects )
  {
    if ( !m_reg->valid( effect.player_entity ) ) continue;

    auto &_pc = m_reg->get<Cmp::PlayableCharacter>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
      _pc.health = std::min( _pc.health + health_bonus.get_value(), 100 );
    }
    else if ( effect.type == "EXTRA_BOMBS" )
    {
      auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
      if ( _pc.bomb_inventory >= 0 ) _pc.bomb_inventory += bomb_bonus.get_value();
    }
    else if ( effect.type == "LOWER_WATER" )
    {
      auto &water_bonus = get_persistent_component<Cmp::Persistent::WaterBonus>();
      for ( auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each() )
      {
        water_level.m_level = std::min( water_level.m_level + water_bonus.get_value(),
                                        static_cast<float>( kDisplaySize.y ) );
      }
    }

    else if ( effect.type == "INFINI_BOMBS" ) { _pc.bomb_inventory = -1; }
    else if ( effect.type == "CHAIN_BOMBS" ) { _pc.blast_radius = std::clamp( _pc.blast_radius + 1, 0, 3 ); }

    // Remove the loot component
    m_reg->remove<Cmp::Loot>( effect.loot_entity );
  }
}

void CollisionSystem::check_end_zone_collision()
{
  for ( auto [_entt, _pc, _pc_pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
  {
    if ( _pc_pos.findIntersection( m_end_zone ) )
    {
      SPDLOG_DEBUG( "Player reached the end zone!" );
      for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
      {
        _sys.level_complete = true;
      }
    }
  }
}

void CollisionSystem::update_obstacle_distances()
{
  // precompute outside of loops for performance
  const auto viewBounds = BaseSystem::calculate_view_bounds( RenderSystem::getWindow().getView() );

  // we only have one player so this is just for convenience
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_db_cmp] : player_view.each() )
  {

    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [obst_entt, obst_cmp, obst_pos_cmp] : obstacle_view.each() )
    {

      if ( !is_visible_in_view( viewBounds, obst_pos_cmp ) ) continue;

      // while we are here calculate the obstacle/player distance for any traversable obstacles
      if ( not obst_cmp.m_enabled && pc_db_cmp.findIntersection( obst_pos_cmp ) )
      {
        auto distance = std::floor( getChebyshevDistance( pc_pos_cmp.position, obst_pos_cmp.position ) );
        m_reg->emplace_or_replace<Cmp::PlayerDistance>( obst_entt, distance );
      }
      else
      {
        // tidy up any out of range obstacles
        m_reg->remove<Cmp::PlayerDistance>( obst_entt );
      }
    }
  }
}

void CollisionSystem::check_player_large_obstacle_collision( Events::PlayerActionEvent::GameActions action )
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerScore>();
  auto large_obstacle_view = m_reg->view<Cmp::LargeObstacle>();
  using PlayerActionEvent = Events::PlayerActionEvent::GameActions;

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_score_cmp] : player_view.each() )
  {
    // slightly larger hitbox for large obstacles because we want to trigger
    // collision when we get CLOSE to them
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, sf::Vector2f{ BaseSystem::kGridSquareSizePixels },
                                          1.5f );

    for ( auto [lo_entity, lo_cmp] : large_obstacle_view.each() )
    {

      if ( player_hitbox.findIntersection( lo_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with LargeObstacle at ({}, {})", lo_cmp.position.x, lo_cmp.position.y );

        auto reserved_view = m_reg->view<Cmp::ReservedPosition>();
        for ( auto [_res_entity, reserved_cmp] : reserved_view.each() )
        {

          if ( reserved_cmp.findIntersection( lo_cmp ) )
          {
            // SHRINES

            if ( reserved_cmp.m_type == "SHRINE" && action == PlayerActionEvent::ACTIVATE )
            {
              if ( pc_score_cmp.get_score() >= 2 && !reserved_cmp.is_animated() )
              {

                // Convert pixel size to grid size, then calculate threshold
                auto lo_grid_size = lo_cmp.size.componentWiseDiv( sf::Vector2f{ BaseSystem::kGridSquareSizePixels } );
                auto lo_count_threshold = ( lo_grid_size.x * lo_grid_size.y );
                SPDLOG_DEBUG( " Count {}, threshold {} ", lo_cmp.get_active_count(), count_threshold );
                if ( lo_cmp.get_active_count() < lo_count_threshold )
                {
                  lo_cmp.increment_active_count();
                  pc_score_cmp.decrement_score( 2 );
                  reserved_cmp.animate();
                  m_reg->emplace_or_replace<Cmp::SpriteAnimation>( _res_entity, 0, 1 );
                  if ( lo_cmp.get_active_count() >= lo_count_threshold )
                  {
                    SPDLOG_DEBUG( "ACTIVATING SHRINE! Count: {}, Threshold: {}", lo_cmp.get_active_count(),
                                  count_threshold );
                    lo_cmp.set_powers_active();
                  }
                }
              }
            }

            // GRAVES
            else if ( reserved_cmp.m_type.contains( "GRAVE" ) && action == PlayerActionEvent::ACTIVATE )
            {
              if ( reserved_cmp.is_broken() == false )
              {

                pc_score_cmp.increment_score( 1 );
                reserved_cmp.break_object();
                auto grave_ms = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>()
                                    ->get_multisprite_by_type( reserved_cmp.m_type );

                // switch to 2nd pair sprite indices - see Grave types in res/json/sprite_metadata.json
                // [ 0 ] --> becomes [ 2 ]
                // [ 1 ] --> becomes [ 3 ]
                // or
                // [ 0 ][ 1 ] --> becomes [ 4 ][ 5 ]
                // [ 2 ][ 3 ] --> becomes [ 6 ][ 7 ]
                reserved_cmp.m_sprite_index += 2 * grave_ms->get_grid_size().width;
              }
            }
          }
        }
      }
    }
  }
}

} // namespace ProceduralMaze::Sys