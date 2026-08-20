#ifndef SRC_SYSTEMS_GRAVESYSTEM_HPP__
#define SRC_SYSTEMS_GRAVESYSTEM_HPP__

#include <Components/Grave/MultiBlock.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>

#include <vector>

// clang-format off
namespace Game::Cmp { class AnimData; }
namespace Game::Cmp::Player { class Character; }
// clang-format on

namespace Game::Sys
{

class GraveSystem : public BaseSystem
{
public:
  GraveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief handler for Events::PlayerActionEvent::GameActions::DIG
  //! @param event
  void on_player_action( const Events::PlayerActionEvent &event );

  //! @brief Use Cmp::SelectedPosition to mark targetted graves and destroy them when appropriate tool is equipped.
  void update();

  //! @brief Random consequence rolled when a grave is fully dug open
  enum class GraveConsequence { NPC_TRAP = 1, BOMB_TRAP, RELIC, JEWELRY };

  //! @brief True if the player has a digging tool equipped with remaining wear
  bool has_digging_tool_equipped();

  //! @brief True while the player's per-entity dig cooldown has not yet elapsed
  bool is_dig_on_cooldown();

  //! @brief Remove Cmp::SelectedPosition from grave entities now that the cooldown has expired
  void clear_stale_grave_selections();

  //! @brief True if any Cmp::Player::Character entity's hitbox intersects the grave
  bool is_player_near( const Cmp::Grave::MultiBlock &grave_cmp );

  //! @brief Apply one dig hit: reduce tool wear and grave hp, then play the hit sound or open the grave
  void apply_dig_hit( entt::entity grave_entity, Cmp::Grave::MultiBlock &grave_cmp, Cmp::AnimData &grave_anim_cmp );

  //! @brief Switch the grave's sprite to its opened state and trigger a random consequence
  void open_grave( entt::entity grave_entity, Cmp::AnimData &grave_anim_cmp );

  //! @brief Randomly spawn an NPC trap, a bomb trap, a relic, or jewelry from the opened grave
  void trigger_grave_consequence( entt::entity grave_entity );

  //! @brief Pick a random item from the pool, drop it as loot, and apply its Cmp::SpawnAction modifiers to the player
  void spawn_grave_loot( const std::vector<Sprites::SpriteMetaType> &loot_pool );
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_GRAVESYSTEM_HPP__
