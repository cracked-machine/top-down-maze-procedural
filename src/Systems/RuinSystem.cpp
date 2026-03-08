#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/HolyWell/HolyWellExit.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Player/PlayerSpeedPenalty.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinShadowHand.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/RuinSystem.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>

#include <numbers>
#include <set>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Sys
{

RuinSystem::RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                        entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
}

void RuinSystem::update( PathFinding::SpatialHashGrid *spatial_grid )
{
  // update the grid
  m_spatial_grid = spatial_grid;
}

void RuinSystem::check_entrance_collision()
{
  auto player_pos = Utils::Player::get_player_position( getReg() );
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
        auto [inventory_entt, inventory_slot_type] = Utils::Player::get_player_inventory_type( getReg() );
        auto dropped_entt = Factory::drop_inventory_slot_into_world(
            getReg(), last_player_pos, m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }

        auto player_entt = Utils::Player::get_player_entity( getReg() );
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

  auto player_pos = Utils::Player::get_player_position( getReg() );
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
  auto player_pos = Utils::Player::get_player_position( getReg() );
  auto player_entt = Utils::Player::get_player_entity( getReg() );

  float slowdown_penalty = 0.0f;

  // Check staircase collision
  if ( Utils::Collision::check_pos<Cmp::RuinStairsLowerMultiBlock>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ) ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.7f );
  }
  if ( Utils::Collision::check_pos<Cmp::RuinStairsUpperMultiBlock>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ) ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.7f );
  }

  // Check cobweb collision
  if ( Utils::Collision::check_cmp<Cmp::RuinCobweb>( getReg(), Cmp::RectBounds( player_pos.position, player_pos.size, 1 ),
                                                     []( const Cmp::RuinCobweb &cobweb ) { return cobweb.integrity > 0; } ) )
  {
    slowdown_penalty = std::max( slowdown_penalty, 0.5f );
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
    if ( Utils::Collision::check_cmp<Cmp::NpcNoPathFinding>( getReg(), pos ) ) { return true; }

    // ensure bookcase is inside scene
    if ( not Cmp::RectBounds( pos.position(), pos.size(), 1.5 ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  constexpr auto &gridsize = Constants::kGridSizePxF;
  constexpr auto ScaleCardinality_VERTICAL = Cmp::RectBounds::ScaleCardinality::VERTICAL;

  int max_rows = scene_dimensions.size.y / gridsize.y;
  int max_cols = scene_dimensions.size.x / gridsize.x;

  // odd number rows should be long broken lines of bookcases
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

  // even number rows are empty so lets add a single bookcase to block the player
  std::set<int> used_cols{};
  constexpr int max_attempts = 60;
  for ( int row : iota( 0, max_rows - 1 ) | std::views::stride( 2 ) )
  {
    for ( int attempt = 0; attempt < max_attempts; ++attempt )
    {
      Cmp::RandomInt column_pick( 0, max_cols );
      int colpick = column_pick.gen();
      Cmp::RectBounds point( { colpick * gridsize.x, row * gridsize.y }, gridsize, 1 );
      if ( has_collision( point ) or used_cols.contains( colpick ) ) continue;
      auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "RUIN.bookcase_midsection" } );
      Factory::create_bookcase( getReg(), point.position(), m_sprite_factory.get_multisprite_by_type( ms ), idx );
      used_cols.insert( colpick );
      break;
    }
  }

  // always block this col on the bottom row
  int colpick = 8;
  Cmp::RectBounds point( { colpick * gridsize.x, scene_dimensions.size.y - gridsize.y }, gridsize, 1 );
  auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "RUIN.bookcase_midsection" } );
  Factory::create_bookcase( getReg(), point.position(), m_sprite_factory.get_multisprite_by_type( ms ), idx );
  used_cols.insert( colpick );
}

