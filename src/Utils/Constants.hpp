#ifndef SRC_UTILS_CONSTANTS_HPP_
#define SRC_UTILS_CONSTANTS_HPP_

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Constants
{

inline constexpr sf::Vector2u kGridSizePx{ 16u, 16u };
inline static constexpr sf::Vector2f kGridSizePxF{ 16.f, 16.f };
inline static constexpr sf::Vector2u kFallbackDisplaySize{ 1920, 1080 };

} // namespace ProceduralMaze::Constants

#endif //  SRC_UTILS_CONSTANTS_HPP_