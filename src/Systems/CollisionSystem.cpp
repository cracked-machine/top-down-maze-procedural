#include <CollisionSystem.hpp>

namespace ProceduralMaze::Sys {

void CollisionSystem::init_context()
{
  if ( not m_reg->ctx().contains<Cmp::Persistent::HealthBonus>() ) { m_reg->ctx().emplace<Cmp::Persistent::HealthBonus>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::BombBonus>() ) { m_reg->ctx().emplace<Cmp::Persistent::BombBonus>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::WaterBonus>() ) { m_reg->ctx().emplace<Cmp::Persistent::WaterBonus>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::NpcDamage>() ) { m_reg->ctx().emplace<Cmp::Persistent::NpcDamage>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::ObstaclePushBack>() ) { m_reg->ctx().emplace<Cmp::Persistent::ObstaclePushBack>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::NpcPushBack>() ) { m_reg->ctx().emplace<Cmp::Persistent::NpcPushBack>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::NPCActivateScale>() ) { m_reg->ctx().emplace<Cmp::Persistent::NPCActivateScale>(); }
  if ( not m_reg->ctx().contains<Cmp::Persistent::NpcDamageDelay>() ) { m_reg->ctx().emplace<Cmp::Persistent::NpcDamageDelay>(); }
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
      if ( _obstacle.m_type != Sprites::SpriteFactory::SpriteMetaType::BONES || not _obstacle.m_enabled || not _obstacle.m_visible ) continue;

      auto &npc_activate_scale = m_reg->ctx().get<Cmp::Persistent::NPCActivateScale>();
      // we just create a temporary RectBounds here instead of a component
      // because we only need it for this one comparison and it already contains the needed scaling logic
      auto npc_activate_bounds = Cmp::RectBounds( _obstacle_pos, sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE }, npc_activate_scale() );
      if ( player_hitbox.findIntersection( npc_activate_bounds.getBounds() ) )
      {
        // dont really care what obstacle this becomes as long as its disabled.
        m_reg->emplace_or_replace<Cmp::Obstacle>( _obstacle_entt, Sprites::SpriteFactory::SpriteMetaType::BONES, 0, false, false );
        getEventDispatcher().trigger( Events::NpcCreationEvent( _obstacle_pos ) );
      }
    }
  }
}

