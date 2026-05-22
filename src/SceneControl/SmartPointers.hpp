#ifndef SRC_SCENECONTROL_SMARTPOITNER_HPP_
#define SRC_SCENECONTROL_SMARTPOITNER_HPP_

namespace Game::Scene
{
class SceneData;

using SceneMapSharedPtr = std::shared_ptr<SceneData>;
using SceneMapWeakPtr = std::weak_ptr<SceneData>;
} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SMARTPOITNER_HPP_