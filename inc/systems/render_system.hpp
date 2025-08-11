#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <font.hpp>
#include <memory>
#include <position.hpp>
#include <systems/base_system.hpp>
#include <spdlog/spdlog.h>

namespace RENAME_THIS_NAMESPACE::Systems {

class RenderSystem : public Systems::BaseSystem {
public:
    RenderSystem(std::shared_ptr<sf::RenderWindow> win) : m_window(win) 
    { SPDLOG_DEBUG("RenderSystem()"); }
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 

    // this listens for changes to Position component
    void update_cb( entt::registry &reg, entt::entity entt )
    {
        SPDLOG_DEBUG("RenderSystem::update_cb()");
    
            // we have a font entity!!
            if( reg.all_of<Components::Font>( entt ) )
            {
                sf::Text text( reg.get<Components::Font>(entt), "Hello" );
                text.setPosition( reg.get<Components::Position>(entt) );
                text.setCharacterSize( 24 );  
                text.setFillColor( sf::Color::Red );
                text.setStyle( sf::Text::Bold | sf::Text::Underlined );
                m_window->draw( text );
            }
            // a non-font entity
            else
            {
                sf::CircleShape dot( 20 );
                dot.setPosition( reg.get<Components::Position>(entt) );         
                m_window->draw( dot );       
            }
        
    }
private:
    std::shared_ptr<sf::RenderWindow> m_window;
};

} // namespace RENAME_THIS_NAMESPACE::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__