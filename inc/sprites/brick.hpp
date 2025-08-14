#ifndef __SPRITES_BRICK_HPP__
#define __SPRITES_BRICK_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace ProceduralMaze::Sprites {

class Brick : public sf::RectangleShape {
public:

    Brick(sf::Vector2f pos) 
    : 
        sf::RectangleShape(sf::Vector2f(WIDTH, HEIGHT)) 
    {
        setPosition(pos);
        setFillColor(sf::Color{245, 73, 39});
        setOutlineColor(sf::Color{254, 243, 198});
        setOutlineThickness(3);
    };

    constexpr static const float WIDTH{32};
    constexpr static const float HALFWIDTH{WIDTH/2};
    constexpr static const float HEIGHT{32};
    constexpr static const float HALFHEIGHT{HEIGHT/2};
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BRICK_HPP__