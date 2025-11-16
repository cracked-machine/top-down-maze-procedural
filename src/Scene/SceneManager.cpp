#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

void SceneManager::push( std::unique_ptr<IScene> scene )
{
  if ( !m_scenes.empty() ) m_scenes.back()->on_exit();

  m_scenes.push_back( std::move( scene ) );
  m_scenes.back()->on_enter();

  inject_registry();
}

void SceneManager::pop()
{
  m_scenes.back()->on_exit();
  m_scenes.pop_back();

  if ( !m_scenes.empty() ) m_scenes.back()->on_enter();

  inject_registry();
}

IScene *SceneManager::current() { return m_scenes.empty() ? nullptr : m_scenes.back().get(); }

void SceneManager::update( float dt )
{
  if ( current() ) current()->update( dt );
}

// PRIVATE FUNCTIONS

void SceneManager::inject_registry()
{
  auto *scene = current();
  entt::registry *reg = scene ? scene->get_registry() : nullptr;

  for ( auto *sys : m_reg_inject_system_ptrs )
    sys->setRegistry( reg );
}

} // namespace ProceduralMaze::Scene