#ifndef SRC_UTILS_JSONDESERIALIZER_HPP__
#define SRC_UTILS_JSONDESERIALIZER_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <source_location>
#include <string>

namespace Game::Utils
{

class JsonDeserializer
{
public:
  static nlohmann::json load_json_file( const std::filesystem::path &path );

  static std::string get_string( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static float get_float( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static int get_int( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static bool get_bool( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static sf::Color get_color( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static sf::FloatRect get_float_rect( const nlohmann::json &object );
  static std::vector<int> get_int_list( const nlohmann::json &j, std::string field, std::source_location loc = std::source_location::current() );
  static std::vector<bool> get_bool_array( const nlohmann::json &j, std::string field, std::source_location loc = std::source_location::current() );
  static std::vector<uint32_t> get_u32_array( const nlohmann::json &j, std::string field,
                                              std::source_location loc = std::source_location::current() );
  static std::vector<float> get_float_array( const nlohmann::json &j, std::string field, std::source_location loc = std::source_location::current() );

  static std::string get_string_property( const nlohmann::json &j, const std::string &field,
                                          std::source_location loc = std::source_location::current() );
  static int get_int_property( const nlohmann::json &j, const std::string &field, std::source_location loc = std::source_location::current() );
  static sf::Color get_color_property( const nlohmann::json &j, const std::string &field,
                                       std::source_location loc = std::source_location::current() );

  static std::vector<int> get_int_list_property( const nlohmann::json &j, const std::string &field,
                                                 std::source_location loc = std::source_location::current() );

  static std::filesystem::path get_abs_path( const std::filesystem::path &rel );

  static sf::Vector2i get_vector2i( const nlohmann::json &j, std::string field, std::source_location loc = std::source_location::current() );

protected:
  ~JsonDeserializer() = default;

private:
  static std::string make_loc_string( const std::source_location &loc );
  static void check_contains( const nlohmann::json &j, const std::string &field, const std::source_location &loc );
};

} // namespace Game::Utils

#endif // SRC_UTILS_JSONDESERIALIZER_HPP__
