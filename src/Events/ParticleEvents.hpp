#ifndef SRC_EVENTS_PARTICLEEVENTS_HPP_
#define SRC_EVENTS_PARTICLEEVENTS_HPP_

namespace Game::Events
{

struct StopParticleSpriteEvent
{
  std::string tag;
};

struct RestartParticleSpriteEvent
{
  std::string tag;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PARTICLEEVENTS_HPP_