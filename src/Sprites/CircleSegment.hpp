#ifndef SRC_SPRITES_CIRCLESEGMENT_
#define SRC_SPRITES_CIRCLESEGMENT_

#include <vector>

namespace Game::Sprites
{

class CircleSegment
{
public:
  //! @brief Construct a new Circle Segment object
  //! @param start
  //! @param end
  //! @param vis
  CircleSegment( float start, float end, bool vis = true )
      : start_angle( start ),
        end_angle( end ),
        visible( vis )
  {
  }

  //! @brief Get the Bounds object
  //! @param center
  //! @param radius
  //! @param outline_thickness
  //! @return sf::FloatRect
  sf::FloatRect get_bounds( sf::Vector2f center, float radius, float outline_thickness ) const;

  //! @brief SFML draw function
  //! @param target
  //! @param states
  //! @param center
  //! @param radius
  //! @param outline_thickness
  //! @param color
  //! @param points_per_segment
  void draw( sf::RenderTarget &target, sf::RenderStates states, sf::Vector2f center, float radius, float outline_thickness, sf::Color color,
             int points_per_segment ) const;

  //! @brief Mark the cache as invalid to force segment vertices regen
  void invalidate_cache() const { vertices_dirty = true; }

  //! @brief Check if segment is visable
  //! @return true
  //! @return false
  bool is_visible() const { return visible; }

  //! @brief Get the segment Start Angle
  //! @return float
  float get_start_angle() const { return start_angle; }

  //! @brief Get the segment End Angle
  //! @return float
  float get_end_angle() const { return end_angle; }

private:
  float start_angle;
  float end_angle;
  bool visible;

  // Vertex caching members
  mutable std::vector<sf::Vertex> cached_vertices;
  mutable bool vertices_dirty = true;
  mutable float cached_radius = -1.0f;
  mutable float cached_outline_thickness = -1.0f;
  mutable sf::Vector2f cached_position = { -1.0f, -1.0f };
  mutable sf::Color cached_color = sf::Color::Transparent;

  void generate_vertices( sf::Vector2f center, float radius, float outline_thickness, sf::Color color, int points_per_segment ) const;
};

} // namespace Game::Sprites

#endif // SRC_SPRITES_CIRCLESEGMENT_