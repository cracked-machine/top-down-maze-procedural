#ifndef SRC_SCENECONTROL_SMARTPOINTERS_HPP__
#define SRC_SCENECONTROL_SMARTPOINTERS_HPP__

namespace Game::Scene
{
//! @brief Deserialized Tiled JSON scene data (tilesets, tile layers, player start position).
class SceneData;

//! @brief Shared pointer to a Game::Scene::SceneData.
using SceneMapSharedPtr = std::shared_ptr<SceneData>;
//! @brief Weak pointer to a Game::Scene::SceneData.
using SceneMapWeakPtr = std::weak_ptr<SceneData>;
} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SMARTPOINTERS_HPP__
