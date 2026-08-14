#ifndef SRC_COMPONENTS_CURRENTSCENE_HPP__
#define SRC_COMPONENTS_CURRENTSCENE_HPP__

namespace Game::Cmp
{

enum class SceneId
{
  GRAVEYARD,
  SHOP,
  HEALING_SPRING,
  RUIN_UPPER_FLOOR,
  RUIN_LOWER_FLOOR,
  CRYPT,
  TITLE,
  PAUSED_MENU,
  SETTINGS_MENU,
  GAME_OVER,
  LEVEL_COMPLETE
};

struct CurrentScene
{
  SceneId id;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CURRENTSCENE_HPP__
