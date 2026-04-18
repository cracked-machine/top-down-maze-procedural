#include <Events/DropInventoryEvent.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Wall.hpp>
#include <Constants.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Random.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Shop/ShopInventory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/ShopSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace nlohmann
{
//! @brief ADL hook used via nlohmann::basic_json::get (see ShopSystem::load_config below)
template <>
struct adl_serializer<ProceduralMaze::Cmp::ShopInventory::Config>
{
  static void from_json( const json &j, ProceduralMaze::Cmp::ShopInventory::Config &c )
  {
    // get the json object for "inventory_config"
    if ( not j.contains( "inventory_config" ) ) throw std::runtime_error( "Missing 'inventory_config' from JSON scene config file" );
    const auto &config = j.at( "inventory_config" );

    //! @brief helper for error checking on JSON Single field types
    auto get_field = [&config]<typename T>( const std::string &key, T &out )
    {
      if ( !config.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON scene config file" );
      try
      {
        out = config.at( key ).get<T>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing '" + key + "': " + e.what() );
      }
    };

    //! @brief helper for error checking on JSON x/y vector types
    auto get_xy_field = [&config]<typename TVec>( const std::string &key, TVec &out )
    {
      using TScalar = decltype( out.x );
      if ( !config.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON scene config file" );
      try
      {
        out.x = config.at( key ).at( "x" ).get<TScalar>();
        out.y = config.at( key ).at( "y" ).get<TScalar>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing '" + key + "': " + e.what() );
      }
    };

    //! @brief helper for error checking on JSON r/g/b/a vector types
    auto get_rgba_field = [&config]<typename TVec>( const std::string &key, TVec &out )
    {
      using TScalar = decltype( out.r );
      if ( !config.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON scene config file" );
      try
      {
        out.r = config.at( key ).at( "r" ).get<TScalar>();
        out.g = config.at( key ).at( "g" ).get<TScalar>();
        out.b = config.at( key ).at( "b" ).get<TScalar>();
        out.a = config.at( key ).at( "a" ).get<TScalar>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing '" + key + "': " + e.what() );
      }
    };

    get_field( "max_items", c.max_items );
    if ( c.max_items > 5 ) { throw std::runtime_error( "Property 'inventory_config.max_items' is hard-clamped to 5." ); }
    get_field( "min_price", c.min_price );
    get_field( "max_price", c.max_price );
    get_field( "ui_mainlinesize", c.ui_mainlinesize );
    get_field( "ui_fontsize", c.ui_fontsize );
    get_field( "ui_slotlinesize", c.ui_slotlinesize );

    get_xy_field( "ui_position", c.ui_position );
    get_xy_field( "ui_size", c.ui_size );

    get_rgba_field( "ui_mainbgcolor", c.ui_mainbgcolor );
    get_rgba_field( "ui_slotbgcolor", c.ui_slotbgcolor );
    get_rgba_field( "ui_fontcolor", c.ui_fontcolor );
    get_rgba_field( "ui_slotlinecolor", c.ui_slotlinecolor );
    get_rgba_field( "ui_mainlinecolor", c.ui_mainlinecolor );
  }
};
} // namespace nlohmann

namespace ProceduralMaze::Sys
{

void ShopSystem::load_config( const std::filesystem::path &config_path )
{
  if ( not std::filesystem::exists( config_path ) )
  {
    SPDLOG_ERROR( "Config file does not exist: {}", config_path.string() );
    throw std::runtime_error( "Config file not found: " + config_path.string() );
  }

  std::ifstream file( config_path );
  if ( not file.is_open() )
  {
    SPDLOG_ERROR( "Unable to open config file: {}", config_path.string() );
    throw std::runtime_error( "Cannot open config file: " + config_path.string() );
  }

  //! @brief Attempt deserialise using the Argument-dependent lookup (ADL) serializer above
  try
  {
    nlohmann::json j;
    file >> j;
    m_shop_inventory_config = j.get<Cmp::ShopInventory::Config>();
  } catch ( const ::nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", config_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }
}

void ShopSystem::add_inventory_item( Cmp::ShopInventory &shop_inventory_cmp )
{
  auto carryitem_types = m_sprite_factory.get_all_sprite_types_by_pattern( "CARRYITEM" );
  Cmp::RandomInt item_picker( 0, static_cast<int>( carryitem_types.size() ) - 1 );
  Cmp::RandomInt price_picker( shop_inventory_cmp.m_config.min_price, shop_inventory_cmp.m_config.max_price );
  auto item = item_picker.gen();
  auto price = price_picker.gen();
  SPDLOG_INFO( "Adding shop item - {} - for {}", carryitem_types.at( item ), price );
  shop_inventory_cmp.m_slots.emplace_back( carryitem_types.at( item ), price );
}

void ShopSystem::create_inventory( entt::entity inventory_entt )
{
  auto *inventory_cmp = reg().try_get<Cmp::ShopInventory>( inventory_entt );
  if ( not inventory_cmp ) return;

  for ( auto _ : std::views::iota( 0, inventory_cmp->m_config.max_items ) )
  {
    add_inventory_item( *inventory_cmp );
  }
}

void ShopSystem::check_exit_collision()
{
  auto player_pos_cmp = Utils::Player::get_position( reg() );
  auto door_view = reg().view<Cmp::Exit, Cmp::Position>();

  for ( auto [door_entity, door_cmp, door_pos_cmp] : door_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), door_pos_cmp ) ) continue;

    auto decreased_entrance_bounds = Cmp::RectBounds::scaled( door_pos_cmp.position, door_pos_cmp.size, 0.1f,
                                                              Cmp::RectBounds::ScaleAxis::XY ); // shrink entrance bounds slightly for better UX

    if ( not player_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

    SPDLOG_INFO( "check_exit_collision: Player exiting Shop to graveyard at position ({}, {})", player_pos_cmp.position.x,
                 player_pos_cmp.position.y );
    m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_SHOP );
  }
}

bool ShopSystem::check_shopkeeper_collision( sf::Vector2f shopkeeper_pos )
{
  bool result = false;
  auto shopkeeper_hitbox = Cmp::RectBounds::scaled( shopkeeper_pos, Constants::kGridSizePxF, 3.f );
  auto player_pos = Utils::Player::get_position( reg() );
  if ( not shopkeeper_hitbox.findIntersection( player_pos ) ) return result;
  return result = true;
}

void ShopSystem::buy_shop_item( uint8_t item_idx )
{
  auto inventory_view = reg().view<Cmp::ShopInventory>().each();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view )
  {
    if ( not inventory_cmp.is_enabled ) continue;
    if ( item_idx == 0 ) return;

    // auto && required: std::views::enumerate returns a temporary tuple, auto & cannot bind to it
    for ( auto &&[i, slot] : std::views::enumerate( inventory_cmp.m_slots ) )
    {
      // selections presented to the user are not zero-indexed so temp bump the idx when checking if it matches user selection
      if ( i + 1 != item_idx ) continue;

      // Found the inventory slot selected by user
      auto &[item, price] = slot;

      SPDLOG_INFO( "Found slot {}: {} - {}", item_idx, item, price );

      // check if player has enough money
      auto &player_wealth = Utils::Player::get_wealth( reg() );
      if ( player_wealth.wealth < static_cast<int32_t>( price ) ) { break; }
      player_wealth.wealth -= price;

      auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
      auto player_pos = Utils::Player::get_position( reg() ).position;
      get_systems_event_queue().trigger( Events::DropInventoryEvent( inventory_entt, player_pos ) );

      // add new carryitem into player inventory
      Factory::add_inventory( reg(), item );

      // delete item from shop inventory
      auto &slots = inventory_cmp.m_slots;
      auto it = std::next( slots.begin(), i );
      slots.erase( it );

      // replace the shop stock
      add_inventory_item( inventory_cmp );
      break;
    }

    // ensure that the shop is always fully stocked.
    while ( inventory_cmp.m_slots.size() < static_cast<size_t>( inventory_cmp.m_config.max_items ) )
    {
      add_inventory_item( inventory_cmp );
    }
  }
}

} // namespace ProceduralMaze::Sys