#ifndef SRC_FACTORY_FACTORY_HPP__
#define SRC_FACTORY_FACTORY_HPP__

namespace ProceduralMaze::Factory
{
// Helper structs for variadic template parameter packs
template <typename... Types>
struct IncludePack
{
};

// Helper structs for variadic template parameter packs
template <typename... Types>
struct ExcludePack
{
};
} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_FACTORY_HPP__