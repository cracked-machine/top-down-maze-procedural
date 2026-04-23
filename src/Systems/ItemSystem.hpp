#ifndef SRC_SYSTEMS_ITEMSYSTEM_HPP_
#define SRC_SYSTEMS_ITEMSYSTEM_HPP_

#include <Inventory/InventoryItem.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>

namespace ProceduralMaze::Sys
{

class ItemSystem : public BaseSystem
{
public:
  ItemSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~ItemSystem() {}

  static ItemSystem &instance()
  {
    if ( !s_instance ) throw std::runtime_error( "ItemSystem not yet initialized" );
    return *s_instance;
  }

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

  [[nodiscard]] Cmp::InventoryItem get_item( const std::string &item_key ) const;
  [[nodiscard]] std::vector<std::string> get_all_item_keys() const;
  [[nodiscard]] std::string get_random_item_from_list( std::vector<std::string> list ) const;

private:
  static ItemSystem *s_instance;
  std::filesystem::path m_item_json_file_path{ "res/json/items.json" };
  std::unordered_map<Sprites::SpriteMetaType, Cmp::InventoryItem> m_item_store;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_ITEMSYSTEM_HPP_