#ifndef SRC_FACTORY_SHADERFACTORY_HPP_
#define SRC_FACTORY_SHADERFACTORY_HPP_

#include <Persistent/DisplayResolution.hpp>
#include <Shaders/DarkModeShader.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/PulsingShader.hpp>
#include <Systems/ShaderSystem.hpp>
#include <ZOrderValue.hpp>

namespace ProceduralMaze::Factory::Shader
{

void add_title( Sys::ShaderSystem &shader_sys, const Cmp::Persist::DisplayResolution &display_res );
void add_mist( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_water( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_pulse( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_dark( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );
void add_curse( Sys::ShaderSystem &shader_sys, sf::Vector2f map_size_pixel );

} // namespace ProceduralMaze::Factory::Shader

#endif // SRC_FACTORY_SHADERFACTORY_HPP_