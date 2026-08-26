#ifndef SRC_SCENECONTROL_SCENES_GRAVEYARDSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_GRAVEYARDSCENE_HPP__

#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/VertexFloor.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/System/Time.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

//! @brief The graveyard hub scene: the main procedurally generated exterior level that connects to the
//! crypt, ruins, healing spring, and shop, and hosts NPCs, hazards, plants, and the grimoire overlay.
class GraveyardScene : public Scene<Events::ProcessGraveyardSceneInputEvent>
{
public:
  //! @brief Construct a new GraveyardScene object
  //! @param sound_bank Shared sound bank used to play/stop graveyard-related audio
  //! @param system_store Store of game systems the scene drives each update
  //! @param nav_event_dispatcher Used to notify the SceneInputRouter when new input events are available
  //! @param sprite_Factory Factory used to create sprites for entities spawned in this scene
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
                  Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_factory( sprite_Factory )
  {
  }

  //! @brief Procedurally generates the graveyard level (obstacles, plants, hazards, NPCs, loot, navmeshes,
  //! floor tiles) and initializes persistent state
  void on_init() override;

  //! @brief Restores the player to their last graveyard position (or start position on first spawn),
  //! starts graveyard music, and resets the scene-exit cooldown
  void on_enter() override;

  //! @brief Clears the registry, stops graveyard music/footstep audio, and clears floor tiles
  void on_exit() override;

  //! @brief Get the name of the scene
  //! @return std::string "GraveyardScene"
  [[nodiscard]] std::string get_name() const override { return "GraveyardScene"; }

  //! @brief Get the registry object owned by the scene
  //! @return entt::registry&
  entt::registry &registry() override;

protected:
  //! @brief Advances all graveyard-related systems (hazards, NPCs, wisps, watchman, wormholes, altar,
  //! healing spring, ruin/crypt entrances, lightning, grimoire, arrows, particles) for one frame and renders the game
  //! @param dt The time elapsed since the last update
  void do_update( sf::Time dt ) override;

private:
  //! @brief Shared sound bank used to play/stop graveyard-related audio
  Audio::SoundBank &m_sound_bank;

  //! @brief Store of game systems the scene drives each update
  Sys::Store &m_sys;

  //! @brief Factory used to create sprites for entities spawned in this scene
  Sprites::SpriteFactory &m_sprite_factory;

  //! @brief Clock measuring elapsed time since the scene was last entered, used to gate exit collision checks
  sf::Clock m_scene_exit_cooldown{};

  //! @brief Minimum time after entering the scene before exit collisions (e.g. crypt entrance) are checked
  sf::Time m_scene_exit_cooldown_time{ sf::seconds( 2 ) };

  //! @brief Whether the grimoire overlay UI is currently open
  bool m_grimoire_overlay_open{ false };
  //! @brief Re-wires the freshly (re)created navmeshes into the systems that depend on them
  void reinit_navmesh();

  //! @brief spatial map for reserving positions during procedural generation
  PathFinding::SpatialHashGridSharedPtr m_reserved_sm;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_GRAVEYARDSCENE_HPP__
