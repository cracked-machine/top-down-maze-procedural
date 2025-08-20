#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <Components/System.hpp>
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
    
    // set a new direction vector on key press
    void handler(const std::shared_ptr<sf::RenderWindow> window, 
                 entt::basic_registry<entt::entity> &m_reg)
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

    sf::Vector2f pop() { 
        auto d = m_direction_queue.front();
        m_direction_queue.pop();
        return d;
    }

    auto empty() { return m_direction_queue.empty(); }
private:
    std::queue<sf::Vector2f> m_direction_queue{};
};

} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__