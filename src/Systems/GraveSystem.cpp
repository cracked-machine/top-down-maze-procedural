
#include <Stats/ExhumeAction.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <typeindex>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <Audio/SoundBank.hpp>
#include <Components/AbsoluteAlpha.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/BombFactory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

GraveSystem::GraveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&GraveSystem::on_player_action>( this );
}

void GraveSystem::check_player_grave_collision()
{
  auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( reg() );
  if ( not inventory_slot_type.contains( "pickaxe" ) and not inventory_slot_type.contains( "axe" ) and not inventory_slot_type.contains( "shovel" ) )
  {
    return;
  }

  if ( Utils::Player::get_inventory_wear_level( reg() ) <= 0 ) { return; }

  // abort if still in cooldown
  auto digging_cooldown_amount = Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( reg() ).get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = reg().view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    reg().remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all closed grave entities
  auto position_view = reg().view<Cmp::Position, Cmp::GraveMultiBlock, Cmp::SpriteAnimation>( entt::exclude<Cmp::SelectedPosition> );
  for ( auto [grave_entity, grave_pos_cmp, grave_cmp, grave_anim_cmp] : position_view.each() )
  {
    if ( grave_anim_cmp.m_sprite_type.contains( ".opened" ) ) continue;

    auto mouse_position_bounds = Utils::get_mouse_bounds_in_gameview( m_window, RenderSystem::get_world_view() );
    if ( mouse_position_bounds.findIntersection( grave_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", grave_cmp.position.x, grave_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
      {
        auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, Constants::kGridSizePxF, 1.5f );
        if ( player_hitbox.findIntersection( grave_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby ) { continue; }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      reg().emplace_or_replace<Cmp::SelectedPosition>( grave_entity, grave_pos_cmp.position );
      m_dig_cooldown_clock.restart();

      float reduction_amount = Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( reg() ).get_value();
      Utils::Player::reduce_inventory_wear_level( reg(), reduction_amount );

      grave_cmp.hp -= Utils::Maths::to_percent( 255.f, Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( reg() ).get_value() );

      if ( grave_cmp.hp > 0 )
      {
        // play bashing animation
        m_sound_bank.get_effect( "hit_grave" ).play();
      }
      else
      {

        if ( std::string::size_type n = grave_anim_cmp.m_sprite_type.find( ".closed" ); n != std::string::npos )
        {
          grave_anim_cmp.m_sprite_type = grave_anim_cmp.m_sprite_type.substr( 0, n ) + ".opened";
          SPDLOG_DEBUG( "Grave Cmp::SpriteAnimation changed to opened type: {}", grave_anim_cmp.m_sprite_type );

          // select the final smash sound
          m_sound_bank.get_effect( "pickaxe_final" ).play();
        }

        auto grave_activation_rng = Cmp::RandomInt( 1, 4 );
        auto consequence = grave_activation_rng.gen();
        switch ( consequence )
        {
          case 1: {
            SPDLOG_DEBUG( "Grave activated NPC trap." );
            Factory::create_npc( reg(), grave_entity, "npc.ghost" );
            m_sound_bank.get_effect( "spawn_ghost" ).play();
            break;
          }
          case 2: {
            SPDLOG_DEBUG( "Grave activated bomb trap." );
            get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::GRAVE_BOMB ) );
            break;
          }
          case 3: {

            auto grave_cmp_bounds = Cmp::RectBounds::scaled( grave_cmp.position, grave_cmp.size, 2.f );
            std::vector<Sprites::SpriteMetaType> relic_selection_list{ "item.relic1", "item.relic2", "item.relic3", "item.relic4" };
            Cmp::RandomInt relic_picker( 0, relic_selection_list.size() - 1 );
            auto selected_relic = relic_picker.gen();
            auto relic_entt = Factory::create_world_item( reg(), Utils::Player::get_position( reg() ), relic_selection_list.at( selected_relic ) );

            // Apply the effects from exhuming this item to the player stats
            auto item = Sys::ItemStore::instance().get_item( relic_selection_list.at( selected_relic ) );
            Utils::Player::get_player_stats( reg() ).apply_modifiers( item.actions.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ).action );

            if ( relic_entt != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
            break;
          }
          case 4: {
            auto grave_cmp_bounds = Cmp::RectBounds::scaled( grave_cmp.position, grave_cmp.size, 2.f );
            std::vector<Sprites::SpriteMetaType> jewelry_selection_list{ "item.jewelry_sapphire_necklace", "item.jewelry_amephyst_ring",
                                                                         "item.jewelry_ruby_ring",         "item.jewelry_emerald_necklace",
                                                                         "item.jewelry_emerald_gemstone",  "item.jewelry_sapphire_gemstone",
                                                                         "item.jewelry_diamond_gemstone",  "item.jewelry_amephyst_gemstone" };
            Cmp::RandomInt jewelry_picker( 0, jewelry_selection_list.size() - 1 );
            auto selected_jewelry = jewelry_picker.gen();
            auto jewelry_entt = Factory::create_world_item( reg(), Utils::Player::get_position( reg() ),
                                                            jewelry_selection_list.at( selected_jewelry ) );

            // Apply the effects from exhuming this item to the player stats
            auto item = Sys::ItemStore::instance().get_item( jewelry_selection_list.at( selected_jewelry ) );
            Utils::Player::get_player_stats( reg() ).apply_modifiers( item.actions.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ).action );

            if ( jewelry_entt != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
            break;
          }
        }
      }
    }
  }
}

void GraveSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_grave_collision();
  }
}

} // namespace ProceduralMaze::Sys