#ifndef EFFECTS_ITEM_HPP
#define EFFECTS_ITEM_HPP

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <memory>

namespace ProceduralMaze::Audio
{

struct EffectsData
{
  //! @brief Sound buffer for the effect
  //! Use unique_ptr for stable address
  std::unique_ptr<sf::SoundBuffer> buffer;

  //! @brief Sound control for the effect
  sf::Sound control;

  EffectsData( const std::filesystem::path &filepath );

  //! @brief Construct a new Effects Data object
  //! Need to define move constructor since we have unique_ptr
  //! @param other
  EffectsData( EffectsData &&other ) noexcept;

  //! @brief Move assignment operator
  //! Need to define move assignment operator since we have unique_ptr
  //! @param other
  //! @return EffectsData&
  EffectsData &operator=( EffectsData &&other ) noexcept;

  //! Delete copy constructor and copy assignment operator
  EffectsData( const EffectsData & ) = delete;
  EffectsData &operator=( const EffectsData & ) = delete;
};

} // namespace ProceduralMaze::Audio

#endif // EFFECTS_ITEM_HPP