#ifndef SRC_UTILS_PLAYER_HPP__
#define SRC_UTILS_PLAYER_HPP__

#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Player/ExtraLife.hpp>
#include <Components/Stats/PlayerStats.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace Game::Cmp
{
class Position;
class RectBounds;
class PlayerHealth;
class ZOrderValue;
class Direction;
class LastDirection;
class PlayerInventorySlot;
class TorchRadius;
} // namespace Game::Cmp

namespace Game::Cmp::Player
{
class Wealth;
class BlastRadius;
class Mortality;
class LastGraveyardPosition;
class Curse;
class CadaverCount;
class LevelDepth;
} // namespace Game::Cmp::Player

namespace Game::Utils::Player
{

entt::entity get_entity( entt::registry &reg );
Cmp::Position &get_position( entt::registry &reg );
Cmp::Direction &get_direction( entt::registry &reg );
Cmp::LastDirection &get_last_direction( entt::registry &reg );
Cmp::AnimData &get_sprite_anim( entt::registry &reg );
int get_ruin_location( entt::registry &reg ); // Returns enum as int to avoid include
Cmp::Player::LastGraveyardPosition *get_last_graveyard_pos( entt::registry &reg );
Cmp::Player::LevelDepth &get_level_depth( entt::registry &reg );
Cmp::Player::Wealth &get_wealth( entt::registry &reg );
Cmp::Player::BlastRadius &get_blast_radius( entt::registry &reg );
Cmp::Player::Mortality &get_mortality( entt::registry &reg );
Cmp::ZOrderValue &get_zorder( entt::registry &reg );
Cmp::AbsoluteAlpha &get_alpha( entt::registry &reg );
Cmp::Player::Curse &get_curse( entt::registry &reg );
void reset_curse( entt::registry &reg );
float get_speed_penalty( entt::registry &reg );
void remove_lerp_cmp( entt::registry &reg );
std::pair<entt::entity, Sprites::SpriteMetaType> get_inventory_type( entt::registry &reg );
float get_inventory_wear_level( entt::registry &reg );
void reduce_inventory_wear_level( entt::registry &reg, float amount );
bool is_in_spawn( entt::registry &reg, const Cmp::Position &player_pos_cmp );
Cmp::Player::CadaverCount &get_cadaver_count( entt::registry &reg );
Cmp::TorchRadius &get_torch_radius( entt::registry &reg );
Cmp::PlayerStats &get_player_stats( entt::registry &reg );
sf::Clock &get_global_bomb_flash_clk( entt::registry &reg );
bool player_has_extra_life( entt::registry &reg );

} // namespace Game::Utils::Player

#endif // SRC_UTILS_PLAYER_HPP__