void RuinSystem::add_lowerfloor_cobwebs( int max_attempts, sf::FloatRect scene_dimensions )
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

  constexpr auto &gridsize = Constants::kGridSizePxF;
  int max_cobwebs = max_attempts;
  for ( auto _ : std::views::iota( 0, max_cobwebs ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( getReg(), {}, Utils::Rnd::ExcludePack<Cmp::RuinBookcase>{} );
    if ( rnd_entt == entt::null ) continue;

    if ( has_collision( Cmp::RectBounds( { rnd_pos.position }, gridsize, 1 ) ) ) continue;
    auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "RUIN.cobweb" } );
    Factory::create_cobweb( getReg(), rnd_pos.position, m_sprite_factory.get_multisprite_by_type( ms ), idx );
  }
}

template <typename COMPONENT>
void RuinSystem::add_stairs( sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms, float zorder )
{
  auto stairs_entt = getReg().create();
  Cmp::Position stairs_pos( spawn_position, stairs_ms.getSpriteSizePixels() );
  getReg().emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
  Factory::create_multiblock<COMPONENT>( getReg(), stairs_entt, stairs_pos, stairs_ms );
  Factory::create_multiblock_segments<COMPONENT, Cmp::RuinStairsSegment>( getReg(), stairs_entt, stairs_pos, stairs_ms );

  for ( auto [stairs_entt, stairs_cmp, stairs_zorder] : getReg().view<COMPONENT, Cmp::ZOrderValue>().each() )
  {
    stairs_zorder.setZOrder( zorder );
  }
}
// Explicit function template instantiations
template void RuinSystem::add_stairs<Cmp::RuinStairsLowerMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );
template void RuinSystem::add_stairs<Cmp::RuinStairsUpperMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );
template void RuinSystem::add_stairs<Cmp::RuinStairsBalustradeMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );

void RuinSystem::creaking_rope_update()
{
  static float m_creaking_rope_swing_freq{ 0.075f }; // oscillations per second
  auto t = m_creaking_rope_swing_timer.getElapsedTime().asSeconds();

  float stereo_pan_value = std::sin( t * m_creaking_rope_swing_freq * std::numbers::pi );

  // SPDLOG_INFO( "creaking_rope_update: {}", stereo_pan_value );
  m_sound_bank.get_music( "ruin_creaking_rope" ).setPan( stereo_pan_value );
}

bool RuinSystem::check_activate_player_curse( sf::Vector2f scene_dimensions )
{
  Cmp::PlayerCurse &player_curse = Utils::Player::get_player_curse( getReg() );

  if ( not player_curse.active && is_player_carrying_witches_jar() )
  {
    if ( not m_curse_activation_future.valid() )
    {
      m_sound_bank.get_music( "ruin_creaking_rope" ).stop();
      m_sound_bank.get_music( "ruin_music" ).stop();

      if ( m_sound_bank.get_effect( "witch_scream" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_effect( "witch_scream" ).play(); }
      if ( m_sound_bank.get_effect( "banging_smashing_sounds" ).getStatus() != sf::Sound::Status::Playing )
      {
        m_sound_bank.get_effect( "banging_smashing_sounds" ).play();
      }
      player_curse.active = true; // prevent the active curse from being re-activated
      const auto &hand_ms = m_sprite_factory.get_multisprite_by_type( "RUIN.shadow_hand" );
      Factory::create_shadow_hand( m_reg, scene_dimensions, hand_ms );
      m_curse_activation_future = std::async( std::launch::async, []() { std::this_thread::sleep_for( std::chrono::seconds( 5 ) ); } );
    }

    // Check each frame if the async delay has completed
    if ( m_curse_activation_future.valid() && m_curse_activation_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready )
    {
      // do something at the end of the curse activation
    }
  }

  return player_curse.active;
}

bool RuinSystem::is_player_carrying_witches_jar()
{
  bool result = false;
  for ( auto [inv_entt, inv_cmp] : getReg().view<Cmp::PlayerInventorySlot>().each() )
  {
    if ( inv_cmp.type == "CARRYITEM.witchesjar" ) { result = true; }
  }
  return result;
}

void RuinSystem::update_shadow_hand_pos( sf::Vector2f scene_dimensions )
{
  if ( not Utils::Player::get_player_curse( getReg() ).active ) return;

  const auto &hand_ms = m_sprite_factory.get_multisprite_by_type( "RUIN.shadow_hand" );
  const auto hand_ms_size = hand_ms.getSpriteSizePixels();
  float max_shadow_hand_xpos = scene_dimensions.x - hand_ms_size.x;

  float shadow_hand_speed = 0.45f;

  for ( auto [hand_entt, hand_cmp, hand_pos] : getReg().view<Cmp::RuinShadowHand, Cmp::Position>().each() )
  {
    if ( hand_pos.position.x + shadow_hand_speed < max_shadow_hand_xpos ) { hand_pos.position.x += shadow_hand_speed; }
  }
}

void RuinSystem::check_player_shadow_hand_collision()
{
  // only trigger PlayerMortalityEvents if player is alive
  if ( Utils::Player::get_player_mortality( getReg() ).state == Cmp::PlayerMortality::State::DEAD ) { return; }

  auto &player_health = Utils::Player::get_player_health( getReg() );
  const auto player_pos = Utils::Player::get_player_position( getReg() );
  if ( Utils::Collision::check_cmp<Cmp::RuinShadowHand>( getReg(), Cmp::RectBounds( player_pos.position, Constants::kGridSizePxF, 1.f ) ) )
  {
    // damage player
    player_health.health -= 1.f;
  }
  if ( player_health.health <= 0.f )
  {
    get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SHADOWCURSED, player_pos ) );
  }
}

