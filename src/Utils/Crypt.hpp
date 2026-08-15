#ifndef SRC_UTILS_CRYPT_HPP__
#define SRC_UTILS_CRYPT_HPP__

#include <Components/Crypt/ShuffleTimer.hpp>
namespace Game::Utils::Crypt
{

void reset_crypt_shuffle_timer( entt::registry &reg );
void update_crypt_shuffle_timer( entt::registry &reg, sf::Time dt );
bool is_crypt_shuffle_timer_expired( entt::registry &reg );
Cmp::Crypt::ShuffleTimer &get_shuffle_timer();

} // namespace Game::Utils::Crypt

#endif // SRC_UTILS_CRYPT_HPP__
