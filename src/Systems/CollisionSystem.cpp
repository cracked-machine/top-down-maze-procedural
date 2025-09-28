#include <CollisionSystem.hpp>
#include <CorruptionCell.hpp>
#include <HazardFieldCell.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <Persistent/PlayerMinVelocity.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <SinkholeCell.hpp>

namespace ProceduralMaze::Sys {

void CollisionSystem::init_context()
{
  add_persistent_component<Cmp::Persistent::HealthBonus>( *m_reg );
  add_persistent_component<Cmp::Persistent::BombBonus>( *m_reg );
  add_persistent_component<Cmp::Persistent::WaterBonus>( *m_reg );
  add_persistent_component<Cmp::Persistent::NpcDamage>( *m_reg );
  add_persistent_component<Cmp::Persistent::ObstaclePushBack>( *m_reg );
  add_persistent_component<Cmp::Persistent::NpcPushBack>( *m_reg );
  add_persistent_component<Cmp::Persistent::NPCActivateScale>( *m_reg );
  add_persistent_component<Cmp::Persistent::NpcDamageDelay>( *m_reg );
  add_persistent_component<Cmp::Persistent::CorruptionDamage>( *m_reg );
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
  for ( auto [_pc_entt, _pc, _pc_pos, _direction] : player_collision_view.each() )
  {

    auto player_hitbox = get_hitbox( _pc_pos );
    for ( auto [_npc_entt, _npc, _npc_pos] : npc_collision_view.each() )
    {

      auto npc_hitbox = get_hitbox( _npc_pos );

      if ( player_hitbox.findIntersection( npc_hitbox ) )
      {
        auto &npc_damage_cooldown = m_reg->ctx().get<Cmp::Persistent::NpcDamageDelay>();
        if ( _npc.m_damage_cooldown.getElapsedTime().asSeconds() < npc_damage_cooldown() ) continue;

        auto &npc_damage = m_reg->ctx().get<Cmp::Persistent::NpcDamage>();
        _pc.health -= npc_damage();

        _npc.m_damage_cooldown.restart();
        // Check if player is moving
        if ( _direction != sf::Vector2f( 0.0f, 0.0f ) )
        {
          // Push back in opposite direction of travel
          auto &npc_push_back = m_reg->ctx().get<Cmp::Persistent::NpcPushBack>();
          _pc_pos -= _direction.normalized() * npc_push_back();
        }
        else
        {
          // If not moving, use the distance as a direction of travel
          sf::Vector2f push_dir = { _pc_pos.x - _npc_pos.x, _pc_pos.y - _npc_pos.y };
          if ( push_dir != sf::Vector2f( 0.0f, 0.0f ) )
          {
            _pc_pos += push_dir.normalized().componentWiseMul(
                sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
          }
        }
      }

      if ( _pc.health <= 0 )
      {
        _pc.alive = false;
        return;
      }
    }
  }
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