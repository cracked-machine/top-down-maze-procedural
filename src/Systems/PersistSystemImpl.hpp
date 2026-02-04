#ifndef SRC_SYSTEMS_PERSISTENT_SYSTEM_IMPL_HPP
#define SRC_SYSTEMS_PERSISTENT_SYSTEM_IMPL_HPP

#include "PersistSystem.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <typeinfo>

namespace ProceduralMaze::Sys
{

template <typename T>
void PersistSystem::add_persist_cmp( entt::registry &reg )
{
  if ( not reg.ctx().contains<T>() ) { reg.ctx().emplace<T>(); }
}

template <typename T, typename... Args>
void PersistSystem::add_persist_cmp( entt::registry &reg, Args &&...args )
{
  if ( not reg.ctx().contains<T>() ) { reg.ctx().emplace<T>( std::forward<Args>( args )... ); }
}

template <typename T>
T &PersistSystem::get_persist_cmp( entt::registry &reg )
{
  if ( not reg.ctx().contains<T>() )
  {
    SPDLOG_CRITICAL( "Attempting to access non-existent persistent component: {}", typeid( T ).name() );
    throw std::runtime_error( "Persistent component not found: " + std::string( typeid( T ).name() ) );
  }
  return reg.ctx().get<T>();
}

template <typename T>
void PersistSystem::registerTypes( const std::string &name )
{
  // Register loader
  m_component_loaders[name] = [this]( const nlohmann::json &j )
  {
    auto &cmp = Sys::PersistSystem::get_persist_cmp<T>( getReg() );
    cmp.deserialize( j );
  };

  // Register serializer
  m_component_serializers[name] = [this]() -> nlohmann::json { return Sys::PersistSystem::get_persist_cmp<T>( getReg() ).serialize(); };

  // Only add to widget list if T inherits from IBasePersistent
  if constexpr ( std::is_base_of_v<Cmp::Persist::IBasePersistent, T> )
  {
    m_registered_components.push_back( &Sys::PersistSystem::get_persist_cmp<T>( getReg() ) );
  }
}

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PERSISTENT_SYSTEM_IMPL_HPP