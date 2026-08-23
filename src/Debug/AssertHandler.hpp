#ifndef SRC_DEBUG_ASSERTHANDLER_HPP__
#define SRC_DEBUG_ASSERTHANDLER_HPP__

#include <csignal>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Debug
{

//! @brief Prints a stack trace of the current call stack to the log (SPDLOG_CRITICAL).
//! @note Implemented via Windows debug-help APIs on Windows; on other platforms this is
//! currently a placeholder that only logs that stack traces are unavailable.
void stack_trace();

//! @brief Custom assertion-failure handler: logs the failed condition, message, and location,
//! optionally breaks into an attached debugger, prints a stack trace, then aborts.
//! @param condition Stringified expression that failed.
//! @param message Human-readable message describing the assertion.
//! @param file Source file where the assertion fired.
//! @param line Source line where the assertion fired.
//! @note Does not return; terminates the process via std::abort().
[[noreturn]] void assert_handler( const char *condition, const char *message, const char *file, const int line );

} // namespace Debug

//! @def ENTT_ASSERT
//! @brief Replaces the default EnTT assert macro with one that routes failures through
//! Debug::assert_handler() for richer diagnostic output (stack trace, message, location).
#ifdef ENTT_ASSERT
#undef ENTT_ASSERT
#endif
#define ENTT_ASSERT( condition, msg )                                                                                                                \
  do                                                                                                                                                 \
  {                                                                                                                                                  \
    if ( !( condition ) ) { ::Debug::assert_handler( #condition, msg, __FILE__, __LINE__ ); }                                                        \
  } while ( 0 )

#endif // SRC_DEBUG_ASSERTHANDLER_HPP__
