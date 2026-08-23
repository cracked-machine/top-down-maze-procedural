#ifndef SRC_COMPONENTS_PARTICLE_CONCEPTS_HPP__
#define SRC_COMPONENTS_PARTICLE_CONCEPTS_HPP__

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <concepts>
#include <type_traits>

namespace Game::Cmp::Particle
{

//! @brief Forward declaration of the particle base interface.
//! @note Forward declared (rather than included) to avoid a circular include between this header
//!       and the header that defines IParticle.
class IParticle;

//! @brief Enforces that TParticle inherits from IParticle
template <typename TParticle>
concept HasParticleMembers = requires( TParticle p ) {
  { p.m_vertex } -> std::same_as<sf::Vertex &>;
  { p.m_velocity } -> std::same_as<sf::Vector2f &>;
  { p.m_lifetime } -> std::same_as<sf::Time &>;
};

//! @brief Enforces that TParticle::emit() is NOT publicly callable.
//! @note emit() must be kept private and only invoked through IParticle::do_emit() (the Non-Virtual
//!       Interface idiom), so this concept is satisfied only when calling p.emit(v, t) from outside
//!       the class would fail to compile.
template <typename TParticle>
concept HasPrivateEmit = !requires( TParticle p, sf::Vector2f v, sf::Time t ) {
  p.emit( v, t );
};

//! @brief Enforces the full contract a particle type must satisfy to be used with SpriteBase.
//! @tparam TParticle The concrete particle type under test.
template <typename TParticle>
concept ParticleConcept = std::derived_from<TParticle, IParticle> && HasParticleMembers<TParticle> && HasPrivateEmit<TParticle>;

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_CONCEPTS_HPP__
