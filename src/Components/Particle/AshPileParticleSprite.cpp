#include <Components/Particle/AshPileParticleSprite.hpp>

#include <Components/Random.hpp>
#include <SFML/System/Time.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <stdexcept>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void AshPileParticle::emit()
{
  m_color_variation = Cmp::RandomInt( 0, 20 ).gen();

  static std::random_device rd;
  static std::mt19937 rng( rd() );
  m_vertex.position = m_emitter_position;
  m_emitted_particle = true;
};
} // namespace detail

AshPileParticleSprite::AshPileParticleSprite()
    : SpriteBase( kMaxPopulation )
{
  if ( kPyramidWidth % 2 != 0 ) throw std::logic_error( "kPyramidWidth must be even: simulate() fills outward from a centre pair of columns" );

  for ( int c = 0; c < kPyramidWidth; ++c )
  {
    for ( int c = 0; c < kPyramidHeight; ++c )
    {
      m_pyramid[c] = std::vector<bool>( c + 1 );
    }
    for ( int c = 0; c < kPyramidHeight; ++c )
    {
      m_pyramid[kPyramidHeight + c] = std::vector<bool>( kPyramidHeight - c );
    }
  }
};

void AshPileParticleSprite::simulate( sf::Time dt )
{
  static sf::Time emit_delay_timeout = sf::seconds( 0.1f );
  m_emit_delay_accumulator += dt;

  // Catch up rather than clamping to one placement per call: if the accumulator has built up more
  // than one timeout's worth (e.g. emit_delay_timeout set below a frame's dt), place that many
  // particles this call instead of silently discarding the surplus and capping the rate at 1/frame.
  while ( m_emit_delay_accumulator >= emit_delay_timeout )
  {
    m_emit_delay_accumulator -= emit_delay_timeout;

    bool placed_one = false;
    for ( auto &p : m_particles_list )
    {
      // keep looking until we find an empty pyramid index
      if ( not p.m_emitted_particle )
      {
        placed_one = true;
        p.do_emit();

        bool valid_pos = false;
        size_t col_index = Cmp::RandomInt( kPyramidHalfWidth - 1, kPyramidHalfWidth ).gen();
        bool going_left = col_index < static_cast<size_t>( kPyramidHalfWidth );

        auto filled_count = []( const std::vector<bool> &column ) -> size_t
        { return static_cast<size_t>( std::count( column.begin(), column.end(), true ) ); };

        while ( not valid_pos )
        {
          auto &column = m_pyramid.at( col_index );
          const size_t filled = filled_count( column );
          const bool column_full = filled == column.size();

          // Prefer a less-filled neighbour over stacking further onto this column, so the pile fills
          // outward in even rings instead of spiking the current column to full height first.
          const bool has_neighbour = going_left ? col_index > 0 : col_index + 1 < static_cast<size_t>( kPyramidWidth );
          if ( has_neighbour )
          {
            const size_t neighbour_index = going_left ? col_index - 1 : col_index + 1;
            const auto &neighbour = m_pyramid.at( neighbour_index );
            const size_t neighbour_filled = filled_count( neighbour );

            if ( neighbour_filled < neighbour.size() and ( column_full or neighbour_filled < filled ) )
            {
              col_index = neighbour_index;
              continue;
            }
          }

          if ( not column_full )
          {
            // Fill from the base upward: the base row (column.size() - 1) is the same ground line for
            // every column, so the first particle in a column always lands there, and later ones climb
            // toward the apex as `filled` grows.
            const size_t row_index = column.size() - 1 - filled;
            column.at( row_index ) = true;
            // OpenGL/SFML draws downwards so we need to flip the shape by inverting the positions using the pyramid height
            const int row_offset = kPyramidHeight - static_cast<int>( column.size() );
            p.m_vertex.position = p.m_emitter_position + sf::Vector2f( static_cast<float>( col_index ), static_cast<float>( row_offset + static_cast<int>( row_index ) ) );
            valid_pos = true;
            continue;
          }

          // This column is full and its only outward neighbour has no room either: the random
          // left/right split was uneven for this batch of particles — flip to the other half, which
          // is guaranteed to still have room since total slots == total particles.
          if ( going_left )
          {
            going_left = false;
            col_index = static_cast<size_t>( kPyramidHalfWidth );
          }
          else
          {
            going_left = true;
            col_index = static_cast<size_t>( kPyramidHalfWidth ) - 1;
          }
        }

        p.m_vertex.color.a = 255;
        p.m_vertex.color.r = 20 + p.m_color_variation;
        p.m_vertex.color.g = 20 + p.m_color_variation;
        p.m_vertex.color.b = 20 + p.m_color_variation;

        break;
      }
    }

    if ( not placed_one ) break; // every particle already emitted; stop looping
  }
}

} // namespace Game::Cmp::Particle
