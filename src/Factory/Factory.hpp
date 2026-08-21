#ifndef SRC_FACTORY_FACTORY_HPP__
#define SRC_FACTORY_FACTORY_HPP__

namespace Game::Factory
{
//! @brief Tag type wrapping a parameter pack of component types a view must include.
//! @tparam Types Component types to require.
template <typename... Types>
struct IncludePack
{
};

//! @brief Tag type wrapping a parameter pack of component types a view must exclude.
//! @tparam Types Component types to exclude.
template <typename... Types>
struct ExcludePack
{
};
} // namespace Game::Factory

#endif // SRC_FACTORY_FACTORY_HPP__
