#ifndef SCR_UTILS_CRYPT_HPP_
#define SCR_UTILS_CRYPT_HPP_

namespace Game::Utils::Crypt
{

void restart_crypt_shuffle_timer( entt::registry &reg );
void stop_crypt_shuffle_timer( entt::registry &reg );
bool is_crypt_shuffle_timer_expired( entt::registry &reg );

} // namespace Game::Utils::Crypt

#endif // SCR_UTILS_CRYPT_HPP_