#ifndef SRC_COMPONENTS_UUID_HPP__
#define SRC_COMPONENTS_UUID_HPP__

#include <algorithm>
#include <functional>
#include <random>
namespace Game::Cmp
{

//! @brief Use this component to associate two entities where persistance cannot be guaranteed with the entity id itself
struct UUID
{
  //! @brief The 16 raw bytes of the UUID (version 4, variant per RFC 4122).
  std::array<uint8_t, 16> data{};

  //! @brief Generate a new random version-4 UUID.
  //! @return UUID A newly generated, randomly-seeded UUID.
  static UUID generate()
  {
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_int_distribution<uint32_t> dist{ 0, 255 };
    UUID uuid;
    for ( auto &byte : uuid.data )
      byte = static_cast<uint8_t>( dist( rng ) );
    // version 4
    uuid.data[6] = ( uuid.data[6] & 0x0F ) | 0x40;
    // variant bits
    uuid.data[8] = ( uuid.data[8] & 0x3F ) | 0x80;
    // uuid.print();
    return uuid;
  }

  //! @brief Log the UUID in canonical hyphenated hex form via SPDLOG_INFO.
  void print() const
  {
    SPDLOG_INFO( "UUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", data[0], data[1],
                 data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15] );
  }

  //! @brief Format the UUID in canonical hyphenated hex form.
  //! @return std::string The UUID as a canonical hex string.
  std::string str() const
  {
    return std::format( "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", data[0], data[1],
                        data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                        data[15] );
  }

  //! @brief spaceship operator overload automatically generates ==, !=, <, >, <=, and >=
  auto operator<=>( const UUID & ) const = default;

  //! @brief Returns true if all bytes are zero (default-constructed, no UUID assigned)
  [[nodiscard]] bool empty() const
  {
    return std::ranges::all_of( data, []( uint8_t b ) { return b == 0; } );
  }
};

} // namespace Game::Cmp

//! @brief std::hash specialization enabling Game::Cmp::UUID to be used as a key in unordered containers.
template <>
struct std::hash<Game::Cmp::UUID>
{
  //! @brief Compute a combined hash over all bytes of the UUID.
  //! @param uuid The UUID to hash.
  //! @return std::size_t The combined hash value.
  std::size_t operator()( const Game::Cmp::UUID &uuid ) const noexcept
  {
    std::size_t seed = 0;
    for ( auto byte : uuid.data )
      seed ^= std::hash<uint8_t>{}( byte ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
    return seed;
  }
};

#endif // SRC_COMPONENTS_UUID_HPP__
