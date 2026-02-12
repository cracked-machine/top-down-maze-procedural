#include <Components/Random.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/Wall.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Random.hpp>
#include <entt/entity/fwd.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Player/PlayerSpeedPenalty.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/RuinSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

RuinSystem::RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                        entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
}

void RuinSystem::update()
{
  auto player_pos = Utils::get_player_position( getReg() );
  auto ruindoor_view = getReg().view<Cmp::RuinEntrance, Cmp::Position>();
  for ( auto [door_entity, ruindoor_cmp, door_pos_cmp] : ruindoor_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

    // Player can't intersect with a closed crypt door so expand their hitbox to facilitate collision detection
    auto player_hitbox = Cmp::RectBounds( player_pos.position, player_pos.size, 0.5f );
    if ( not player_hitbox.findIntersection( door_pos_cmp ) ) continue;

    auto ruin_mb_view = getReg().view<Cmp::RuinBuildingMultiBlock>();
    for ( auto [ruin_mb_entity, ruin_mb_cmp] : ruin_mb_view.each() )
    {
      if ( door_pos_cmp.findIntersection( ruin_mb_cmp ) )
      {
        getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y - 16.f );
        SPDLOG_DEBUG( "check_entrance_collision: Player entering ruin from graveyard at position ({}, {})", player_pos.position.x,
                      player_pos.position.y );
        m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_RUIN_LOWER );

        // remember player position
        auto last_player_pos = Factory::add_player_last_graveyard_pos( getReg(), door_pos_cmp );

        // drop any inventory outside the door
        auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
        auto dropped_entt = Factory::dropInventorySlotIntoWorld( getReg(), last_player_pos,
                                                                 m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }

        auto player_entt = Utils::get_player_entity( getReg() );
        getReg().emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::LOWER );
      }
      else { getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y + 16.f ); }
    }
  }
}

void RuinSystem::spawn_floor_access( sf::Vector2f spawn_position, sf::Vector2f size, Cmp::RuinFloorAccess::Direction dir )
{
  auto floor_access_entt = getReg().create();
  getReg().emplace_or_replace<Cmp::RuinFloorAccess>( floor_access_entt, spawn_position, size, dir );
  SPDLOG_DEBUG( "Spawning floor access at {},{}", spawn_position.x, spawn_position.y );
}

void RuinSystem::check_floor_access_collision( Cmp::RuinFloorAccess::Direction direction )
{
  if ( m_floor_access_cooldown.getElapsedTime().asSeconds() < kFloorAccessCooldownSeconds ) { return; }

  auto player_pos = Utils::get_player_position( getReg() );
  bool currently_on_floor_access = false;

  for ( auto [access_entt, access_cmp] : getReg().view<Cmp::RuinFloorAccess>().each() )
  {
    if ( player_pos.findIntersection( access_cmp ) )
    {
      currently_on_floor_access = true;

      // Only trigger if player wasn't already on floor access (must leave and re-enter)
      if ( m_was_on_floor_access ) { continue; }

      // Check if THIS floor access entity leads in the direction we're looking for
      if ( access_cmp.m_direction != direction ) { continue; }

      switch ( access_cmp.m_direction )
      {
        case Cmp::RuinFloorAccess::Direction::TO_UPPER:
          m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_RUIN_UPPER );
          break;
        case Cmp::RuinFloorAccess::Direction::TO_LOWER:
          m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_RUIN_UPPER );
          break;
      }
    }
  }

  // Update tracking - player must leave floor access area before it can trigger again
  m_was_on_floor_access = currently_on_floor_access;
}

