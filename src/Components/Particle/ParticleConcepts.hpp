#ifndef SRC_CMPS_PARTICLE_PARTICLECONCEPTS_HPP_
#define SRC_CMPS_PARTICLE_PARTICLECONCEPTS_HPP_

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <concepts>
#include <type_traits>

namespace ProceduralMaze::Cmp::Particle
{

class IParticle; // forward declare — avoid circular include

//! @brief Enforces that TParticle inherits from IParticle
template <typename TParticle>
concept HasParticleMembers = requires( TParticle p ) {
  { p.m_vertex } -> std::same_as<sf::Vertex &>;
  { p.m_velocity } -> std::same_as<sf::Vector2f &>;
  { p.m_lifetime } -> std::same_as<sf::Time &>;
};

template <typename TParticle>
concept HasPrivateEmit = !requires( TParticle p, sf::Vector2f v, sf::Time t ) {
  p.emit( v, t ); // emit() must NOT be publicly accessible — keep it private (NVI idiom)
};

template <typename TParticle>
concept ParticleConcept = std::derived_from<TParticle, IParticle> && HasParticleMembers<TParticle> && HasPrivateEmit<TParticle>;

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMPS_PARTICLE_PARTICLECONCEPTS_HPP_