#ifndef SRC_SCENE_SCENESTACK_HPP_
#define SRC_SCENE_SCENESTACK_HPP_

#include <Scene/IScene.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

namespace ProceduralMaze::Scene
{

class SceneStack
{
public:
  SceneStack() = default;
  ~SceneStack() = default;

  // Non-copyable, moveable
  SceneStack( const SceneStack & ) = delete;
  SceneStack &operator=( const SceneStack & ) = delete;
  SceneStack( SceneStack && ) = default;
  SceneStack &operator=( SceneStack && ) = default;

  // Stack operations
  void push( std::unique_ptr<IScene> scene ) { m_stack.push_back( std::move( scene ) ); }

  void pop()
  {
    if ( m_stack.empty() ) { throw std::runtime_error( "SceneStack::pop: Cannot pop from empty stack" ); }
    m_stack.pop_back();
  }

  // Safe access to current scene
  IScene &current()
  {
    if ( m_stack.empty() ) { throw std::runtime_error( "SceneStack::current: No current scene available" ); }
    return *m_stack.back();
  }

  const IScene &current() const
  {
    if ( m_stack.empty() ) { throw std::runtime_error( "SceneStack::current: No current scene available" ); }
    return *m_stack.back();
  }

  // Utility methods
  bool empty() const noexcept { return m_stack.empty(); }

  std::size_t size() const noexcept { return m_stack.size(); }

  // Safe access to previous scene (for overlays)
  IScene *previous() { return ( m_stack.size() >= 2 ) ? m_stack[m_stack.size() - 2].get() : nullptr; }

  const IScene *previous() const { return ( m_stack.size() >= 2 ) ? m_stack[m_stack.size() - 2].get() : nullptr; }

  // Iterator access for debugging/logging (read-only)
  auto begin() const { return m_stack.begin(); }
  auto end() const { return m_stack.end(); }

  void print_stack()
  {
    std::stringstream ss;
    for ( std::size_t i = 0; i < m_stack.size(); ++i )
    {
      auto s = m_stack[i].get();
      ss << "[" << i << "] " << s->get_name() << " ";
    }
    ss << "(top)";
    SPDLOG_INFO( "{}", ss.str() );
  }

private:
  std::vector<std::unique_ptr<IScene>> m_stack;
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENE_SCENESTACK_HPP_