#ifndef __COMPONENTS_POSITION_HPP__
#define __COMPONENTS_POSITION_HPP__

#include <SFML/System/Vector2.hpp>
#include <base.hpp>

namespace RENAME_THIS_NAMESPACE::Components {

class Position : public sf::Vector2f, Components::Base {
public:
    Position(const sf::Vector2f& pos) : sf::Vector2f(pos) { SPDLOG_DEBUG("Position()"); }
    ~Position() { SPDLOG_DEBUG("~Position()"); }

    Position& operator=(const sf::Vector2f& other)
    {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }
};

} // namespace RENAME_THIS_NAMESPACE::Components

#endif // __COMPONENTS_POSITION_HPP__