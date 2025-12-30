#include <Components/GraveMultiBlock.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpawnArea.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

GraveSystem::GraveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&GraveSystem::on_player_action>( this );
}

void GraveSystem::check_player_collision( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerCandlesCount, Cmp::PlayerKeysCount>();
  auto grave_view = getReg().view<Cmp::GraveMultiBlock>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_candles_cmp, pc_keys_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF, 1.5f );

    for ( auto [grave_entity, grave_cmp] : grave_view.each() )
    {
      if ( player_hitbox.findIntersection( grave_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with Grave at ({}, {})", grave_cmp.position.x, grave_cmp.position.y );
        check_player_grave_activation( grave_entity, grave_cmp, pc_cmp );
      }
    }
  }
}

void GraveSystem::check_player_grave_activation( entt::entity &grave_entity, Cmp::GraveMultiBlock &grave_cmp, Cmp::PlayableCharacter &pc_cmp )
{
  if ( grave_cmp.are_powers_active() ) return;
  if ( grave_cmp.get_activation_count() < grave_cmp.get_activation_threshold() )
  {

    SPDLOG_DEBUG( "Activating grave sprite {}/{}.", grave_cmp.get_activated_sprite_count() + 1, grave_cmp.get_activation_threshold() );
    grave_cmp.set_activation_count( grave_cmp.get_activation_threshold() );

    auto anim_cmp = getReg().try_get<Cmp::SpriteAnimation>( grave_entity );
    if ( anim_cmp )
    {
      if ( std::string::size_type n = anim_cmp->m_sprite_type.find( "." ); n != std::string::npos )
      {
        anim_cmp->m_sprite_type = anim_cmp->m_sprite_type.substr( 0, n ) + ".opened";
        SPDLOG_DEBUG( "Grave Cmp::SpriteAnimation changed to opened type: {}", anim_cmp->m_sprite_type );
      }
    }

    grave_cmp.set_powers_active( true );

    // choose a random consequence for activating graves: spawn npc, drop bomb, give candles
    if ( grave_cmp.are_powers_active() )
    {
      auto grave_activation_rng = Cmp::RandomInt( 1, 3 );
      auto consequence = grave_activation_rng.gen();
      switch ( consequence )
      {
        case 1:
          SPDLOG_DEBUG( "Grave activated NPC trap." );
          Factory::createNPC( getReg(), grave_entity, "NPCGHOST" );
          m_sound_bank.get_effect( "spawn_ghost" ).play();
          break;
        case 2:
          SPDLOG_DEBUG( "Grave activated bomb trap." );
          pc_cmp.bomb_inventory += 1;
          get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::GRAVE_BOMB ) );
          break;
        case 3:

          auto grave_cmp_bounds = Cmp::RectBounds( grave_cmp.position, grave_cmp.size, 2.f );
          // clang-format off
              auto obst_entity = Factory::createLootDrop(getReg(), 
                Cmp::SpriteAnimation{ 0,0,true,"CANDLE_DROP", 0 },
                sf::FloatRect{ grave_cmp_bounds.position(), 
                grave_cmp_bounds.size() }, 
                Factory::IncludePack<>{},
                Factory::ExcludePack<Cmp::PlayableCharacter, Cmp::GraveSegment, Cmp::SpawnArea>{}
              );
          // clang-format on

          if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
          break;
      }
      // }
    }
  }
}

} // namespace ProceduralMaze::Sys