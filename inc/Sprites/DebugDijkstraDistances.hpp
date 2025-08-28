#ifndef __SPRITES_DEBUG_DIJKSTRA_DISTANCES_HPP__
#define __SPRITES_DEBUG_DIJKSTRA_DISTANCES_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Angle.hpp>
#include <Settings.hpp>


namespace ProceduralMaze::Sprites::Containers {

class DebugDijkstraDistances
{
public:
    DebugDijkstraDistances(const sf::Font& font)
        : m_font(font)
    {

        m_render_texture.setSmooth(false); // Disable smoothing for pixel-perfect text
        m_render_texture.clear(sf::Color::Transparent);
    }

    void clear()
    {
        m_render_texture.clear(sf::Color::Transparent);
    }

    void addDistance(uint32_t distance, const sf::Vector2f& position)
    {
        // Use larger font size (12-16 pixels minimum for readability)
        sf::Text text(m_font, std::to_string(distance), 10);
        text.setFillColor(sf::Color::Black); // White text is more visible
        text.setOutlineColor(sf::Color::White); // Add white outline for contrast
        text.setOutlineThickness(1.0f);
       
        // Position text slightly offset to be centered on the entity
        text.setPosition({position.x, position.y});

        m_render_texture.draw(text);
    }

    sf::Sprite getSprite() 
    {
        m_render_texture.display();
        sf::Sprite sprite(m_render_texture.getTexture());
        return sprite;
    
    }

private:
    sf::RenderTexture m_render_texture{{Settings::DISPLAY_SIZE.x, Settings::DISPLAY_SIZE.y}};
    const sf::Font& m_font;
};

} // namespace ProceduralMaze::Sprites::Containers

#endif // __SPRITES_DEBUG_DIJKSTRA_DISTANCES_HPP__
