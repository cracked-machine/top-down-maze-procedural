#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>


#include <Collision.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <spdlog/spdlog.h>


#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>
#include <Systems/BaseSystem.hpp>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>

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
        int stuck_loop = 0;
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
                    if(stuck_loop > 10 ) 
                    { 
                        _pc_pos = ProceduralMaze::Settings::PLAYER_START_POS;
                        _xbb.position = ProceduralMaze::Settings::PLAYER_START_POS;
                        _ybb.position = ProceduralMaze::Settings::PLAYER_START_POS;
                        return;
                    }
                    auto brickCenter = Sprites::Brick(_ob_pos).getGlobalBounds().getCenter();
                    auto diffX = _xbb.getCenter().x - brickCenter.x;
                    auto minXDist = (_xbb.size.x / 2) + (Sprites::Brick(_ob_pos).getGlobalBounds().size.x / 2);
                    auto depthX = diffX > 0 ? minXDist - diffX : -minXDist - diffX;

                    auto adjustX = 1;
                    if (depthX >= 0) {
                        SPDLOG_INFO("left side collision - depthX: {}, diffX: {}, minXDist: {}", depthX, diffX, minXDist);
                        has_collision = Cmp::Xbb( {_xbb.position.x += adjustX, _xbb.position.y}, _xbb.size ).findIntersection(
                            Sprites::Brick(_ob_pos).getGlobalBounds());
                        _pc_pos.x += adjustX;
                        _ybb.position.x += adjustX;
                        SPDLOG_INFO("new pos:{},{}", _pc_pos.x, _pc_pos.y);
                    } 
                    else 
                    {
                        SPDLOG_INFO("right side collision - depthX: {}, diffX: {}, minXDist: {}", depthX, diffX, minXDist);
                        has_collision = Cmp::Xbb( {_xbb.position.x -= adjustX, _xbb.position.y}, _xbb.size ).findIntersection(
                            Sprites::Brick(_ob_pos).getGlobalBounds());
                        _pc_pos.x -= adjustX;
                        _ybb.position.x -= adjustX;
                        SPDLOG_INFO("new pos:{},{}", _pc_pos.x, _pc_pos.y);
                    }
                    stuck_loop++;
                }
            }   
                
        }
    }

    void check_ybb()
    {
        int stuck_loop = 0;
        for( auto [_pc_entt, _pc,  _pc_pos, _xbb, _ybb]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
        {

            for( auto [_ob_entt, _ob,  _ob_pos]: 
                m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) 
            {
                if( not _ob.m_enabled ) { continue; }
                
                auto has_collision = 
                    _ybb.findIntersection(
                        Sprites::Brick(_ob_pos).getGlobalBounds()
                    );
                
                while( has_collision ) 
                {
                    if(stuck_loop > 10 ) 
                    { 
                        _pc_pos = ProceduralMaze::Settings::PLAYER_START_POS;
                        _xbb.position = ProceduralMaze::Settings::PLAYER_START_POS;
                        _ybb.position = ProceduralMaze::Settings::PLAYER_START_POS;
                        return;
                    }
                    auto brickCenter = Sprites::Brick(_ob_pos).getGlobalBounds().getCenter();
                    auto diffY = _ybb.getCenter().y - brickCenter.y;
                    auto minYDist = (_ybb.size.y / 2) + (Sprites::Brick(_ob_pos).getGlobalBounds().size.y / 2);
                    auto depthY = diffY > 0 ? minYDist - diffY : -minYDist - diffY;

                    if (depthY >= 0) {
                        SPDLOG_INFO("top side collision - depthX: {}, diffX: {}, minXDist: {}", depthY, diffY, minYDist);

                        has_collision = Cmp::Ybb( {_ybb.position.x, _ybb.position.y += abs(depthY)}, _ybb.size ).findIntersection(
                            Sprites::Brick(_ob_pos).getGlobalBounds());
                        
                        _pc_pos.y += abs(depthY);
                        _xbb.position.y += abs(depthY); 
                        SPDLOG_INFO("new pos:{},{}", _pc_pos.x, _pc_pos.y);
                    } 
                    else 
                    {
                        SPDLOG_INFO("bottom side collision - depthX: {}, diffX: {}, minXDist: {}", depthY, diffY, minYDist);

                        has_collision = Cmp::Ybb( {_ybb.position.x, _ybb.position.y -= abs(depthY)}, _ybb.size ).findIntersection(
                            Sprites::Brick(_ob_pos).getGlobalBounds());
                        
                        _pc_pos.y -= abs(depthY);
                        _xbb.position.y -= abs(depthY);
                        SPDLOG_INFO("new pos:{},{}", _pc_pos.x, _pc_pos.y);
                    }
                    stuck_loop++;
                }
            }   
                
        }
    }

    void check()
    {
        check_ybb();
        check_xbb();
    }
    


};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__