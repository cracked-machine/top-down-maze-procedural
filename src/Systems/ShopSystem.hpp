#ifndef SRC_SYSTEMS_SHOPSYSTEM_HPP_
#define SRC_SYSTEMS_SHOPSYSTEM_HPP_

#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/Shop/ShopInventory.hpp>
#include <Events/BuyShopItemEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class ShopSystem : public BaseSystem
{
public:
  ShopSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    get_systems_event_queue().sink<Events::BuyShopItemEvent>().connect<&ShopSystem::on_buy_shop_item>( this );
  }

  //! @brief Load the json configuration file
  //! @param config_path
  //! @return Cmp::ShopInventory::Config
  void load_config( const std::filesystem::path &config_path );

  //! @brief Pick a random "sprite.item." sprite and price and insert in the inventory component
  //! @param shop_inventory_cmp The inventory component
  void add_shop_inventory_item( Cmp::ShopInventory &shop_inventory_cmp );

  //! @brief Create an inventory component in the registry.
  //! @param inventory_entt The entity that will own the component
  void create_shop_inventory( entt::entity inventory_entt );

  //! @brief Check for collisions with the exit
  void check_exit_collision();

  //! @brief If player has walked up to shop keeper and triggered shop overlay
  bool check_shopkeeper_collision( sf::Vector2f shopkeeper_pos );

  //! @brief Deduct player wealth, transfer item to player, replace item in shop inventory
  //! @param item_idx
  void buy_shop_item( uint8_t item_idx );

  //! @brief Intercepts an event, calls `buy_shop_item` function.
  //! @param ev
  void on_buy_shop_item( const Events::BuyShopItemEvent &ev ) { buy_shop_item( ev.m_item_idx ); }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  const Cmp::ShopInventory::Config &get_inventory_config() { return m_shop_inventory_config; }

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  Cmp::ShopInventory::Config m_shop_inventory_config;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SHOPSYSTEM_HPP_