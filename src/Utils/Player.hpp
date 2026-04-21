#ifndef SRC_UTILS_PLAYER_HPP_
#define SRC_UTILS_PLAYER_HPP_

#include <SpriteAnimation.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Stats/PlayerStats.hpp>

namespace ProceduralMaze::Cmp
{
class Position;
class RectBounds;
class PlayerHealth;
class PlayerWealth;
class PlayerBlastRadius;
class PlayerMortality;
class PlayerLastGraveyardPosition;
class ZOrderValue;
class PlayerCurse;
class Direction;
class PlayerCadaverCount;
class PlayerLevelDepth;
} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::Utils::Player
{

entt::entity get_entity( entt::registry &reg );
Cmp::Position &get_position( entt::registry &reg );
Cmp::Direction &get_direction( entt::registry &reg );
Cmp::SpriteAnimation &get_sprite_anim( entt::registry &reg );
int get_ruin_location( entt::registry &reg ); // Returns enum as int to avoid include
Cmp::PlayerLastGraveyardPosition *get_last_graveyard_pos( entt::registry &reg );
Cmp::PlayerLevelDepth &get_level_depth( entt::registry &reg );
Cmp::PlayerWealth &get_wealth( entt::registry &reg );
Cmp::PlayerBlastRadius &get_blast_radius( entt::registry &reg );
Cmp::PlayerMortality &get_mortality( entt::registry &reg );
Cmp::ZOrderValue &get_zorder( entt::registry &reg );
Cmp::PlayerCurse &get_curse( entt::registry &reg );
void reset_curse( entt::registry &reg );
float get_speed_penalty( entt::registry &reg );
void remove_lerp_cmp( entt::registry &reg );
std::pair<entt::entity, Sprites::SpriteMetaType> get_inventory_type( entt::registry &reg );
float get_inventory_wear_level( entt::registry &reg );
void reduce_inventory_wear_level( entt::registry &reg, float amount );
bool is_in_spawn( entt::registry &reg, const Cmp::Position &player_pos_cmp );
Cmp::PlayerCadaverCount &get_cadaver_count( entt::registry &reg );
Cmp::PlayerStats &get_player_stats( entt::registry &reg );

} // namespace ProceduralMaze::Utils::Player

#endif // SRC_UTILS_PLAYER_HPP_