#include <Components/Crypt/CryptShuffleTimer.hpp>
#include <Utils/Crypt.hpp>

namespace Game::Utils::Crypt
{

void restart_crypt_shuffle_timer( entt::registry &reg )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    timer_cmp.restart();
  }
}

void stop_crypt_shuffle_timer( entt::registry &reg )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    timer_cmp.reset();
  }
}

bool is_crypt_shuffle_timer_expired( entt::registry &reg )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    if ( timer_cmp.getElapsedTime() < timer_cmp.m_timeout ) return false;
  }
  return true;
}

} // namespace Game::Utils::Crypt