#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/RectBounds.hpp>
#include <Components/WormholeJump.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <Components/Destructable.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Random.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ShrineSegment.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

namespace ProceduralMaze::Sys
{

NpcSystem::NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                      Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = Sys::BaseSystem::get_systems_event_queue()
                    .sink<Events::NpcCreationEvent>()
                    .connect<&Sys::NpcSystem::on_npc_creation>( this );
  std::ignore = Sys::BaseSystem::get_systems_event_queue().sink<Events::NpcDeathEvent>().connect<&Sys::NpcSystem::on_npc_death>(
      this );
  SPDLOG_DEBUG( "NpcSystem initialized" );
}

void NpcSystem::add_npc_entity( const Events::NpcCreationEvent &event )
{
  auto pos_cmp = getReg().try_get<Cmp::Position>( event.position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component", static_cast<int>( event.position_entity ) );
    return;
  }

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = getReg().create();
  getReg().emplace<Cmp::Position>( new_pos_entity, pos_cmp->position, kGridSquareSizePixelsF );
  getReg().emplace<Cmp::Destructable>( new_pos_entity );
  getReg().emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
  getReg().emplace_or_replace<Cmp::NPCScanBounds>( new_pos_entity, pos_cmp->position, kGridSquareSizePixelsF,
                                                   npc_scan_scale.get_value() );
  if ( event.type == "NPCGHOST" )
  {
    getReg().emplace_or_replace<Cmp::NPC>( new_pos_entity );
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCGHOST.walk.east" );
  }
  else if ( event.type == "NPCSKELE" )
  {
    getReg().emplace_or_replace<Cmp::NPC>( new_pos_entity );
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCSKELE.walk.east" );
  }

  getReg().remove<Cmp::NpcContainer>( new_pos_entity );
  getReg().remove<Cmp::ReservedPosition>( new_pos_entity );
  getReg().remove<Cmp::Obstacle>( new_pos_entity );

  // Remove the npc container component from the original entity
  getReg().remove<Cmp::NpcContainer>( event.position_entity );

  if ( event.type == "NPCGHOST" )
  {
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), event.type,
                 pos_cmp->position.x, pos_cmp->position.y );
    m_sound_bank.get_effect( "spawn_ghost" ).play();
  }
  else if ( event.type == "NPCSKELE" )
  {
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), event.type,
                 pos_cmp->position.x, pos_cmp->position.y );
    m_sound_bank.get_effect( "spawn_skeleton" ).play();
  }
}

void NpcSystem::remove_npc_entity( entt::entity npc_entity )
{
  // check for position component
  auto npc_pos_cmp = getReg().try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp )
  {
    SPDLOG_WARN( "Cannot process loot drop for NPC entity {} without a Position component", static_cast<int>( npc_entity ) );
  }
  else
  {
    // 1 in 20 chance of dropping a relic
    auto loot_chance_rng = Cmp::RandomInt( 1, 10 );
    if ( loot_chance_rng.gen() == 1 )
    {
      auto npc_pos_cmp_bounds = Cmp::RectBounds( npc_pos_cmp->position, kGridSquareSizePixelsF, 1.5f );
      // clang-format off
      auto loot_entity = create_loot_drop( 
        Cmp::Loot( "RELIC_DROP", 0 ),                                   
        sf::FloatRect{ npc_pos_cmp_bounds.position(), npc_pos_cmp_bounds.size() },
        IncludePack<>{}, 
        ExcludePack<>{} 
      );
      // clang-format on
      if ( loot_entity != entt::null )
      {
        SPDLOG_INFO( "Dropped RELIC_DROP loot at NPC death position." );
        m_sound_bank.get_effect( "drop_relic" ).play();
      }
    }
  }

  // kill npc once we are done
  getReg().remove<Cmp::NPC>( npc_entity );
  getReg().remove<Cmp::Position>( npc_entity );
  getReg().remove<Cmp::NPCScanBounds>( npc_entity );
  getReg().remove<Cmp::Direction>( npc_entity );
}

