#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>


#include <collision.hpp>
#include <obstacle.hpp>
#include <pc.hpp>
#include <position.hpp>
#include <spdlog/spdlog.h>


#include <sprites/brick.hpp>
#include <sprites/player.hpp>
#include <systems/base_system.hpp>
#include <xbb.hpp>
#include <ybb.hpp>

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem {
public:
    CollisionSystem() {}
    ~CollisionSystem() = default;

     entt::reactive_mixin<entt::storage<void>> m_position_updates;

    sf::Vector2f getCenter(sf::Vector2f pos, sf::Vector2f size)
    {
        return sf::FloatRect(pos, size).getCenter();
    }

    void check_xbb()
    {
        for( auto [_pc_entt, _pc,  _pc_pos, _xbb, _ybb]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
        {

            for( auto [_ob_entt, _ob,  _ob_pos]: 
                m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) 
            {
                if( not _ob.m_enabled ) { continue; }

                auto has_collision = 
                    _xbb.findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                
                while( has_collision ) 
                {
                    
                    auto brickCenter = getCenter(_ob_pos, Sprites::Brick::SIZE);
                    auto diffX = _xbb.getCenter().x - brickCenter.x;
                    auto minXDist = (_xbb.size.x / 2) + Sprites::Brick::HALFWIDTH;
                    auto depthX = diffX > 0 ? minXDist - diffX : -minXDist - diffX;
                    
                    if (depthX >= 0) {
                        SPDLOG_INFO("left side collision");
                        has_collision = Sprites::Player({_pc_pos.x -= 1, _pc_pos.y} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                        _xbb.position.x -= 1;
                        _ybb.position.x -= 1;                        
                    } 
                    else {
                        SPDLOG_INFO("right side collision");
                        has_collision = Sprites::Player({_pc_pos.x += 1, _pc_pos.y} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds()); 
                        _xbb.position.x += 1;
                        _ybb.position.x += 1;
                    }
                }
            }   
                
        }
    }

    void check_ybb()
    {
        for( auto [_pc_entt, _pc,  _pc_pos, _xbb, _ybb]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
        {

            for( auto [_ob_entt, _ob,  _ob_pos]: 
                m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) 
            {
                if( not _ob.m_enabled ) { continue; }
                
                auto has_collision = 
                    _ybb.findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                
                while( has_collision ) 
                {
                    
                    auto brickCenter = getCenter(_ob_pos, Sprites::Brick::SIZE);
                    auto diffY = _ybb.getCenter().y - brickCenter.y;
                    auto minYDist = (_ybb.size.y / 2) + Sprites::Brick::HALFHEIGHT;
                    auto depthY = diffY > 0 ? minYDist - diffY : -minYDist - diffY;

                    // Collision along the Y axis.
                    if (depthY >= 0) {
                        SPDLOG_INFO("top side collision");
                        has_collision = Sprites::Player({_pc_pos.x, _pc_pos.y -= 1} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                        _xbb.position.y -= 1;
                        _ybb.position.y -= 1; 
                    } else {
                        SPDLOG_INFO("bottom side collision");
                        has_collision = Sprites::Player({_pc_pos.x, _pc_pos.y += 1} ).getGlobalBounds().findIntersection(Sprites::Brick(_ob_pos).getGlobalBounds());
                        _xbb.position.y += 1;
                        _ybb.position.y += 1; 
                    }
                
                }
            }   
                
        }
    }

    void check()
    {
        check_xbb();
        check_ybb();
    }
    


};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__