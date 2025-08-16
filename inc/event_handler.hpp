#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
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
    const float move_delta{1.f};

    void handler(
        const std::shared_ptr<sf::RenderWindow> window, 
        entt::basic_registry<entt::entity> &m_reg
    )
    {
      
        using namespace sf::Keyboard;
        while (const std::optional event = window->pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { window->close(); }
        }

        // move player up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            for( auto [ _entt, _, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                m_reg.patch<Cmp::Position>(_entt, [&](auto &_pos) { 
                    _pos.y -= move_delta; 
                    _xbb.position.y -= move_delta;
                    _ybb.position.y -= move_delta;
                });
            }
        }
        // move player left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            for( auto [ _entt, _, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                m_reg.patch<Cmp::Position>(_entt, [&](auto &_pos) { 
                    _pos.x -= move_delta; 
                    _xbb.position.x -= move_delta;
                    _ybb.position.x -= move_delta;
                });
            }
        }
        // move player right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            for( auto [ _entt, _, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                m_reg.patch<Cmp::Position>(_entt, [&](auto &_pos) { 
                    _pos.x += move_delta; 
                    _xbb.position.x += move_delta;
                    _ybb.position.x += move_delta;
                });
            }
        }
        // move player down
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            for( auto [ _entt, _, _current_pos, _xbb, _ybb] : 
                m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
            {
                m_reg.patch<Cmp::Position>(_entt, [&](auto &_pos) { 
                    _pos.y += move_delta; 
                    _xbb.position.y += move_delta;
                    _ybb.position.y += move_delta;                    
                });
            }
        }
    }
};


} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__