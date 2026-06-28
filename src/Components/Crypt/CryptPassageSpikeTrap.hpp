#ifndef SRC_COMPONENTS_CRYPT_CRYPTPASSAGESPIKETRAP_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTPASSAGESPIKETRAP_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

class CryptPassageSpikeTrap : public sf::Vector2f
{
public:
  CryptPassageSpikeTrap( sf::Vector2f pos, unsigned int passage_id )
      : sf::Vector2f( pos ),
        m_passage_id( passage_id ) {

        };
  CryptPassageSpikeTrap( float x, float y, unsigned int passage_id )
      : sf::Vector2f( x, y ),
        m_passage_id( passage_id ) {

        };

  unsigned int m_passage_id = 0;
  sf::Clock m_cooldown_timer;
  sf::Time m_cooldown_threshold{ sf::seconds( 2 ) };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTPASSAGESPIKETRAP_HPP__
