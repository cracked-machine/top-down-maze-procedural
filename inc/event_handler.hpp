#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include <optional>

#include <components/pc.hpp>
#include <components/position.hpp>

#include <spdlog/spdlog.h>

namespace ProceduralMaze {

class InputEventHandler
{
public:
    InputEventHandler() = default;
    
    void handler(const std::shared_ptr<sf::RenderWindow> window, 
        entt::basic_registry<entt::entity> &m_reg)
    {
      
        using namespace sf::Keyboard;
        using namespace Components;
        while (const std::optional event = window->pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { window->close(); }
        }

        // move player up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            for( auto [ _entt, _, _current_pos] : 
                m_reg.view<PlayableCharacter, Position>().each() )
            {
                m_reg.patch<Position>(_entt, [&](auto &_pos) { _pos.y -= 1; });
            }
        }
        // move player left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            for( auto [ _entt, _, _current_pos] : 
                m_reg.view<PlayableCharacter, Position>().each() )
            {
                m_reg.patch<Position>(_entt, [&](auto &_pos) { _pos.x -= 1; });
            }
        }
        // move player right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            for( auto [ _entt, _, _current_pos] : 
                m_reg.view<PlayableCharacter, Position>().each() )
            {
                m_reg.patch<Position>(_entt, [&](auto &_pos) { _pos.x += 1; });
            }
        }
        // move player down
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            for( auto [ _entt, _, _current_pos] : 
                m_reg.view<PlayableCharacter, Position>().each() )
            {
                m_reg.patch<Position>(_entt, [&](auto &_pos) { _pos.y += 1; });
            }
        }
    }
};


} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__