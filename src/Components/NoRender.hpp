#ifndef SRC_COMPONENTS_NORENDER_HPP__
#define SRC_COMPONENTS_NORENDER_HPP__

namespace Game::Cmp
{

//! @brief Add this component to a renderable entity to disable render
struct NoRender
{
public:
  //! @brief Whether rendering is currently disabled for this entity.
  [[maybe_unused]] bool m_active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_NORENDER_HPP__