//! @brief Check if diagonal movement should be blocked due to adjacent obstacles
//! @param current_pos Current position rectangle
//! @param diagonal_direction The diagonal direction vector (e.g., {1, -1} for up-right)
//! @return true if diagonal movement is blocked by adjacent obstacles
bool NpcSystem::isDiagonalBlocked( const sf::FloatRect &current_pos, const sf::Vector2f &diagonal_direction )
{
  // Only check for diagonal movements (both x and y components non-zero)
  if ( diagonal_direction.x == 0.f || diagonal_direction.y == 0.f )
  {
    return false; // Not a diagonal move
  }

  // Create test positions for the two cardinal directions
  sf::FloatRect horizontal_test = current_pos;
  horizontal_test.position.x += diagonal_direction.x * kGridSquareSizePixelsF.x;

  sf::FloatRect vertical_test = current_pos;
  vertical_test.position.y += diagonal_direction.y * kGridSquareSizePixelsF.y;

  // If EITHER cardinal direction is blocked, block the diagonal
  bool horizontal_blocked = !is_valid_move( horizontal_test );
  bool vertical_blocked = !is_valid_move( vertical_test );

  return horizontal_blocked || vertical_blocked;
}

void NpcSystem::update_movement( sf::Time globalDeltaTime )
{
  auto exclusions = entt::exclude<Cmp::ShrineSegment, Cmp::SpawnAreaSprite, Cmp::PlayableCharacter>;
  auto view = getReg().view<Cmp::Position, Cmp::LerpPosition, Cmp::NPCScanBounds, Cmp::Direction>( exclusions );

  for ( auto [entity, pos_cmp, lerp_pos_cmp, npc_scan_bounds, dir_cmp] : view.each() )
  {

    // skip over obstacles that are still enabled i.e. dont travel though them
    auto obst_cmp = getReg().try_get<Cmp::Obstacle>( entity );
    if ( obst_cmp && obst_cmp->m_enabled ) continue;

    // If this is the first update, store the start position
    if ( lerp_pos_cmp.m_lerp_factor == 0.0f )
    {
      // Allow NPCs to escape wormholes if they're mid-lerp.
      if ( getReg().try_get<Cmp::WormholeJump>( entity ) ) continue;

      lerp_pos_cmp.m_start = pos_cmp.position;
    }

    lerp_pos_cmp.m_lerp_factor += lerp_pos_cmp.m_lerp_speed * globalDeltaTime.asSeconds();

    if ( lerp_pos_cmp.m_lerp_factor >= 1.0f )
    {
      pos_cmp.position = lerp_pos_cmp.m_target;
      getReg().remove<Cmp::LerpPosition>( entity );
    }
    else
    {
      // Lerp from start to target directly
      pos_cmp.position.x = std::lerp( lerp_pos_cmp.m_start.x, lerp_pos_cmp.m_target.x, lerp_pos_cmp.m_lerp_factor );
      pos_cmp.position.y = std::lerp( lerp_pos_cmp.m_start.y, lerp_pos_cmp.m_target.y, lerp_pos_cmp.m_lerp_factor );
    }

    // clang-format off
    getReg().patch<Cmp::NPCScanBounds>( entity, 
      [&]( auto &npc_scan_bounds ) 
      { 
        npc_scan_bounds.position( pos_cmp.position ); 
      });
    // clang-format on
  }
}

void NpcSystem::check_bones_reanimation()
{
  auto player_collision_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto npccontainer_collision_view = getReg().view<Cmp::NpcContainer, Cmp::Position>();
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
        getReg().emplace_or_replace<Cmp::Obstacle>( npccontainer_entt, "BONES", 0, false );
        get_systems_event_queue().trigger( Events::NpcCreationEvent( npccontainer_entt, "NPCSKELE" ) );
      }
    }
  }
}

