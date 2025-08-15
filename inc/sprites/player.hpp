#ifndef __SPRITES_PLAYER_HPP__
#define __SPRITES_PLAYER_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sprites {

class Player : public sf::RectangleShape {
public:

    Player(sf::Vector2f pos) 
    : 
        sf::RectangleShape(sf::Vector2f(WIDTH, HEIGHT))
    {
        setPosition(pos);
        setFillColor(sf::Color::White);
    }

    constexpr static const float WIDTH{32};
    constexpr static const float HALFWIDTH{WIDTH/2};
    constexpr static const float HEIGHT{32};
    constexpr static const float HALFHEIGHT{HEIGHT/2};
    constexpr static const sf::Vector2f SIZE{WIDTH, HEIGHT};


private:
    

};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_PLAYER_HPP__