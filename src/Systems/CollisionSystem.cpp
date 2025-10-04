#include <CollisionSystem.hpp>
#include <CorruptionCell.hpp>
#include <HazardFieldCell.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <Persistent/PlayerMinVelocity.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <SinkholeCell.hpp>

namespace ProceduralMaze::Sys {

CollisionSystem::CollisionSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  init_context();
}

void CollisionSystem::init_context()
{
  add_persistent_component<Cmp::Persistent::HealthBonus>();
  add_persistent_component<Cmp::Persistent::BombBonus>();
  add_persistent_component<Cmp::Persistent::WaterBonus>();
  add_persistent_component<Cmp::Persistent::NpcDamage>();
  add_persistent_component<Cmp::Persistent::ObstaclePushBack>();
  add_persistent_component<Cmp::Persistent::NpcPushBack>();
  add_persistent_component<Cmp::Persistent::NPCActivateScale>();
  add_persistent_component<Cmp::Persistent::NpcDamageDelay>();
  add_persistent_component<Cmp::Persistent::CorruptionDamage>();
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
    auto player_hitbox = get_hitbox( _pc_pos );
    for ( auto [_obstacle_entt, _obstacle, _obstacle_pos] : obstacle_collision_view.each() )
    {
      if ( _obstacle.m_type != Sprites::SpriteFactory::SpriteMetaType::BONES || not _obstacle.m_enabled ||
           not _obstacle.m_visible )
        continue;

      auto &npc_activate_scale = m_reg->ctx().get<Cmp::Persistent::NPCActivateScale>();
      // we just create a temporary RectBounds here instead of a component
      // because we only need it for this one comparison and it already contains the needed scaling
      // logic
      auto npc_activate_bounds = Cmp::RectBounds(
          _obstacle_pos, sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions }, npc_activate_scale() );
      if ( player_hitbox.findIntersection( npc_activate_bounds.getBounds() ) )
      {
        // dont really care what obstacle this becomes as long as its disabled.
        m_reg->emplace_or_replace<Cmp::Obstacle>( _obstacle_entt, Sprites::SpriteFactory::SpriteMetaType::BONES, 0,
                                                  false, false );
        getEventDispatcher().trigger( Events::NpcCreationEvent( _obstacle_pos ) );
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
    auto player_hitbox = get_hitbox( pc_pos_cmp );
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_collision_view.each() )
    {
      auto npc_hitbox = get_hitbox( npc_pos_cmp );

      if ( player_hitbox.findIntersection( npc_hitbox ) )
      {
        auto &npc_damage_cooldown = m_reg->ctx().get<Cmp::Persistent::NpcDamageDelay>();
        if ( npc_cmp.m_damage_cooldown.getElapsedTime().asSeconds() < npc_damage_cooldown() ) continue;

        auto &npc_damage = m_reg->ctx().get<Cmp::Persistent::NpcDamage>();
        pc_cmp.health -= npc_damage();

        npc_cmp.m_damage_cooldown.restart();

        auto &npc_push_back = m_reg->ctx().get<Cmp::Persistent::NpcPushBack>();

        // Find a valid pushback position by checking all 8 directions
        sf::Vector2f target_push_back_pos = findValidPushbackPosition( pc_pos_cmp, npc_pos_cmp, dir_cmp,
                                                                       npc_push_back() );

        // Update player position if we found a valid pushback position
        if ( target_push_back_pos != pc_pos_cmp ) { pc_pos_cmp = target_push_back_pos; }
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
    sf::Vector2f candidate_pos = player_pos + push_dir.normalized() * pushback_distance;
    candidate_pos = snap_to_grid( candidate_pos );

    // Check if this position is valid and different from current position
    if ( candidate_pos != player_pos && is_valid_move( candidate_pos ) ) { return candidate_pos; }
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
    Sprites::SpriteFactory::SpriteMetaType type;
    entt::entity player_entity;
  };

  std::vector<LootEffect> loot_effects;

  // First pass: detect collisions and gather effects to apply
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

  for ( auto [_pc_entt, _pc, _pc_pos] : player_collision_view.each() )
  {
    auto player_hitbox = get_hitbox( _pc_pos );

    for ( auto [_loot_entt, _loot, _loot_pos] : loot_collision_view.each() )
    {
      auto loot_hitbox = get_hitbox( _loot_pos );
      if ( player_hitbox.findIntersection( loot_hitbox ) )
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
    switch ( effect.type )
    {
      case Sprites::SpriteFactory::SpriteMetaType::EXTRA_HEALTH: {
        auto &health_bonus = m_reg->ctx().get<Cmp::Persistent::HealthBonus>();
        _pc.health = std::min( _pc.health + health_bonus(), 100 );
        break;
      }
      case Sprites::SpriteFactory::SpriteMetaType::EXTRA_BOMBS: {
        auto &bomb_bonus = m_reg->ctx().get<Cmp::Persistent::BombBonus>();
        if ( _pc.bomb_inventory >= 0 ) _pc.bomb_inventory += bomb_bonus();
        break;
      }
      case Sprites::SpriteFactory::SpriteMetaType::LOWER_WATER: {
        auto &water_bonus = m_reg->ctx().get<Cmp::Persistent::WaterBonus>();
        for ( auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each() )
        {
          water_level.m_level = std::min( water_level.m_level + water_bonus(), static_cast<float>( kDisplaySize.y ) );
          break;
        }
        break;
      }
      case Sprites::SpriteFactory::SpriteMetaType::INFINI_BOMBS:
        _pc.bomb_inventory = -1;
        break;

      case Sprites::SpriteFactory::SpriteMetaType::CHAIN_BOMBS:
        _pc.blast_radius = std::clamp( _pc.blast_radius + 1, 0, 3 );
        break;

      default:
        break;
    }

    // Remove the loot entity
    if ( m_reg->valid( effect.loot_entity ) ) { m_reg->erase<Cmp::Loot>( effect.loot_entity ); }
  }
}

void CollisionSystem::check_end_zone_collision()
{
  for ( auto [_entt, _pc, _pc_pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
  {
    auto player_hitbox = get_hitbox( _pc_pos );
    if ( player_hitbox.findIntersection( m_end_zone ) )
    {
      SPDLOG_INFO( "Player reached the end zone!" );
      for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
      {
        _sys.level_complete = true;
      }
    }
  }
}

void CollisionSystem::update_obstacle_distances()
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PCDetectionBounds>();
  for ( auto [_pc_entt, _pc, _pc_pos, pc_detection_bounds] : player_view.each() )
  {

    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [_ob_entt, _ob, _ob_pos] : obstacle_view.each() )
    {
      // while we are here calculate the obstacle/player distance for any traversable obstacles
      if ( not _ob.m_enabled && pc_detection_bounds.findIntersection( get_hitbox( _ob_pos ) ) )
      {
        auto distance = std::floor( getChebyshevDistance( _pc_pos, _ob_pos ) );
        m_reg->emplace_or_replace<Cmp::PlayerDistance>( _ob_entt, distance );
      }
      else { m_reg->remove<Cmp::PlayerDistance>( _ob_entt ); }
    }
  }
}

} // namespace ProceduralMaze::Sys