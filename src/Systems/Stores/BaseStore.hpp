#ifndef SRC_SYSTEMS_BASESTORE_HPP_
#define SRC_SYSTEMS_BASESTORE_HPP_

#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Random.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>

#include <filesystem>
#include <nlohmann/json_fwd.hpp>

namespace ProceduralMaze::Sys
{

class BaseStore : public BaseSystem
{
public:
  BaseStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~BaseStore() {}

  nlohmann::json load_json_file( const std::filesystem::path &json_file );
  void init_item_store();
  int health( const nlohmann::json & );
  int fear( const nlohmann::json & );
  int despair( const nlohmann::json & );
  int infamy( const nlohmann::json & );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

protected:
  std::filesystem::path m_json_file_path;
};

//////////////////////////////////////////////////////////////////////////
// CRTP intermediate — gives each derived class its own static instance //
//////////////////////////////////////////////////////////////////////////

template <typename Derived, typename StoreValue>
class StoreSingleton : public BaseStore
{
public:
  using BaseStore::BaseStore;
  using store_map = std::unordered_map<std::string, StoreValue>;

  static Derived &instance()
  {
    if ( not s_instance ) throw std::runtime_error( "Store not yet initialized" );
    return *s_instance;
  }

  [[nodiscard]] StoreValue get_item( const std::string &key ) const
  {
    auto it = m_store.find( key );
    if ( it == m_store.end() ) throw std::runtime_error( "Unknown key: " + key );
    return it->second;
  }

  [[nodiscard]] std::vector<std::string> get_all_item_keys() const
  {
    std::vector<std::string> keys;
    keys.reserve( m_store.size() );
    for ( const auto &[k, _] : m_store )
      keys.push_back( k );
    return keys;
  }

  [[nodiscard]] std::string get_random_item_from_list( std::vector<std::string> list ) const
  {
    if ( list.empty() ) throw std::runtime_error( "provided list is empty" );
    Cmp::RandomInt picker( 0, static_cast<int>( list.size() ) - 1 );
    return list.at( picker.gen() );
  }

protected:
  store_map m_store;
  static Derived *s_instance;
};

template <typename Derived, typename StoreValue>
Derived *StoreSingleton<Derived, StoreValue>::s_instance = nullptr;

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_BASESTORE_HPP_