void CollisionSystem::check_player_to_npc_collision()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::Movement>();
  auto npc_collision_view = m_reg->view<Cmp::NPC, Cmp::Position>();
  for ( auto [_pc_entt, _pc, _pc_pos, _direction, _movement] : player_collision_view.each() )
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
        if ( _direction.x != 0.f || _direction.y != 0.f )
        {
          // Push back in opposite direction of travel
          auto &npc_push_back = m_reg->ctx().get<Cmp::Persistent::NpcPushBack>();
          _pc_pos -= _direction.normalized() * npc_push_back();
        }
        else
        {
          // If not moving, use the distance as a direction of travel
          sf::Vector2f push_dir = { _pc_pos.x - _npc_pos.x, _pc_pos.y - _npc_pos.y };
          _pc_pos += push_dir.normalized() * 8.f;
        }
      }

      if ( _pc.health <= 0 ) { _pc.alive = false; }
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
    sf::Vector2f original_velocity;
  };

  std::vector<LootEffect> loot_effects;

  // First pass: detect collisions and gather effects to apply
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>();
  auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

  for ( auto [_pc_entt, _pc, _pc_pos, _movement] : player_collision_view.each() )
  {
    auto player_hitbox = get_hitbox( _pc_pos );

    for ( auto [_loot_entt, _loot, _loot_pos] : loot_collision_view.each() )
    {
      auto loot_hitbox = get_hitbox( _loot_pos );
      if ( player_hitbox.findIntersection( loot_hitbox ) )
      {
        // Store effect to apply after collision detection, along with
        // original velocity
        loot_effects.push_back( { _loot_entt, _loot.m_type, _pc_entt, _movement.velocity } );
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
        water_level.m_level = std::min( water_level.m_level + water_bonus(), static_cast<float>( DISPLAY_SIZE.y ) );
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

    // Restore original movement velocity
    if ( m_reg->valid( effect.player_entity ) && m_reg->all_of<Cmp::Movement>( effect.player_entity ) )
    {
      auto &movement = m_reg->get<Cmp::Movement>( effect.player_entity );
      movement.velocity = effect.original_velocity;
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
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::PCDetectionBounds>();
  for ( auto [_pc_entt, _pc, _pc_pos, _movement, pc_detection_bounds] : player_view.each() )
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

void CollisionSystem::check_player_obstacle_collision()
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>();
  for ( auto [_pc_entt, _pc, _pc_pos, _movement] : player_view.each() )
  {
    sf::Vector2f starting_pos = { _pc_pos.x, _pc_pos.y };
    int stuck_loop = 0;

    // Reset collision flag at start of frame
    _movement.is_colliding = false;

    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
    for ( auto [_ob_entt, _ob, _ob_pos] : obstacle_view.each() )
    {

      // otherwise we are not interested in collision detection on traversable obstacles
      if ( not _ob.m_enabled ) { continue; }

      auto player_floatrect = get_hitbox( _pc_pos );
      auto brick_floatRect = get_hitbox( _ob_pos );

      auto collision = player_floatrect.findIntersection( brick_floatRect );
      if ( !collision ) continue;

      stuck_loop++;

      // We'll keep track if we're near the top wall for special handling
      bool near_top_wall = ( _pc_pos.y < MAP_GRID_OFFSET.y * 16.f );

      if ( stuck_loop > 5 ) // Reduced threshold, but we'll be smarter about resolution
      {
        // First try moving back to starting position
        _pc_pos.x = starting_pos.x;
        _pc_pos.y = starting_pos.y;

        player_floatrect = sf::FloatRect( { _pc_pos.x, _pc_pos.y }, sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE } );
        if ( !player_floatrect.findIntersection( brick_floatRect ) )
        {
          SPDLOG_INFO( "Recovered by reverting to start position" );
          continue;
        }

        // If still stuck, reset to spawn
        SPDLOG_INFO( "Could not recover, resetting to spawn" );
        _pc_pos = PLAYER_START_POS;
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.player_stuck = true;
        }
        return;
      }

      auto brickCenter = brick_floatRect.getCenter();
      auto playerCenter = player_floatrect.getCenter();

      auto diffX = playerCenter.x - brickCenter.x;
      auto diffY = playerCenter.y - brickCenter.y;

      auto minXDist = ( player_floatrect.size.x / 2.0f ) + ( brick_floatRect.size.x / 2.0f );
      auto minYDist = ( player_floatrect.size.y / 2.0f ) + ( brick_floatRect.size.y / 2.0f );

      // Calculate signed penetration depths
      float depthX = ( diffX > 0 ? 1.0f : -1.0f ) * ( minXDist - std::abs( diffX ) );
      float depthY = ( diffY > 0 ? 1.0f : -1.0f ) * ( minYDist - std::abs( diffY ) );

      // Store current position in case we need to revert
      sf::Vector2f pre_resolve_pos = { _pc_pos.x, _pc_pos.y };

      // Near top wall: Slightly bias vertical resolution but don't force it
      if ( near_top_wall && depthY > 0 )
      {
        // If pushing down would resolve collision and we're at the top wall,
        // slightly prefer vertical resolution (by reducing the X penetration)
        depthX *= 1.2f; // Makes horizontal resolution slightly less likely
      }

      auto &land_max_speed = m_reg->ctx().get<Cmp::Persistent::LandMaxSpeed>();
      auto &water_max_speed = m_reg->ctx().get<Cmp::Persistent::WaterMaxSpeed>();
      auto &friction_coefficient = m_reg->ctx().get<Cmp::Persistent::FrictionCoefficient>();
      auto &friction_falloff = m_reg->ctx().get<Cmp::Persistent::FrictionFalloff>();

      // Always resolve along the axis of least penetration
      if ( std::abs( depthX ) < std::abs( depthY ) )
      {
        // Push out along X axis
        _pc_pos.x += depthX * m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>()();

        // Calculate speed-based friction coefficient
        float speed_ratio = 0.f;
        if ( _pc.underwater ) { speed_ratio = std::abs( _movement.velocity.y ) / water_max_speed(); }
        else { speed_ratio = std::abs( _movement.velocity.y ) / land_max_speed(); }
        float dynamic_friction = friction_coefficient() * ( 1.0f - ( friction_falloff() * speed_ratio ) );

        // Apply friction to Y velocity with smooth falloff
        _movement.velocity.y *= ( 1.0f - dynamic_friction );

        // Check if Y velocity is below minimum
        if ( std::abs( _movement.velocity.y ) < _movement.min_velocity ) { _movement.velocity.y = 0.0f; }
      }
      else
      {
        // Push out along Y axis
        _pc_pos.y += depthY * m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>()();

        // Calculate speed-based friction coefficient
        float speed_ratio = 0.f;
        if ( _pc.underwater ) { speed_ratio = std::abs( _movement.velocity.x ) / water_max_speed(); }
        else { speed_ratio = std::abs( _movement.velocity.x ) / land_max_speed(); }
        float dynamic_friction = friction_coefficient() * ( 1.0f - ( friction_falloff() * speed_ratio ) );

        // Apply friction to X velocity with smooth falloff
        _movement.velocity.x *= ( 1.0f - dynamic_friction );

        // Check if X velocity is below minimum
        if ( std::abs( _movement.velocity.x ) < _movement.min_velocity ) { _movement.velocity.x = 0.0f; }
      }

      // Verify the resolution worked
      player_floatrect = get_hitbox( _pc_pos );
      if ( player_floatrect.findIntersection( brick_floatRect ) )
      {
        // If resolution failed, try reverting and using the other axis
        _pc_pos = pre_resolve_pos;
        if ( std::abs( depthX ) < std::abs( depthY ) )
        {
          _pc_pos.y += depthY * m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>()();
          _movement.velocity.x *= ( 1.0f - friction_coefficient() );
        }
        else
        {
          _pc_pos.x += depthX * m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>()();
          _movement.velocity.y *= ( 1.0f - friction_coefficient() );
        }
      }

      // Special case for top wall: prevent any upward movement
      if ( near_top_wall && _pc_pos.y < MAP_GRID_OFFSET.y * 16.f + 4.0f ) { _movement.velocity.y = std::max( 0.0f, _movement.velocity.y ); }

      // Mark that we're colliding for this frame
      _movement.is_colliding = true;

      // Extra safety for top wall
      if ( near_top_wall && _pc_pos.y < MAP_GRID_OFFSET.y * 16.f ) { _pc_pos.y = MAP_GRID_OFFSET.y * 16.f + 1.0f; }

      SPDLOG_DEBUG( "Collision resolved - new pos: {},{}", _pc_pos.x, _pc_pos.y );
    }
  }
}

} // namespace ProceduralMaze::Sys