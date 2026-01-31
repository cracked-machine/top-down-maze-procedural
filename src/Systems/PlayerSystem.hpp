#ifndef SRC_SYSTEMS_PLAYERSYSTEM_HPP__
#define SRC_SYSTEMS_PLAYERSYSTEM_HPP__

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys
{

class PlayerSystem : public BaseSystem
{
public:
  enum class FootStepSfx { NONE, GRAVEL, FLOORBOARDS };
  PlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Update the player system.
  //! @note This enqueues 'Events::SceneManagerEvent::Type::GAME_OVER' if player is dead
  void update( sf::Time globalDeltaTime, FootStepSfx footstep_sfx = FootStepSfx::GRAVEL );

  void playFootstepsSound( FootStepSfx type );
  void stopFootstepsSound();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Rotation, scaling, offset, alpha, etc
  void localTransforms();

  //! @brief Position updates, lerping, etc
  //! @param globalDeltaTime
  //! @param collision_detection disabled if false, enabled if true
  void globalTranslations( sf::Time globalDeltaTime, bool collision_detection );

  //! @brief Refresh the PlayerDistance components for positions near the player
  void refreshPlayerDistances();

  //! @brief Check if the player is mortal or dead
  void checkPlayerMortality();

  void checkShockwavePlayerCollision( Cmp::NpcShockwave &shockwave );
  void check_player_axe_npc_kill();

  void on_player_mortality_event( ProceduralMaze::Events::PlayerMortalityEvent ev );
  void on_player_action_event( ProceduralMaze::Events::PlayerActionEvent ev );

  // Cooldown clock to manage attack intervals
  // sf::Clock m_attack_cooldown_clock;

  //! @brief Use this to send events to the scene manager
  entt::dispatcher &m_scenemanager_event_dispatcher;

  sf::Clock m_debug_info_timer;

  sf::Clock m_post_death_timer;

  sf::Clock m_inventory_cooldown_timer;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PLAYERSYSTEM_HPP__
