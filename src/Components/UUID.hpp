#ifndef SRC_CMPS_UUID_HPP_
#define SRC_CMPS_UUID_HPP_

#include <random>
namespace ProceduralMaze::Cmp
{

struct UUID
{
  std::array<uint8_t, 16> data{};

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

  void print() const
  {
    SPDLOG_INFO( "UUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", data[0], data[1],
                 data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15] );
  }

  std::string str() const
  {
    return std::format( "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", data[0], data[1],
                        data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                        data[15] );
  }

  //! @brief spaceship operator overload automatically generates ==, !=, <, >, <=, and >=
  bool operator<=>( const UUID & ) const = default;
};

} // namespace ProceduralMaze::Cmp
#endif // SRC_CMPS_UUID_HPP_