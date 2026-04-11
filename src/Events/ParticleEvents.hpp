#ifndef SRC_EVENTS_PARTICLEEVENTS_HPP_
#define SRC_EVENTS_PARTICLEEVENTS_HPP_

namespace ProceduralMaze::Events
{

struct StopParticleSpriteEvent
{
  std::string tag;
};

struct RestartParticleSpriteEvent
{
  std::string tag;
};

} // namespace ProceduralMaze::Events

#endif // SRC_EVENTS_PARTICLEEVENTS_HPP_