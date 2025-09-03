#include <Components/PlayableCharacter.hpp>
#include <Settings.hpp>

namespace ProceduralMaze::Cmp {

sf::Vector2f PlayableCharacter::PLAYER_START_POS{ 20, static_cast<float>(Settings::DISPLAY_SIZE.y) / 2 };

} // namespace ProceduralMaze::Cmp
