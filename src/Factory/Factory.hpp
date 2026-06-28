#ifndef SRC_FACTORY_FACTORY_HPP__
#define SRC_FACTORY_FACTORY_HPP__

namespace Game::Factory
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
} // namespace Game::Factory

#endif // SRC_FACTORY_FACTORY_HPP__
