#ifndef SRC_SCENECONTROL_EVENTS_PROCESSGAMEOVERSCENEINPUTEVENT_HPP__
#define SRC_SCENECONTROL_EVENTS_PROCESSGAMEOVERSCENEINPUTEVENT_HPP__

namespace Game::Events
{

//! @brief Per-frame marker enqueued by the active game-over scene to tell SceneInputRouter to process its input.
struct ProcessGameoverSceneInputEvent
{
};

} // namespace Game::Events

#endif // SRC_SCENECONTROL_EVENTS_PROCESSGAMEOVERSCENEINPUTEVENT_HPP__
