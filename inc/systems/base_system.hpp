#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace RENAME_THIS_NAMESPACE::Systems {

class BaseSystem {
public:
    BaseSystem() = default;
    ~BaseSystem() = default;
};

} // namespace RENAME_THIS_NAMESPACE::Systems

#endif // __SYSTEMS_BASE_SYSTEM_HPP__