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

template <typename ComponentType, typename... DefaultArgTypes>
void PersistSystem::registerComponent( const std::string &key, DefaultArgTypes &&...default_args )
{
  auto args_tuple = std::make_tuple( std::forward<DefaultArgTypes>( default_args )... );

  std::apply( [this]( auto &&...unpacked_args )
              { Sys::PersistSystem::add_persist_cmp<ComponentType>( getReg(), std::forward<decltype( unpacked_args )>( unpacked_args )... ); },
              args_tuple );

  m_component_loaders[key] = [this, args_tuple = std::move( args_tuple )]( const nlohmann::json &persistent_object )
  {
    auto &component = get_persist_cmp<ComponentType>( getReg() );
    component.deserialize( persistent_object );
    auto deserialized_value = component.get_value();
    SPDLOG_DEBUG( "Loaded {} from JSON with value {}", component.class_name(), deserialized_value );
  };
}

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PERSISTENT_SYSTEM_IMPL_HPP