#ifndef SRC_UTILS_PLAYER_HPP_
#define SRC_UTILS_PLAYER_HPP_

#include <Sprites/SpriteMetaType.hpp>

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
} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::Utils::Player
{

entt::entity get_player_entity( entt::registry &reg );
Cmp::Position &get_player_position( entt::registry &reg );
int get_player_ruin_location( entt::registry &reg ); // Returns enum as int to avoid include
Cmp::PlayerLastGraveyardPosition *get_player_last_graveyard_position( entt::registry &reg );
Cmp::PlayerHealth &get_player_health( entt::registry &reg );
Cmp::PlayerWealth &get_player_wealth( entt::registry &reg );
Cmp::PlayerBlastRadius &get_player_blast_radius( entt::registry &reg );
Cmp::PlayerMortality &get_player_mortality( entt::registry &reg );
Cmp::ZOrderValue &get_player_zorder( entt::registry &reg );
Cmp::PlayerCurse &get_player_curse( entt::registry &reg );
float get_player_speed_penalty( entt::registry &reg );
void remove_player_lerp_cmp( entt::registry &reg );
std::pair<entt::entity, Sprites::SpriteMetaType> get_player_inventory_type( entt::registry &reg );
float get_player_inventory_wear_level( entt::registry &reg );
void reduce_player_inventory_wear_level( entt::registry &reg, float amount );

} // namespace ProceduralMaze::Utils::Player

#endif // SRC_UTILS_PLAYER_HPP_