#ifndef __CMP_RECTBOUNDS_HPP__
#define __CMP_RECTBOUNDS_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <windows.h>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp {

class RectBounds {
public:
    RectBounds(sf::Vector2f position, sf::Vector2f size, float ScaleFactor)
    : m_scale_factor(ScaleFactor)
    {
        m_bounds.size = size * m_scale_factor;
        m_bounds.position = position - sf::Vector2f{Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE} * POSITION_OFFSET;
    }
    void position(sf::Vector2f new_position)
    {
        m_bounds.position = new_position - sf::Vector2f{Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE} * POSITION_OFFSET;
    }
    void size(sf::Vector2f new_size) { m_bounds.size = new_size * m_scale_factor; }
    sf::Vector2f position() { return m_bounds.position; }
    sf::Vector2f size() { return m_bounds.size; }
    std::optional<sf::FloatRect> findIntersection(const sf::FloatRect& rectangle)
    {
        return m_bounds.findIntersection(rectangle);
    }
    sf::FloatRect getBounds() const { return m_bounds; }
private:
    sf::FloatRect m_bounds;
    float m_scale_factor;
    const float POSITION_OFFSET{(m_scale_factor / 2.f) - 0.5f};

    
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_RECTBOUNDS_HPP__
