#ifndef SRC_CMPS_PLAYER_ALTARACTIVATIONCOOLDOWN_HPP__
#define SRC_CMPS_PLAYER_ALTARACTIVATIONCOOLDOWN_HPP__

namespace Game::Cmp::Player
{

//! @brief Added to the player entity when altar activation cooldown is in effect.
struct AltarActivationCooldown : public sf::Clock
{
};

} // namespace Game::Cmp::Player

#endif // SRC_CMPS_PLAYER_ALTARACTIVATIONCOOLDOWN_HPP__