void NpcSystem::check_player_to_npc_collision()
{
  auto player_collision_view = getReg()
                                   .view<Cmp::PlayableCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position,
                                         Cmp::Direction>();
  auto npc_collision_view = getReg().view<Cmp::NPC, Cmp::Position>();
  auto &npc_push_back = get_persistent_component<Cmp::Persistent::NpcPushBack>();
  auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();

  for ( auto [pc_entity, pc_cmp, pc_health_cmp, pc_mort_cmp, pc_pos_cmp, dir_cmp] : player_collision_view.each() )
  {
    if ( pc_mort_cmp.state != Cmp::PlayerMortality::State::ALIVE ) return;
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_collision_view.each() )
    {
      // relaxed bounds to allow player to sneak past during lerp transition
      Cmp::RectBounds npc_pos_cmp_bounds_current{ npc_pos_cmp.position, npc_pos_cmp.size, 0.1f };
      if ( not pc_pos_cmp.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) continue;

      if ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() ) continue;

      auto &npc_damage = get_persistent_component<Cmp::Persistent::NpcDamage>();
      pc_health_cmp.health -= npc_damage.get_value();

      m_sound_bank.get_effect( "damage_player" ).play();

      if ( pc_health_cmp.health <= 0 )
      {
        pc_mort_cmp.state = Cmp::PlayerMortality::State::HAUNTED;
        return;
      }

      pc_cmp.m_damage_cooldown_timer.restart();

      // Find a valid pushback position by checking all 8 directions
      sf::Vector2f target_push_back_pos = findValidPushbackPosition( pc_pos_cmp.position, npc_pos_cmp.position, dir_cmp,
                                                                     npc_push_back.get_value() );

      // Update player position if we found a valid pushback position
      if ( target_push_back_pos != pc_pos_cmp.position ) { pc_pos_cmp.position = target_push_back_pos; }
    }
  }
}

void NpcSystem::on_npc_death( const Events::NpcDeathEvent &event )
{
  SPDLOG_DEBUG( "NPC Death Event received" );
  remove_npc_entity( event.npc_entity );
}
void NpcSystem::on_npc_creation( const Events::NpcCreationEvent &event )
{
  SPDLOG_DEBUG( "NPC Creation Event received" );
  add_npc_entity( event );
}

sf::Vector2f NpcSystem::findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos,
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
    sf::Vector2f opposite_dir = -player_direction;

    // Normalize for comparison
    float mag = std::sqrt( opposite_dir.x * opposite_dir.x + opposite_dir.y * opposite_dir.y );
    if ( mag > 0.0f )
    {
      opposite_dir.x /= mag;
      opposite_dir.y /= mag;
    }

    // Find the closest matching cardinal/diagonal direction
    float best_dot = -2.0f;
    sf::Vector2f best_dir;
    for ( const auto &dir : directions )
    {
      float dot = opposite_dir.x * dir.x + opposite_dir.y * dir.y;
      if ( dot > best_dot )
      {
        best_dot = dot;
        best_dir = dir;
      }
    }

    preferred_directions.push_back( best_dir );

    // Add perpendicular directions as secondary options
    // Find directions that are perpendicular to best_dir
    for ( const auto &dir : directions )
    {
      float dot = std::abs( best_dir.x * dir.x + best_dir.y * dir.y );
      if ( dot < 0.1f ) // approximately perpendicular
      {
        preferred_directions.push_back( dir );
      }
    }
  }
  else
  {
    // Player is stationary - prefer pushing away from NPC
    sf::Vector2f away_from_npc = player_pos - npc_pos;
    if ( away_from_npc != sf::Vector2f( 0.0f, 0.0f ) )
    {
      // Normalize for comparison
      float mag = std::sqrt( away_from_npc.x * away_from_npc.x + away_from_npc.y * away_from_npc.y );
      if ( mag > 0.0f )
      {
        away_from_npc.x /= mag;
        away_from_npc.y /= mag;
      }

      // Find the closest matching cardinal/diagonal direction
      float best_dot = -2.0f;
      sf::Vector2f best_dir;
      for ( const auto &dir : directions )
      {
        float dot = away_from_npc.x * dir.x + away_from_npc.y * dir.y;
        if ( dot > best_dot )
        {
          best_dot = dot;
          best_dir = dir;
        }
      }
      preferred_directions.push_back( best_dir );
    }
  }

  // Add all 8 directions to ensure we check everything
  for ( const auto &dir : directions )
  {
    preferred_directions.push_back( dir );
  }

  // Try each direction in priority order
  for ( const auto &push_dir : preferred_directions )
  {
    sf::FloatRect candidate_pos{ player_pos + push_dir * pushback_distance, kGridSquareSizePixelsF };
    candidate_pos = snap_to_grid( candidate_pos );

    // Check if this position is valid and different from current position
    if ( candidate_pos.position != player_pos && is_valid_move( candidate_pos ) ) { return candidate_pos.position; }
  }

  // If no valid position found, return original position
  return player_pos;
}

} // namespace ProceduralMaze::Sys