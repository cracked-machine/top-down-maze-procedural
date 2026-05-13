#ifndef SRC_SCENECONTROL_SMARTPOITNER_HPP_
#define SRC_SCENECONTROL_SMARTPOITNER_HPP_

namespace ProceduralMaze::Scene
{
class SceneData;

using SceneMapSharedPtr = std::shared_ptr<SceneData>;
using SceneMapWeakPtr = std::weak_ptr<SceneData>;
} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SMARTPOITNER_HPP_