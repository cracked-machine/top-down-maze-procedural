#ifndef SRC_FACTORY_SHADERFACTORY_HPP__
#define SRC_FACTORY_SHADERFACTORY_HPP__

#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/ZOrderValue.hpp>
#include <Shaders/DarkModeShader.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/NightStaticShader.hpp>
#include <Systems/ShaderSystem.hpp>

namespace Game::Factory::Shader
{

void add_title( Sys::ShaderSystem &shader_sys, const Cmp::Persist::DisplayResolution &display_res );
void add_mist( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_water( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_night_static( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_dark( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_curse( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );

} // namespace Game::Factory::Shader

#endif // SRC_FACTORY_SHADERFACTORY_HPP__
