#ifndef SRC_EVENTS_PARTICLEEVENTS_HPP__
#define SRC_EVENTS_PARTICLEEVENTS_HPP__

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

#endif // SRC_EVENTS_PARTICLEEVENTS_HPP__
