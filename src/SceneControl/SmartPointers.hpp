#ifndef SRC_SCENECONTROL_SMARTPOINTERS_HPP__
#define SRC_SCENECONTROL_SMARTPOINTERS_HPP__

namespace Game::Scene
{
class SceneData;

using SceneMapSharedPtr = std::shared_ptr<SceneData>;
using SceneMapWeakPtr = std::weak_ptr<SceneData>;
} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SMARTPOINTERS_HPP__
