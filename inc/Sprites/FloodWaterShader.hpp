#ifndef __SPRITES_FLOODERWATERSHADER_HPP__
#define __SPRITES_FLOODERWATERSHADER_HPP__

#include <Settings.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites {

class FloodWaterShader : public BaseFragmentShader {
public:
    FloodWaterShader() : BaseFragmentShader("res/FloodWater2.glsl", Settings::DISPLAY_SIZE) {
        setup();
        SPDLOG_INFO("FloodWaterShader initialized");
    }
    ~FloodWaterShader() override = default;

    void pre_setup_texture() override {
        m_texture.clear(sf::Color(64, 128, 255));
        std::ignore = m_texture.resize(Settings::DISPLAY_SIZE);
    }

    void post_setup_shader() override {
        m_shader.setUniform("resolution", sf::Vector2f(Settings::DISPLAY_SIZE));
    }

    void update(float waterLevel) {
        m_shader.setUniform("time", m_clock.getElapsedTime().asSeconds());
        m_shader.setUniform("waterLevel", waterLevel);
        m_sprite.setPosition({0, waterLevel});
    }
    void update() override { /* unused */ }
};


} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_FLOODERWATERSHADER_HPP__