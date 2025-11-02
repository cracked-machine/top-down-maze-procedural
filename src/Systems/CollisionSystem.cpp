#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Window.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/Destructable.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/LargeObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDamageDelay.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/ShrineCost.hpp>
#include <Components/Persistent/WaterBonus.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WeaponLevel.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

CollisionSystem::CollisionSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                                  Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( reg, window, sprite_factory )
{

  SPDLOG_DEBUG( "CollisionSystem initialized" );
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

void CollisionSystem::check_loot_collision()
{
  // Store both loot effects and the player velocities
  struct LootEffect
  {
    entt::entity loot_entity;
    Sprites::SpriteMetaType type;
    entt::entity player_entity;
  };

  std::vector<LootEffect> loot_effects;

  // First pass: detect collisions and gather effects to apply
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::WeaponLevel,
                                           Cmp::PlayerKeysCount, Cmp::PlayerCandlesCount>();
  auto loot_collision_view = m_reg->view<Cmp::Loot, Cmp::Position>();

  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_weapon_level, pc_keys_count, pc_candles_count] :
        player_collision_view.each() )
  {
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp] : loot_collision_view.each() )
    {
      if ( not is_visible_in_view( RenderSystem::getGameView(), loot_pos_cmp ) ) continue;

      if ( pc_pos_cmp.findIntersection( loot_pos_cmp ) )
      {
        // Store effect to apply after collision detection
        loot_effects.push_back( { loot_entt, loot_cmp.m_type, pc_entt } );
      }
    }
  }

  // Second pass: apply effects and remove loots
  for ( const auto &effect : loot_effects )
  {
    if ( !m_reg->valid( effect.player_entity ) ) continue;

    auto &pc_cmp = m_reg->get<Cmp::PlayableCharacter>( effect.player_entity );
    auto &weapon_level_cmp = m_reg->get<Cmp::WeaponLevel>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
      pc_cmp.health = std::min( pc_cmp.health + health_bonus.get_value(), 100 );
      m_get_loot_sound_player.play();
    }
    else if ( effect.type == "EXTRA_BOMBS" )
    {
      auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
      if ( pc_cmp.bomb_inventory >= 0 )
      {
        pc_cmp.bomb_inventory += bomb_bonus.get_value();
        m_get_loot_sound_player.play();
      }
    }
    else if ( effect.type == "LOWER_WATER" )
    {
      auto &water_bonus = get_persistent_component<Cmp::Persistent::WaterBonus>();
      for ( auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each() )
      {
        water_level.m_level = std::min( water_level.m_level + water_bonus.get_value(),
                                        static_cast<float>( kDisplaySize.y ) );
        m_get_loot_sound_player.play();
      }
    }

    else if ( effect.type == "INFINI_BOMBS" ) { pc_cmp.bomb_inventory = -1; }
    else if ( effect.type == "CHAIN_BOMBS" ) { pc_cmp.blast_radius = std::clamp( pc_cmp.blast_radius + 1, 0, 3 ); }
    else if ( effect.type == "WEAPON_BOOST" )
    {
      // increase weapon level by 50, up to max level 100
      weapon_level_cmp.m_level = std::clamp( weapon_level_cmp.m_level + 50.f, 0.f, 100.f );
      m_get_loot_sound_player.play();
    }
    else if ( effect.type == "CANDLE_DROP" )
    {
      auto &pc_candles_count = m_reg->get<Cmp::PlayerCandlesCount>( effect.player_entity );
      pc_candles_count.increment_count( 1 );
      m_get_loot_sound_player.play();
    }
    else if ( effect.type == "KEY_DROP" )
    {
      auto &pc_keys_count = m_reg->get<Cmp::PlayerKeysCount>( effect.player_entity );
      pc_keys_count.increment_count( 1 );
      m_get_key_sound_player.play();
    }
    else if ( effect.type == "RELIC_DROP" )
    {
      auto &pc_relic_count = m_reg->get<Cmp::PlayerRelicCount>( effect.player_entity );
      pc_relic_count.increment_count( 1 );
      m_get_loot_sound_player.play();
    }
    else
    {
      SPDLOG_WARN( "Unknown loot type encountered during pickup: {}", effect.type );
      continue;
    }

    // Remove the loot component
    m_reg->remove<Cmp::Loot>( effect.loot_entity );
    m_reg->emplace_or_replace<Cmp::Destructable>( effect.loot_entity );
  }
}

} // namespace ProceduralMaze::Sys