void RuinSystem::check_starcase_multiblock_collision()
{
  bool slowdown = false;
  auto player_pos = Utils::get_player_position( getReg() );
  for ( auto [access_entt, access_cmp] : getReg().view<Cmp::RuinStairsLowerMultiBlock>().each() )
  {
    if ( player_pos.findIntersection( access_cmp ) ) { slowdown = true; }
  }
  for ( auto [access_entt, access_cmp] : getReg().view<Cmp::RuinStairsUpperMultiBlock>().each() )
  {
    if ( player_pos.findIntersection( access_cmp ) ) { slowdown = true; }
  }

  if ( slowdown )
  {
    auto player_entt = Utils::get_player_entity( getReg() );
    getReg().emplace_or_replace<Cmp::PlayerSpeedPenalty>( player_entt, 0.7 );
  }
  else
  {
    auto player_entt = Utils::get_player_entity( getReg() );
    if ( getReg().any_of<Cmp::PlayerSpeedPenalty>( player_entt ) ) { getReg().remove<Cmp::PlayerSpeedPenalty>( player_entt ); }
  }
}

void RuinSystem::gen_lowerfloor_obstacles( sf::FloatRect scene_dimensions )
{
  using namespace std::views;
  constexpr auto &gridsize = Constants::kGridSquareSizePixelsF;

  auto check_existing_collisions = [&]( Cmp::RectBounds pos )
  {
    if ( Utils::check_pos_collision<Cmp::RuinStairsLowerMultiBlock>( getReg(), pos ) ) return true;
    if ( Utils::check_cmp_collision<Cmp::RuinBookcase>( getReg(), pos ) ) return true;
    if ( Utils::check_cmp_collision<Cmp::Wall>( getReg(), pos ) ) return true;
    // ensure bookcase is inside scene
    if ( not Cmp::RectBounds( pos.position(), pos.size(), 1.5 ).findIntersection( scene_dimensions ) ) return true;
    return false;
  };

  SPDLOG_INFO( "gen_lowerfloor_obstacles" );
  auto excludepack = Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::RuinBookcase, Cmp::RuinCobweb>{};
  for ( auto _ : iota( 0, 100 ) )
  {
    auto [pos_entt, pos_cmp] = Utils::Rnd::get_random_position( getReg(), {}, excludepack, 0 );

    // bookcase left edge - use a scaled temporary to check for adjacent bookcases, walls, etc...
    Cmp::RectBounds left_side_pos( pos_cmp.position, pos_cmp.size, 1 );
    if ( check_existing_collisions( Cmp::RectBounds( left_side_pos.position(), left_side_pos.size(), 1.5 ) ) ) continue;
    Factory::create_bookcase( getReg(), left_side_pos.position(), m_sprite_factory.get_multisprite_by_type( "RUIN.bookcase_leftedge" ) );

    Cmp::RectBounds mid_pos = left_side_pos;
    Cmp::RandomInt bookcase_length( 2, 6 );
    for ( auto col : iota( 1, bookcase_length.gen() ) )
    {
      // bookcase middle pieces - x1.5 scale provides space padding
      mid_pos = Cmp::RectBounds( { pos_cmp.position.x + gridsize.x * col, pos_cmp.position.y }, gridsize, 1 );
      std::vector<std::string> pick_list = { "RUIN.bookcase_cobweb", "RUIN.bookcase_jars", "RUIN.bookcase_potions", "RUIN.bookcase_scrolls" };
      auto ms_type = m_sprite_factory.get_random_type( pick_list );
      if ( check_existing_collisions( Cmp::RectBounds( mid_pos.position(), mid_pos.size(), 1, Cmp::RectBounds::ScaleCardinality::VERTICAL ) ) )
      {
        // we hit something, so undo the last X pos advance to prevent gaps between here and the end right edge piece
        mid_pos = Cmp::RectBounds( { pos_cmp.position.x - gridsize.x, pos_cmp.position.y }, gridsize, 1 );
        break;
      }
      Factory::create_bookcase( getReg(), mid_pos.position(), m_sprite_factory.get_multisprite_by_type( ms_type ) );
    }

    // bookcase right edge -
    Cmp::RectBounds right_edge_pos( { mid_pos.position().x + gridsize.x, mid_pos.position().y }, gridsize, 1 );
    if ( check_existing_collisions( right_edge_pos ) ) continue;
    Factory::create_bookcase( getReg(), right_edge_pos.position(), m_sprite_factory.get_multisprite_by_type( "RUIN.bookcase_rightedge" ) );
  }
}

} // namespace ProceduralMaze::Sys