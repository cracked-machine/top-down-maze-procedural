#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <Components/System.hpp>
#include <Settings.hpp>
#include <optional>

#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>

#include <spdlog/spdlog.h>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>

#include <queue>

namespace ProceduralMaze {

class InputEventHandler
{
public:
    InputEventHandler() = default;

    enum class GameActions {
        DROP_BOMB
    };

    enum class SystemActions {
        START_GAME,
        PAUSE_GAME,
        RESUME_GAME,
        QUIT_GAME
    };
    
    // set a new direction vector on key press
    void handler(
        const std::shared_ptr<sf::RenderWindow> window, 
        entt::basic_registry<entt::entity> &reg,
        Settings::GameState &game_state
    )
    {
      
        using namespace sf::Keyboard;


        
        switch(game_state)
        {

            case Settings::GameState::MENU:
                menu_state_handler(window);
                break;
            case Settings::GameState::PLAYING:
                game_state_handler(window, reg);
                break;
            case Settings::GameState::PAUSED:
                paused_state_handler(window);
                break;
            case Settings::GameState::GAME_OVER:
                game_over_state_handler(window);
                break;
        }
    }

    void menu_state_handler(
        const std::shared_ptr<sf::RenderWindow> window)
    {
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { window->close(); }
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                window->setView(sf::View(visibleArea));
            }
            // press any key to start (more options can be added later)
            else if (event->is<sf::Event::KeyReleased>())
            {
                m_system_action_queue.push(SystemActions::START_GAME);
            }
        }    
    }

    void game_state_handler(
        const std::shared_ptr<sf::RenderWindow> window, 
        entt::basic_registry<entt::entity> &m_reg)
    {
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            // allow basic window events regardless of the game state
            if (event->is<sf::Event::Closed>()) { window->close(); }
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                window->setView(sf::View(visibleArea));
            }
            else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyReleased->scancode == sf::Keyboard::Scancode::F1)
                {
                    for( auto [ _entt, _sys] : m_reg.view<Cmp::System>().each() )
                    {
                        _sys.collisions_enabled = not _sys.collisions_enabled;
                    }
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::F2)
                {
                    for( auto [ _entt, _sys] :
                        m_reg.view<Cmp::System>().each() )
                    {
                        _sys.show_player_hitboxes = not _sys.show_player_hitboxes;
                    }
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::F3)
                {
                    for( auto [ _entt, _sys] :
                        m_reg.view<Cmp::System>().each() )
                    {
                        _sys.show_obstacle_entity_id = not _sys.show_obstacle_entity_id;
                    }
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::Escape)
                {
                    m_system_action_queue.push(SystemActions::QUIT_GAME);
                }
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
                {
                     m_action_queue.push(GameActions::DROP_BOMB);
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::P)
                {
                    using namespace std::chrono_literals;
                    m_system_action_queue.push(SystemActions::PAUSE_GAME);
                    SPDLOG_INFO("Pausing game....");
                    std::this_thread::sleep_for(200ms);
                }
            }
        }    

        // allow multiple changes to the direction vector, otherwise we get a delayed slurred movement
        sf::Vector2f new_direction{0,0};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { new_direction.y = -1; } // move player up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { new_direction.x = -1; } // move player left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { new_direction.x = 1; } // move player right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { new_direction.y = 1; } // move player down
        m_direction_queue.push(new_direction);
        

    }

    void paused_state_handler(
        const std::shared_ptr<sf::RenderWindow> window)
    {
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            // allow basic window events regardless of the game state
            if (event->is<sf::Event::Closed>()) { window->close(); }
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                window->setView(sf::View(visibleArea));
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::P)
                {
                     m_system_action_queue.push(SystemActions::RESUME_GAME);
                }
            }
        }
    }

    void game_over_state_handler(
        const std::shared_ptr<sf::RenderWindow> window)
    {
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            // allow basic window events regardless of the game state
            if (event->is<sf::Event::Closed>()) { window->close(); }
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                window->setView(sf::View(visibleArea));
            }
            // press any key to start (more options can be added later)
            else if (event->is<sf::Event::KeyReleased>())
            {
                m_system_action_queue.push(SystemActions::QUIT_GAME);
            }
        }           
    }

    std::queue<sf::Vector2f> m_direction_queue{};
    std::queue<GameActions> m_action_queue{};
    std::queue<SystemActions> m_system_action_queue{};
private:
};

} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__