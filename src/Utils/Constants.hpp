#ifndef SRC_UTILS_CONSTANTS_HPP__
#define SRC_UTILS_CONSTANTS_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Constants
{

//! @brief Size in pixels of a single grid cell, as unsigned integer components.
inline constexpr sf::Vector2u kGridSizePx{ 16u, 16u };

//! @brief Size in pixels of a single grid cell, as floating-point components.
inline static constexpr sf::Vector2f kGridSizePxF{ 16.f, 16.f };

//! @brief Display resolution used when the actual display size cannot be determined.
inline static constexpr sf::Vector2u kFallbackDisplaySize{ 1920, 1080 };

//! @brief the directory containing the game resources files
inline static std::filesystem::path res_dir( "res" );

} // namespace Game::Constants

#endif // SRC_UTILS_CONSTANTS_HPP__
