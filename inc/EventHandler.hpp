#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <Components/System.hpp>
#include <Direction.hpp>
#include <GameState.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <optional>

#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>

#include <spdlog/spdlog.h>

#include <queue>

namespace ProceduralMaze {

class InputEventHandler {
public:
  InputEventHandler(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
      : m_reg(reg) {}

  enum class GameActions { DROP_BOMB };

  void menu_state_handler(sf::RenderWindow &window) {
    auto gamestate_view = m_reg->view<Cmp::GameState>();
    for (auto [entity, game_state] : gamestate_view.each()) {
      using namespace sf::Keyboard;
      while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          game_state.current_state = Cmp::GameState::State::EXITING;
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
          sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
          window.setView(sf::View(visibleArea));
        } else if (const auto *keyPressed =
                       event->getIf<sf::Event::KeyPressed>()) {
          if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
            game_state.current_state = Cmp::GameState::State::LOADING;
          } else if (keyPressed->scancode == sf::Keyboard::Scancode::Q) {
            game_state.current_state = Cmp::GameState::State::EXITING;
          }
        }
      }
    }
  }

  void game_state_handler(sf::RenderWindow &window) {
    auto gamestate_view = m_reg->view<Cmp::GameState>();
    for (auto [entity, game_state] : gamestate_view.each()) {
      using namespace sf::Keyboard;
      while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          game_state.current_state = Cmp::GameState::State::EXITING;
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
          sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
          window.setView(sf::View(visibleArea));
        } else if (const auto *keyReleased =
                       event->getIf<sf::Event::KeyReleased>()) {
          if (keyReleased->scancode == sf::Keyboard::Scancode::F1) {
            for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each()) {
              _sys.collisions_enabled = not _sys.collisions_enabled;
              SPDLOG_INFO("Collisions are now {}",
                          _sys.collisions_enabled ? "ENABLED" : "DISABLED");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::F2) {
            for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each()) {
              _sys.pause_flood = not _sys.pause_flood;
              SPDLOG_INFO("Pause flood is now {}",
                          _sys.pause_flood ? "ENABLED" : "DISABLED");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::F3) {
            for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each()) {
              _sys.show_path_distances = not _sys.show_path_distances;
              SPDLOG_INFO("Show Dijkstra distance is now {}",
                          _sys.show_path_distances ? "ENABLED" : "DISABLED");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::F4) {
            for (auto [_entt, _sys] : m_reg->view<Cmp::System>().each()) {
              _sys.show_armed_obstacles = not _sys.show_armed_obstacles;
              SPDLOG_INFO("Show armed obstacles is now {}",
                          _sys.show_armed_obstacles ? "ENABLED" : "DISABLED");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::F11) {
            for (auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each()) {
              _pc.alive = false;
              SPDLOG_INFO("Player committed suicide");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::F12) {
            for (auto [_ent, _sys] : m_reg->view<Cmp::System>().each()) {
              _sys.level_complete = true;
              SPDLOG_INFO("Level complete (player cheated)");
            }
          } else if (keyReleased->scancode == sf::Keyboard::Scancode::Escape) {
            game_state.current_state = Cmp::GameState::State::UNLOADING;
          }

        } else if (const auto *keyPressed =
                       event->getIf<sf::Event::KeyPressed>()) {

          if (keyPressed->scancode == sf::Keyboard::Scancode::P) {
            using namespace std::chrono_literals;
            game_state.current_state = Cmp::GameState::State::PAUSED;
            SPDLOG_INFO("Pausing game....");
            std::this_thread::sleep_for(200ms);
          }
        }
      }

      // allow multiple changes to the direction vector, otherwise we get a
      // delayed slurred movement
      auto player_direction_view =
          m_reg->view<Cmp::PlayableCharacter, Cmp::Direction>();
      for (auto [entity, player, direction] : player_direction_view.each()) {
        direction.x = 0;
        direction.y = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
          direction.y = -1;
        } // move player up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
          direction.x = -1;
        } // move player left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
          direction.x = 1;
        } // move player right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
          direction.y = 1;
        } // move player down
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        m_action_queue.push(GameActions::DROP_BOMB);
      }
    }
  }

  void paused_state_handler(sf::RenderWindow &window) {
    auto gamestate_view = m_reg->view<Cmp::GameState>();
    for (auto [entity, game_state] : gamestate_view.each()) {
      using namespace sf::Keyboard;
      while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          game_state.current_state = Cmp::GameState::State::EXITING;
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
          sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
          window.setView(sf::View(visibleArea));
        } else if (const auto *keyPressed =
                       event->getIf<sf::Event::KeyPressed>()) {
          if (keyPressed->scancode == sf::Keyboard::Scancode::P) {
            game_state.current_state = Cmp::GameState::State::PLAYING;
          }
        }
      }
    }
  }

  void game_over_state_handler(sf::RenderWindow &window) {
    auto gamestate_view = m_reg->view<Cmp::GameState>();
    for (auto [entity, game_state] : gamestate_view.each()) {
      using namespace sf::Keyboard;
      while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          game_state.current_state = Cmp::GameState::State::EXITING;
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
          sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
          window.setView(sf::View(visibleArea));
        } else if (const auto *keyPressed =
                       event->getIf<sf::Event::KeyPressed>()) {
          if (keyPressed->scancode == sf::Keyboard::Scancode::R) {
            game_state.current_state = Cmp::GameState::State::UNLOADING;
          }
        }
      }
    }
  }

  entt::reactive_mixin<entt::storage<void>> m_gamestate_updates;
  std::queue<GameActions> m_action_queue{};

private:
  std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
};

} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__