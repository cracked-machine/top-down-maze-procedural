#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <components/system.hpp>
#include <optional>

#include <components/pc.hpp>
#include <components/position.hpp>

#include <spdlog/spdlog.h>
#include <xbb.hpp>
#include <ybb.hpp>

namespace ProceduralMaze {

class InputEventHandler
{
public:
    InputEventHandler() = default;

    // move player/hit boxes to new position on key event
    void handler(
        const std::shared_ptr<sf::RenderWindow> window, 
        entt::basic_registry<entt::entity> &m_reg
    )
    {
      
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { window->close(); }
            else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyReleased->scancode == sf::Keyboard::Scancode::M)
                {
                    SPDLOG_INFO("Pressed M");
                    for( auto [ _entt, _sys] :
                        m_reg.view<Cmp::System>().each() )
                    {
                        _sys.local_view = not _sys.local_view;
                        SPDLOG_INFO("Set _sys.local_view to {}", _sys.local_view);
                    }
                }
            }
        }

        // move player up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            for( auto [ _entt, _pc, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                _current_pos.y -= Cmp::PlayableCharacter::MOVE_DIST; 
                _xbb.position.y -= Cmp::PlayableCharacter::MOVE_DIST;
                _ybb.position.y -= Cmp::PlayableCharacter::MOVE_DIST;
            }
        }
        // move player left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            for( auto [ _entt, _pc, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                _current_pos.x -= Cmp::PlayableCharacter::MOVE_DIST; 
                _xbb.position.x -= Cmp::PlayableCharacter::MOVE_DIST;
                _ybb.position.x -= Cmp::PlayableCharacter::MOVE_DIST;
            }
        }
        // move player right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            for( auto [ _entt, _pc, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                _current_pos.x += Cmp::PlayableCharacter::MOVE_DIST; 
                _xbb.position.x += Cmp::PlayableCharacter::MOVE_DIST;
                _ybb.position.x += Cmp::PlayableCharacter::MOVE_DIST;
            }
        }
        // move player down
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            for( auto [ _entt, _pc, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                _current_pos.y += Cmp::PlayableCharacter::MOVE_DIST; 
                _xbb.position.y += Cmp::PlayableCharacter::MOVE_DIST;
                _ybb.position.y += Cmp::PlayableCharacter::MOVE_DIST;                    
            }
        }
    }
};

} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__