#ifndef SRC_EVENTS_PARTICLEEVENTS_HPP__
#define SRC_EVENTS_PARTICLEEVENTS_HPP__

namespace Game::Events
{

//! @brief Requests that the particle sprite(s) matching a tag stop emitting.
struct StopParticleSpriteEvent
{
  //! @brief Tag identifying which particle sprite(s) to stop.
  std::string tag;
};

//! @brief Requests that the particle sprite(s) matching a tag resume/restart emitting.
struct RestartParticleSpriteEvent
{
  //! @brief Tag identifying which particle sprite(s) to restart.
  std::string tag;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PARTICLEEVENTS_HPP__
