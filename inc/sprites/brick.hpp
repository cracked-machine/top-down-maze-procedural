#ifndef __SPRITES_BRICK_HPP__
#define __SPRITES_BRICK_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace ProceduralMaze::Sprites {

// brick graphic
class Brick : public sf::RectangleShape {
public:

    Brick(sf::Vector2f pos) 
    : 
        sf::RectangleShape(sf::Vector2f(WIDTH, HEIGHT)) 
    {
        setPosition(pos);
        setFillColor(FILLCOLOUR);
        setOutlineColor(LINECOLOUR);
        setOutlineThickness(LINEWIDTH);
    };

    // so we can query the attributes without having to 
    // store each brick geometry before they are drawn
    constexpr static const float WIDTH{40};
    constexpr static const float HALFWIDTH{WIDTH/2};
    constexpr static const float HEIGHT{40};
    constexpr static const float HALFHEIGHT{HEIGHT/2};
    constexpr static const sf::Vector2f SIZE{WIDTH, HEIGHT};
    constexpr static const float LINEWIDTH{3};
    constexpr static const sf::Color FILLCOLOUR{245, 73, 39};
    constexpr static const sf::Color LINECOLOUR{254, 243, 198};
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BRICK_HPP__