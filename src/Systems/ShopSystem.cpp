#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Shop/ShopExit.hpp>
#include <Components/Wall.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/ShopSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

void ShopSystem::spawn_exit( sf::Vector2u spawn_position )
{
  const auto kGridSizePx = Constants::kGridSizePx;
  const auto kGridSizePxF = Constants::kGridSizePxF;

  sf::Vector2f middle_door_position{ static_cast<float>( spawn_position.x ) * kGridSizePx.x, static_cast<float>( spawn_position.y ) * kGridSizePx.y };

  struct ExitSpriteData
  {
    sf::FloatRect pos;
    std::string ms_name;
    int ms_idx;
  };

  std::array<ExitSpriteData, 3> exit_sprite_data = {
      { { sf::FloatRect{ { middle_door_position.x, middle_door_position.y - kGridSizePx.y }, kGridSizePxF }, "CRYPT.interior_wall", 23 },
        { sf::FloatRect{ { middle_door_position.x, middle_door_position.y }, kGridSizePxF }, "CRYPT.interior_wall", 24 },
        { sf::FloatRect{ { middle_door_position.x, middle_door_position.y + kGridSizePx.y }, kGridSizePxF }, "CRYPT.interior_wall", 20 } } };

  // clear walls for doorway
  for ( auto [entt, wall_cmp, pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
  {
    for ( auto item : exit_sprite_data )
    {
      if ( not item.pos.findIntersection( pos_cmp ) ) continue;
      getReg().destroy( entt );
    }
  }

  // add doorway
  for ( auto item : exit_sprite_data )
  {
    auto new_entt = getReg().create();
    getReg().emplace_or_replace<Cmp::Position>( new_entt, item.pos.position, Constants::kGridSizePxF );
    getReg().emplace_or_replace<Cmp::Exit>( new_entt, false ); // unlocked at start
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( new_entt, 0, 0, true, item.ms_name, item.ms_idx );
    getReg().emplace_or_replace<Cmp::ZOrderValue>( new_entt, item.pos.position.y );
    getReg().emplace_or_replace<Cmp::NpcNoPathFinding>( new_entt );
    getReg().emplace_or_replace<Cmp::ShopExit>( new_entt );
  }

  // SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position.x, spawn_position.y );
  return;
}

void ShopSystem::check_exit_collision()
{
  auto player_pos_cmp = Utils::Player::get_position( getReg() );
  auto door_view = getReg().view<Cmp::ShopExit, Cmp::Position>();

  for ( auto [door_entity, door_cmp, door_pos_cmp] : door_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

    Cmp::RectBounds decreased_entrance_bounds( door_pos_cmp.position, door_pos_cmp.size, 0.1f,
                                               Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

    if ( not player_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

    SPDLOG_INFO( "check_exit_collision: Player exiting Shop to graveyard at position ({}, {})", player_pos_cmp.position.x,
                 player_pos_cmp.position.y );
    m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_HOLYWELL );
  }
}

} // namespace ProceduralMaze::Sys