void RuinSystem::reset_player_curse()
{
  Utils::Player::reset_player_curse( getReg() );
  m_curse_activation_future = std::future<void>{}; // allow re-entrant scene to trigger the future again
}

void RuinSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto holywelldoor_view = getReg().view<Cmp::HolyWellExit, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, holywelldoor_cmp, holywell_door_pos_cmp] : holywelldoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), holywell_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds( holywell_door_pos_cmp.position, holywell_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_exit_collision: Player exiting holywell to graveyard at position ({}, {})", pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_HOLYWELL );

      reset_player_curse();
    }
  }
}

void RuinSystem::check_create_witch( entt::registry &reg, sf::FloatRect scene_dimensions )
{

  auto has_collision = [&]( Cmp::RectBounds pos )
  {
    if ( Utils::Collision::check_cmp<Cmp::RuinBookcase>( reg, pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::NpcNoPathFinding>( reg, pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::RuinStairsSegment>( reg, pos ) ) { return true; }

    // ensure bookcase is inside scene
    if ( not Cmp::RectBounds( pos.position(), pos.size(), 1.5 ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  bool witch_exists = false;
  for ( auto [npc_entt, npc_cmp, npc_sprite_cmp] : reg.view<Cmp::NPC, Cmp::SpriteAnimation>().each() )
  {
    if ( npc_sprite_cmp.m_sprite_type == "NPCWITCH" ) { witch_exists = true; }
  }
  if ( not witch_exists )
  {

    for ( auto _ : std::views::iota( 0, 100 ) )
    {
      auto [rnd_entt, rnd_pos_cmp] = Utils::Rnd::get_random_position( reg, {}, {} );
      if ( has_collision( Cmp::RectBounds( rnd_pos_cmp.position, rnd_pos_cmp.size, 1 ) ) ) continue;

      auto new_witch_entity = reg.create();
      Cmp::Position position_cmp = reg.emplace<Cmp::Position>( new_witch_entity, rnd_pos_cmp.position, rnd_pos_cmp.size );
      [[maybe_unused]] Cmp::ZOrderValue zorder_cmp = reg.emplace<Cmp::ZOrderValue>( new_witch_entity, position_cmp.position.y );
      Factory::create_npc( reg, new_witch_entity, "NPCWITCH" );
      break;
    }
  }
}

} // namespace ProceduralMaze::Sys