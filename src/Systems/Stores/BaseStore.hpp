#ifndef SRC_SYSTEMS_STORES_BASESTORE_HPP__
#define SRC_SYSTEMS_STORES_BASESTORE_HPP__

#include <Components/Inventory/WorldItem.hpp>
#include <Components/Random.hpp>
#include <Components/Stats/BaseAction.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>

#include <filesystem>
#include <nlohmann/json_fwd.hpp>

namespace Game::Sys
{

//! @brief Base class for JSON-backed data stores. Loads JSON files from disk and provides shared helpers for
//! extracting Cmp::Stats::BaseAction fields (health, fear, despair, infamy, toxicity, tick, disease) from JSON entries.
class BaseStore : public BaseSystem
{
public:
  //! @brief Construct a new Base Store object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  BaseStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~BaseStore() {}

  //! @brief Load and parse a JSON file from disk.
  //! @param json_file
  //! @return The parsed JSON document.
  //! @throws std::runtime_error if the file does not exist or cannot be opened.
  nlohmann::json load_json_file( const std::filesystem::path &json_file );

  //! @brief Declared but not implemented; item store initialization is instead handled by ItemStore::init_store().
  void init_item_store();

  //! @brief Extract the "health" field from a JSON action entry.
  //! @param item
  int health( const nlohmann::json &item );

  //! @brief Extract the "fear" field from a JSON action entry.
  //! @param item
  int fear( const nlohmann::json &item );

  //! @brief Extract the "despair" field from a JSON action entry.
  //! @param item
  int despair( const nlohmann::json &item );

  //! @brief Extract the "infamy" field from a JSON action entry.
  //! @param item
  int infamy( const nlohmann::json &item );

  //! @brief Extract the "toxicity" field from a JSON action entry.
  //! @param item
  int toxicity( const nlohmann::json &item );

  //! @brief Extract the "tick" field from a JSON action entry.
  //! @param item
  float tick( const nlohmann::json &item );

  //! @brief Extract the "disease" field (or the entry itself, if it has no nested "disease" object) from a JSON action entry.
  //! @param item
  Cmp::Stats::Disease disease( const nlohmann::json &item );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

protected:
  //! @brief Filesystem path of the JSON file this store loads its data from.
  std::filesystem::path m_json_file_path;
};

//////////////////////////////////////////////////////////////////////////
// CRTP intermediate — gives each derived class its own static instance //
//////////////////////////////////////////////////////////////////////////

//! @brief CRTP base for singleton, string-keyed data stores. Gives each Derived store class its own static
//! instance and a shared map of string id to StoreValue, plus lookup helpers.
//! @tparam Derived
//! @tparam StoreValue
template <typename Derived, typename StoreValue>
class StoreSingleton : public BaseStore
{
public:
  using BaseStore::BaseStore;
  using store_map = std::unordered_map<std::string, StoreValue>;

  //! @brief Get the singleton instance of the Derived store.
  //! @return Reference to the Derived store instance.
  //! @throws std::runtime_error if the store has not yet been initialized.
  static Derived &instance()
  {
    if ( not s_instance ) throw std::runtime_error( "Store not yet initialized" );
    return *s_instance;
  }

  //! @brief Look up a stored value by its string key.
  //! @param key
  //! @return A copy of the stored value.
  //! @throws std::runtime_error if the key is not present in the store.
  [[nodiscard]] StoreValue get_item( const std::string &key ) const
  {
    auto it = m_store.find( key );
    if ( it == m_store.end() ) throw std::runtime_error( "Unknown key: " + key );
    return it->second;
  }

  //! @brief Get all keys in the store, excluding the "ERROR_SPRITE" sentinel key.
  //! @return The list of keys.
  [[nodiscard]] std::vector<std::string> get_all_item_keys() const
  {
    std::vector<std::string> keys;
    keys.reserve( m_store.size() );
    for ( const auto &[k, _] : m_store )
    {
      if ( k == "ERROR_SPRITE" ) continue;
      keys.push_back( k );
    }
    return keys;
  }

  //! @brief Pick a random element from a caller-supplied list of keys.
  //! @param list
  //! @return The randomly chosen key.
  //! @throws std::runtime_error if list is empty.
  [[nodiscard]] std::string get_random_item_from_list( std::vector<std::string> list ) const
  {
    if ( list.empty() ) throw std::runtime_error( "provided list is empty" );
    Cmp::RandomInt picker( 0, static_cast<int>( list.size() ) - 1 );
    return list.at( picker.gen() );
  }

protected:
  //! @brief String-keyed map of all loaded StoreValue entries.
  store_map m_store;

  //! @brief Pointer to the single Derived instance, set by the derived class's constructor.
  static Derived *s_instance;
};

template <typename Derived, typename StoreValue>
Derived *StoreSingleton<Derived, StoreValue>::s_instance = nullptr;

} // namespace Game::Sys

#endif // SRC_SYSTEMS_STORES_BASESTORE_HPP__
