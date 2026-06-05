#ifndef SRC_FACTORY_PARTICLEFACTORY_HPP_
#define SRC_FACTORY_PARTICLEFACTORY_HPP_

#include <Components/ZOrderValue.hpp>
#include <Particle/Flame.hpp>
#include <Particle/ParticleSpriteTest.hpp>
#include <Particle/ShockWave.hpp>
#include <Particle/Smoke.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/Stores/SystemStore.hpp>
#include <Utils/Player.hpp>

#include <entt/entity/fwd.hpp>

namespace Game::Particle::Factory
{

void add_test( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag );
void add_flame( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );
void add_flame_for_player_inventory_slot( entt::registry &reg );
void add_smoke( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );
void add_shockwave( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder );

void delete_expired_particle_sprites( entt::registry &reg, const std::string &search_pattern );
void update_position( entt::registry &reg, const std::string &search_pattern, sf::Vector2f pos );
void update_position( entt::registry &reg, Cmp::UUID uuid_cmp, sf::Vector2f pos );

} // namespace Game::Particle::Factory

#endif // SRC_FACTORY_PARTICLEFACTORY_HPP_