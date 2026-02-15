#ifndef DEBUG_ASSERT_HANDLER_HPP
#define DEBUG_ASSERT_HANDLER_HPP

#include <csignal>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Debug
{

void stack_trace();

[[noreturn]] void assert_handler( const char *condition, const char *message, const char *file, const int line );

} // namespace Debug

// Replace the default entt assert macro with custom one that provides better output
#ifdef ENTT_ASSERT
#undef ENTT_ASSERT
#endif
#define ENTT_ASSERT( condition, msg )                                                                                                                \
  do                                                                                                                                                 \
  {                                                                                                                                                  \
    if ( !( condition ) ) { ::Debug::assert_handler( #condition, msg, __FILE__, __LINE__ ); }                                                        \
  } while ( 0 )

#endif // DEBUG_ASSERT_HANDLER_HPP
