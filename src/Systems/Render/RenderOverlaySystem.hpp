#ifndef SRC_SYSTEMS_RENDER_RENDEROVERLAYSYSTEM_HPP__
#define SRC_SYSTEMS_RENDER_RENDEROVERLAYSYSTEM_HPP__

#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Render/UiData.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

// clang-format off
namespace Game::PathFinding { class SpatialHashGrid; enum class QueryCompass; } 
namespace Game::Cmp { class ZOrderValue; }
namespace Game::Cmp::Inventory { class WearLevel; }
namespace Game::Sprites { class SpriteSheet; class SpriteFactory; } 
namespace Game::PathFinding { class SpatialHashGrid; }
// clang-format on

namespace Game::Sys
{

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : RenderSystem( reg, window, sprite_factory, sound_bank ),
        m_debug_overlay_tex( { 1, 1 } )
  {
    m_main_ui_data = std::make_unique<Render::UiData>( "res/ui/ui.json" );
    m_dbg_ui_data = std::make_unique<Render::UiData>( "res/ui/dbg.json" );
    m_shop_ui_data = std::make_unique<Render::UiData>( "res/ui/shop.json" );
  };

  //! @brief Initialise the debug texture and use it to override the render target
  //!        Call this before using draw_screen()/draw_world()
  //! @param size
  void begin_debug_overlay( sf::Vector2u size )
  {
    if ( m_debug_overlay_tex.getSize() != size ) { [[maybe_unused]] auto _ = m_debug_overlay_tex.resize( size ); }
    m_debug_overlay_tex.clear( sf::Color::Transparent );
    set_render_target( m_debug_overlay_tex );
  }

  //! @brief reset the render target and finialise the debug texture
  //!        Call this after using draw_screen()/draw_world()
  void end_debug_overlay()
  {
    restore_render_target();
    m_debug_overlay_tex.display();
    m_debug_overlay_ready = true;
  }

  //! @brief Render the debug texture to the window.
  //!        Call this after using end_debug_overlay()
  //! @param window
  void draw_debug_overlay( sf::RenderWindow &window ) const
  {
    if ( m_debug_overlay_ready ) window.draw( sf::Sprite( m_debug_overlay_tex.getTexture() ) );
  }

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param spatial_grid_ptr
  void init( const PathFinding::SpatialHashGridSharedPtr &spatial_grid_ptr ) { m_npc_navmesh = spatial_grid_ptr; }

  void render_ui_outlines();
  void render_ui_icons();

  //! @brief Render the sprite in the Inventory UI.
  //! @note ParticleSprites are rendered seperately in RenderGameSystem::render_game
  void render_ui_inventory_icon();
  void render_ui_meters( sf::Time dt );
  void render_ui_texts();
  void render_ui_labels( sf::Time dt );

  void render_level_depth();
  void render_shop_inventory_overlay();
  void render_grimoire_inventory_overlay();

  void render_ui_misc_stats();
  void render_ui_zorder_list( std::vector<ZOrder> &zorder_queue );
  void render_ui_npc_list();

  void render_lerp_positions();
  void render_square( sf::Vector2f pos, sf::Vector2f size, sf::Color color );

  void render_crypt_maze_timer( sf::Vector2f pos, unsigned int size );

  void render_wear_level( float wearlevel, const Cmp::Position &pos );
  void render_spatial_grid_neighbours( const Cmp::Position &query_pos, sf::Color color, PathFinding::QueryCompass query_compass );
  void render_pathfinding_vector( const Cmp::Position &start_pos_cmp, const Cmp::Position &end_pos_cmp, sf::Color color,
                                  PathFinding::QueryCompass query_compass );

  void render_navmesh( PathFinding::SpatialHashGridSharedPtr npc_navmesh );

  void render_ui_entity_inspect();

  template <typename Component>
  void render_square_for_entity( entt::entity entity, sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    auto *pos_cmp = reg().try_get<Cmp::Position>( entity );
    auto requested_cmp = reg().try_get<Component>( entity );
    if ( pos_cmp && requested_cmp )
    {
      sf::RectangleShape rectangle;
      rectangle.setSize( Constants::kGridSizePxF );
      rectangle.setPosition( pos_cmp->position );
      rectangle.setFillColor( sf::Color::Transparent );
      rectangle.setOutlineColor( square_color );
      rectangle.setOutlineThickness( square_thickness );
      draw_world( rectangle );
    }
  }

  template <typename Component>
  void render_square_for_vector2f_cmp( sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    for ( auto [entity, requested_cmp] : reg().view<Component>().each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), sf::FloatRect( requested_cmp, Constants::kGridSizePxF ) ) ) continue;
      sf::RectangleShape rectangle;
      rectangle.setSize( Constants::kGridSizePxF );
      rectangle.setPosition( requested_cmp );
      rectangle.setFillColor( sf::Color::Transparent );
      rectangle.setOutlineColor( square_color );
      rectangle.setOutlineThickness( square_thickness );
      draw_world( rectangle );
    }
  }

  template <typename Component>
  void render_square_for_floatrect_cmp( sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    for ( auto [entity, requested_cmp] : reg().view<Component>().each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), requested_cmp ) ) continue;
      sf::RectangleShape rectangle;
      rectangle.setSize( requested_cmp.size );
      rectangle.setPosition( requested_cmp.position );
      rectangle.setFillColor( sf::Color::Transparent );
      rectangle.setOutlineColor( square_color );
      rectangle.setOutlineThickness( square_thickness );
      draw_world( rectangle );
    }
  }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief Layout data object for the main UI
  std::unique_ptr<Render::UiData> m_main_ui_data;
  //! @brief Layout data object for the debug UI
  std::unique_ptr<Render::UiData> m_dbg_ui_data;
  //! @brief Layout data object for the shop scene overlay
  std::unique_ptr<Render::UiData> m_shop_ui_data;

private:
  //! @brief Draw the debug ui to this texture.
  sf::RenderTexture m_debug_overlay_tex;
  //! @brief Signals when the texture is ready to be drawn to the sf::RenderWindow
  bool m_debug_overlay_ready{ false };
  //! @brief tracks the npc pathfinding navmesh i.e. where the NPC cannot move to
  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;
  //! @brief Used to flash the UI wealth text
  sf::Time m_flash_wealth_ui_interval;
  //! @brief Used to flash the UI health text
  sf::Time m_flash_health_ui_interval;
  //! @brief Used to flash the UI cadaver text
  sf::Time m_flash_cadaver_ui_interval;
  //! @brief Used to flash the UI inevntory text
  sf::Time m_flash_inventory_ui_interval;
  //! @brief Used to flash the UI radius text
  sf::Time m_flash_radius_ui_interval;
  //! @brief Screen flash frequency
  int m_ui_flash_factor{ 300 };
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_RENDER_RENDEROVERLAYSYSTEM_HPP__
