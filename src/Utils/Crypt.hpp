#ifndef SCR_UTILS_CRYPT_HPP_
#define SCR_UTILS_CRYPT_HPP_

namespace Game::Crypt::Utils
{

void restart_crypt_shuffle_timer( entt::registry &reg );
void stop_crypt_shuffle_timer( entt::registry &reg );
bool is_crypt_shuffle_timer_expired( entt::registry &reg );

} // namespace Game::Crypt::Utils

#endif // SCR_UTILS_CRYPT_HPP_