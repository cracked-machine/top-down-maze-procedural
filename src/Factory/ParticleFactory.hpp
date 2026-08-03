#ifndef SRC_FACTORY_PARTICLEFACTORY_HPP__
#define SRC_FACTORY_PARTICLEFACTORY_HPP__

#include <Components/Particle/Flame.hpp>
#include <Components/Particle/ParticleSpriteTest.hpp>
#include <Components/Particle/ShockWave.hpp>
#include <Components/Particle/Smoke.hpp>
#include <Components/ZOrderValue.hpp>
#include <Systems/Stores/SystemStore.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

namespace Game::Factory::Particle
{

void add_test( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag );
void add_crypt_altar_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                         float zorder );
void add_rune_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                  float zorder );
void add_wormhole_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                      float zorder );
void add_obstacledig_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                         float zorder );
void add_flame( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );
void add_flame_for_player_inventory_slot( entt::registry &reg );
void add_smoke( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );
void add_shockwave( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );

void delete_expired_particle_sprites( entt::registry &reg, const std::string &search_pattern );
void update_position( entt::registry &reg, const std::string &search_pattern, sf::Vector2f pos );
void update_position( entt::registry &reg, Cmp::UUID uuid_cmp, sf::Vector2f pos );

} // namespace Game::Factory::Particle

#endif // SRC_FACTORY_PARTICLEFACTORY_HPP__
