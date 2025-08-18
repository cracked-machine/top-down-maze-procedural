#ifndef __ISPRITE_HPP__
#define __ISPRITE_HPP__

#include <SFML/Graphics/RectangleShape.hpp>


namespace ProceduralMaze::Sprites {

class ISprite : public sf::RectangleShape{
public:
    ISprite(sf::Vector2f pos, sf::Vector2f size) :
        sf::RectangleShape(size)
    {
        setPosition(pos);
    }    
    virtual ~ISprite() = default;

    sf::FloatRect getCollisionBounds() const {
        // Always return the intended collision area, not affected by outline thickness
        return sf::FloatRect(getPosition(), getSize());
    }

};

} // namespace ProceduralMaze::Sprites


#endif // __ISPRITE_HPP__