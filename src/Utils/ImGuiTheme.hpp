#ifndef SRC_UTILS_IMGUITHEME_HPP__
#define SRC_UTILS_IMGUITHEME_HPP__

namespace Game::Utils
{

//! @brief Apply the game's custom ImGui look (colors + shape) to the current ImGui context.
//! @note Must be called after ImGui::SFML::Init, since that creates the ImGui context and
//! resets the style to ImGui's default before this can override it.
void apply_imgui_theme();

} // namespace Game::Utils

#endif // SRC_UTILS_IMGUITHEME_HPP__
