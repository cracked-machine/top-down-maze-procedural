#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <SFML/Graphics/Rect.hpp>
#include <entt/entity/fwd.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Player/PlayerSpeedPenalty.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/RuinSystem.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Random.hpp>
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

void RuinSystem::check_movement_slowdowns()
{
  auto player_pos = Utils::get_player_position( getReg() );
  auto player_entt = Utils::get_player_entity( getReg() );

  float slowdown_penalty = 0.0f;

  // Check staircase collision
  if ( Utils::Collision::check_pos<Cmp::RuinStairsLowerMultiBlock>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ) ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.5f );
  }
  if ( Utils::Collision::check_pos<Cmp::RuinStairsUpperMultiBlock>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ) ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.5f );
  }

  // Check cobweb collision
  if ( Utils::Collision::check_cmp<Cmp::RuinCobweb>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ),
                                                     []( const Cmp::RuinCobweb &cobweb ) { return cobweb.integrity > 0; } ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.25f );
  }

  // Apply or remove penalty
  if ( slowdown_penalty > 0.0f ) { getReg().emplace_or_replace<Cmp::PlayerSpeedPenalty>( player_entt, slowdown_penalty ); }
  else
  {
    if ( getReg().any_of<Cmp::PlayerSpeedPenalty>( player_entt ) ) { getReg().remove<Cmp::PlayerSpeedPenalty>( player_entt ); }
  }
}

void RuinSystem::gen_lowerfloor_bookcases( sf::FloatRect scene_dimensions )
{
  SPDLOG_INFO( "gen_lowerfloor_obstacles" );
  using namespace std::views;

  auto has_collision = [&]( Cmp::RectBounds pos )
  {
    if ( Utils::Collision::check_cmp<Cmp::RuinBookcase>( getReg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::Wall>( getReg(), pos ) ) { return true; }

    // ensure bookcase is inside scene
    if ( not Cmp::RectBounds( pos.position(), pos.size(), 1.5 ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  constexpr auto &gridsize = Constants::kGridSquareSizePixelsF;
  constexpr auto ScaleCardinality_VERTICAL = Cmp::RectBounds::ScaleCardinality::VERTICAL;

  int max_rows = scene_dimensions.size.y / gridsize.y;
  int max_cols = scene_dimensions.size.x / gridsize.x;

  for ( int row : iota( 1, max_rows ) | std::views::stride( 2 ) )
  {
    for ( int col = 1; col < max_cols; /* incremented in loop body */ )
    {
      std::vector<std::pair<Cmp::RectBounds, Sprites::SpriteMetaType>> bookshelf_row_candidate{};
      bool valid_candidate = true;

      // column picker as we traverse row left->right: 1 in N chance of starting a new bookcase
      Cmp::RandomInt column_pick( 0, 1 );
      if ( column_pick.gen() )
      {
        SPDLOG_DEBUG( "Skipping col #{}", col );
        ++col;
        continue;
      }

      // bookcase left edge
      Cmp::RectBounds bc_left_coord( { col * gridsize.x, row * gridsize.y }, gridsize, 1 );
      SPDLOG_DEBUG( "Row #{} candidate left edge is {},{}", row, bc_left_coord.position().x, bc_left_coord.position().y );
      if ( has_collision( Cmp::RectBounds( bc_left_coord.position(), bc_left_coord.size(), 1.f ) ) )
      {
        SPDLOG_DEBUG( "Left edge rejected at {},{}", bc_left_coord.position().x, bc_left_coord.position().y );
        ++col;
        continue;
      }
      bookshelf_row_candidate.push_back( { bc_left_coord, "RUIN.bookcase_leftsection" } );

      // bookcase middle pieces - advance N times from the left edge section coord
      Cmp::RectBounds bc_mid_coord = bc_left_coord;
      Cmp::RandomInt bookcase_length( 2, 3 );

      for ( auto bc_mid_idx : iota( 1, bookcase_length.gen() ) )
      {
        bc_mid_coord = Cmp::RectBounds( { bc_left_coord.position().x + gridsize.x * bc_mid_idx, bc_left_coord.position().y }, gridsize, 1 );

        if ( has_collision( Cmp::RectBounds( bc_mid_coord.position(), bc_mid_coord.size(), 1, ScaleCardinality_VERTICAL ) ) )
        {
          SPDLOG_DEBUG( "Mid section rejected at {},{}", bc_mid_coord.position().x, bc_mid_coord.position().y );
          valid_candidate = false;
          break;
        }
        bookshelf_row_candidate.push_back( { bc_mid_coord, "RUIN.bookcase_midsection" } );
      }

      if ( not valid_candidate )
      {
        ++col;
        continue;
      }

      // bookcase right edge
      Cmp::RectBounds bc_right_coord( { bc_mid_coord.position().x + gridsize.x, bc_mid_coord.position().y }, gridsize, 1 );
      if ( has_collision( bc_right_coord ) )
      {
        SPDLOG_DEBUG( "Right edge rejected at {},{}", bc_right_coord.position().x, bc_right_coord.position().y );
        ++col;
        continue;
      }
      bookshelf_row_candidate.push_back( { bc_right_coord, "RUIN.bookcase_rightsection" } );

      // add the line of bookshelf sprites
      SPDLOG_DEBUG( "bookcase candidate of length: {}", bookshelf_row_candidate.size() );
      for ( auto [point, type] : bookshelf_row_candidate )
      {
        auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { type } );
        Factory::create_bookcase( getReg(), point.position(), m_sprite_factory.get_multisprite_by_type( ms ), idx );
      }

      // ensure we have one horizontal gap inbetween bookcases
      col += bookshelf_row_candidate.size() + 1;
    }
  }
}

void RuinSystem::add_lowerfloor_cobwebs( sf::FloatRect scene_dimensions )
{
  auto has_collision = [&]( Cmp::RectBounds pos )
  {
    if ( Utils::Collision::check_cmp<Cmp::RuinBookcase>( getReg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::RuinStairsLowerMultiBlock>( getReg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::Wall>( getReg(), pos, []( const Cmp::Wall &wall ) { return wall.blocking; } ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::RuinCobweb>( getReg(), pos ) ) { return true; }

    // ensure bookcase is inside scene
    if ( not Cmp::RectBounds( pos.position(), pos.size(), 1.5 ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  constexpr auto &gridsize = Constants::kGridSquareSizePixelsF;
  int max_cobwebs = 500;
  for ( auto _ : std::views::iota( 0, max_cobwebs ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( getReg(), {}, Utils::Rnd::ExcludePack<Cmp::RuinBookcase>{} );
    if ( rnd_entt == entt::null ) continue;

    if ( has_collision( Cmp::RectBounds( { rnd_pos.position }, gridsize, 1 ) ) ) continue;
    auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "RUIN.cobweb" } );
    Factory::create_cobweb( getReg(), rnd_pos.position, m_sprite_factory.get_multisprite_by_type( ms ), idx );
  }
}

} // namespace ProceduralMaze::Sys