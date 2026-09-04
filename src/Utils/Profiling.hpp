#ifndef SRC_UTILS_PROFILING_HPP__
#define SRC_UTILS_PROFILING_HPP__

#include <tracy/Tracy.hpp>

//! @brief Runs `expr` (a single call expression) inside a Tracy zone named after `expr` itself, so
//! the zone label can never drift out of sync with the code it profiles. Entirely a no-op — down to
//! the underlying ZoneScopedN call — unless built with -DENABLE_TRACY=ON.
//! @note Variadic so top-level commas in `expr` (e.g. multiple call arguments) are captured correctly.
#define PROFILED( ... )                                                                                                                              \
  do                                                                                                                                                 \
  {                                                                                                                                                  \
    ZoneScopedN( #__VA_ARGS__ );                                                                                                                     \
    __VA_ARGS__;                                                                                                                                     \
  } while ( false )

#endif // SRC_UTILS_PROFILING_HPP__
