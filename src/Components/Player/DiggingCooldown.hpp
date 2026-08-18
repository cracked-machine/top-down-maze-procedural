#ifndef SRC_CMPS_PLAYER_DIGGINGCOOLDOWN_HPP__
#define SRC_CMPS_PLAYER_DIGGINGCOOLDOWN_HPP__

namespace Game::Cmp::Player
{

//! @brief Added to the player entity when digging cooldown is in effect.
struct DiggingCooldown : public sf::Clock
{
};

} // namespace Game::Cmp::Player

#endif // SRC_CMPS_PLAYER_DIGGINGCOOLDOWN_HPP__