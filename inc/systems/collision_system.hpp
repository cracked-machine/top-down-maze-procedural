#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>


#include <collision.hpp>
#include <obstacle.hpp>
#include <pc.hpp>
#include <position.hpp>
#include <spdlog/spdlog.h>


#include <sprites/brick.hpp>
#include <sprites/player.hpp>
#include <systems/base_system.hpp>

namespace ProceduralMaze::Systems {

class CollisionSystem : public BaseSystem {
public:
    CollisionSystem() {}
    ~CollisionSystem() = default;

     entt::reactive_mixin<entt::storage<void>> m_position_updates;

    void check()
    {
        using namespace Components;
        for( auto [entity, _pc,  _pc_pos]: 
            m_position_updates.view<PlayableCharacter, Position>().each() ) {
        
            for( auto [entity, _ob,  _ob_pos]: 
                m_position_updates.view<Obstacle, Position>().each() ) {

                auto has_collision = 
                    Sprites::Player(_pc_pos).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                
                while( has_collision ) 
                {
                    if (has_collision) SPDLOG_INFO("Collision at {},{}", has_collision->position.x, has_collision->position.y);
          
                    m_position_updates.registry().emplace_or_replace<Collision>(entity, has_collision);

                    auto playerCenter = sf::FloatRect({_pc_pos.x, _pc_pos.y}, {Sprites::Player::WIDTH, Sprites::Player::HEIGHT}).getCenter();
                    auto brickCenter = sf::FloatRect({_ob_pos.x, _ob_pos.y}, {Sprites::Brick::WIDTH, Sprites::Brick::HEIGHT}).getCenter();
                    auto diffX = playerCenter.x - brickCenter.x;
                    auto diffY = playerCenter.y - brickCenter.y;
                    auto minXDist = Sprites::Player::HALFWIDTH + Sprites::Brick::HALFWIDTH;
                    auto minYDist = Sprites::Player::HALFHEIGHT + Sprites::Brick::HALFHEIGHT;
                    auto depthX = diffX > 0 ? minXDist - diffX : -minXDist - diffX;
                    auto depthY = diffY > 0 ? minYDist - diffY : -minYDist - diffY;

                    if (depthX != 0 && depthY != 0) {
                        if (abs(depthX) < abs(depthY)) {
                            // Collision along the X axis. React accordingly
                            if (depthX > 0) {
                                SPDLOG_INFO("left side collision");
                                has_collision = Sprites::Player({_pc_pos.x -= 1, _pc_pos.y} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                            } 
                            else {
                                SPDLOG_INFO("right side collision");
                                has_collision = Sprites::Player({_pc_pos.x += 1, _pc_pos.y} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds()); 
                            }
                        } 
                        else {
                            // Collision along the Y axis.
                            if (depthY > 0) {
                                SPDLOG_INFO("top side collision");
                                has_collision = Sprites::Player({_pc_pos.x, _pc_pos.y -= 1} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                            } else {
                                SPDLOG_INFO("bottom side collision");
                                has_collision = Sprites::Player({_pc_pos.x, _pc_pos.y += 1} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                            }
                        }
                    }
               
                    if (has_collision) SPDLOG_INFO("FAILED: {},{}", _pc_pos.x, _pc_pos.y);
                    else SPDLOG_INFO("SUCCESS: {},{}", _pc_pos.x, _pc_pos.y);
                }
                
            }
        }
    }   
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__