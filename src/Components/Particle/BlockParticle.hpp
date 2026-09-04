#ifndef SRC_COMPONENTS_PARTICLE_BLOCKPARTICLE_HPP__
#define SRC_COMPONENTS_PARTICLE_BLOCKPARTICLE_HPP__

namespace Game::Cmp::Particle
{

//! @brief Mark entity that blocks particles (see ParticleSystem::check_collsion). Attached alongside
//! Cmp::Obstacle and Cmp::Player::NoPath wherever those are attached, except for Cmp::Altar::Segment.
struct BlockParticle
{
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_BLOCKPARTICLE_HPP__
