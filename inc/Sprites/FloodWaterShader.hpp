#ifndef __SPRITES_FLOOD_WATER_SHADER_HPP__
#define __SPRITES_FLOOD_WATER_SHADER_HPP__

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Settings.hpp>
#include <string>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites {

class FloodWaterShader : public sf::Drawable, public sf::Transformable {
public:
    FloodWaterShader(std::string shader_file) 
    {
        m_waterTexture.clear(sf::Color(64, 128, 255));
        m_waterTexture.display();
        
        m_waterSprite.setTexture(m_waterTexture.getTexture());

        if( !m_waterShader.loadFromFile(shader_file, sf::Shader::Type::Fragment) ) {
            SPDLOG_CRITICAL("Failed to load water shader");
            std::get_terminate();
        }
        m_waterShader.setUniform("resolution", sf::Vector2f(Settings::DISPLAY_SIZE));
    }

    ~FloodWaterShader() {};

    void update(float waterLevel) {
        m_waterShader.setUniform("time", m_waterClock.getElapsedTime().asSeconds());
        m_waterShader.setUniform("waterLevel", waterLevel);
        m_waterSprite.setPosition({0, waterLevel});
    }

    void draw(sf::RenderTarget& target, [[maybe_unused]] sf::RenderStates states) const override {
        target.draw(m_waterSprite, &m_waterShader);
    }

private:

    sf::RenderTexture m_waterTexture{Settings::DISPLAY_SIZE};
    sf::Sprite m_waterSprite{m_waterTexture.getTexture()};
    sf::Shader m_waterShader;
    sf::Clock m_waterClock;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_FLOOD_WATER_SHADER_HPP__