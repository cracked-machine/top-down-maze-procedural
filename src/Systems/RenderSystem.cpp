#include <RenderSystem.hpp>

namespace ProceduralMaze::Sys {

std::unique_ptr<sf::RenderWindow> RenderSystem::m_window =
    std::make_unique<sf::RenderWindow>( sf::VideoMode( DISPLAY_SIZE ), "ProceduralMaze" );

} // namespace ProceduralMaze::Sys
