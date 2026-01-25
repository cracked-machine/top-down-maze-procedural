#include <Components/RectBounds.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinMultiBlock.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
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

    auto ruin_mb_view = getReg().view<Cmp::RuinMultiBlock>();
    for ( auto [ruin_mb_entity, ruin_mb_cmp] : ruin_mb_view.each() )
    {
      if ( door_pos_cmp.findIntersection( ruin_mb_cmp ) )
      {
        getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y - 16.f );
        SPDLOG_INFO( "check_entrance_collision: Player entering ruin from graveyard at position ({}, {})", player_pos.position.x,
                     player_pos.position.y );
        m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_RUIN );
        Factory::add_player_last_graveyard_pos( getReg(), door_pos_cmp );
      }
      else { getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y + 16.f ); }
    }
  }
}

} // namespace ProceduralMaze::Sys