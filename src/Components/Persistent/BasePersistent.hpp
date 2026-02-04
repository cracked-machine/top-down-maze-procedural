#ifndef __CMP_BASEPERSISTENT_HPP__
#define __CMP_BASEPERSISTENT_HPP__

#include <Components/Persistent/IBasePersistent.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp::Persist
{

// Base class for persistent settings
// These types should be added to the registry as context variables
// If you add them as components to entities, they will be destroyed on game reset
// and lose their values
template <typename T>
class BasePersistent : public IBasePersistent
{
public:
  BasePersistent( T value_in, T min_value_in = T{}, T max_value_in = T{} )
      : value( value_in ),
        min_value( min_value_in ),
        max_value( max_value_in )
  {
  }

  //! @brief polymorphic destructor for derived classes
  virtual ~BasePersistent() = default;

  T &get_value() { return value; };
  T &get_min_value() { return min_value; };
  T &get_max_value() { return max_value; };
  std::string &get_detail() { return detail; };
  std::string &get_format() { return format; };

  // Add pointer accessors for ImGui SliderScalar compatibility
  T *get_value_ptr() { return &value; };
  T *get_min_value_ptr() { return &min_value; };
  T *get_max_value_ptr() { return &max_value; };
  std::string *get_detail_ptr() { return &detail; };
  std::string *get_format_ptr() { return &format; };

  // object serialization to json
  virtual nlohmann::json serialize() const override
  {
    nlohmann::json json_data;

    if constexpr ( std::is_floating_point_v<T> ) { json_data["type"] = "float"; }
    else if constexpr ( std::is_integral_v<T> ) { json_data["type"] = "int"; }

    json_data["value"] = value;
    json_data["min_value"] = min_value;
    json_data["max_value"] = max_value;
    json_data["detail"] = detail;
    json_data["format"] = format;

    return json_data;
  }

  // json deserialization to object
  virtual void deserialize( const nlohmann::json &json_data ) override
  {
    SPDLOG_DEBUG( "deserialize called with: {}", json_data.dump() );

    if ( json_data.contains( "value" ) && json_data["value"].is_number() ) value = json_data["value"].get<T>();

    if ( json_data.contains( "min_value" ) && json_data["min_value"].is_number() ) { min_value = json_data["min_value"].get<T>(); }

    if ( json_data.contains( "max_value" ) && json_data["max_value"].is_number() ) { max_value = json_data["max_value"].get<T>(); }

    if ( json_data.contains( "detail" ) )
    {
      SPDLOG_DEBUG( "detail field exists, is_string={}", json_data["detail"].is_string() );
      if ( json_data["detail"].is_string() )
      {
        detail = json_data["detail"].get<std::string>();
        SPDLOG_DEBUG( "detail set to: '{}'", detail );
      }
    }
    else { SPDLOG_WARN( "detail field NOT found in json" ); }

    if ( json_data.contains( "format" ) )
    {
      SPDLOG_DEBUG( "format field exists, is_string={}", json_data["format"].is_string() );
      if ( json_data["format"].is_string() )
      {
        format = json_data["format"].get<std::string>();
        SPDLOG_DEBUG( "format set to: '{}'", format );
      }
    }
    else { SPDLOG_WARN( "format field NOT found in json" ); }

    if ( format.empty() ) { SPDLOG_WARN( "InvalidJSONField: 'format' for {}", class_name() ); }
    if ( detail.empty() ) { SPDLOG_WARN( "InvalidJSONField: 'details' for {}", class_name() ); }
  }

  // Default widget implementation - can be overridden by derived classes
  void render_widget() override
  {
    ImGui::PushID( this );

    const char *label = detail.empty() ? "<Invalid Label>" : detail.c_str();

    if constexpr ( std::is_same_v<T, float> )
    {
      const char *fmt = format.empty() ? "%.3f" : format.c_str();
      ImGui::SliderScalar( label, ImGuiDataType_Float, get_value_ptr(), get_min_value_ptr(), get_max_value_ptr(), fmt );
    }
    else if constexpr ( std::is_same_v<T, uint8_t> )
    {
      const char *fmt = format.empty() ? "%d" : format.c_str();
      ImGui::SliderScalar( label, ImGuiDataType_U8, get_value_ptr(), get_min_value_ptr(), get_max_value_ptr(), fmt );
    }
    else if constexpr ( std::is_same_v<T, uint16_t> )
    {
      const char *fmt = format.empty() ? "%d" : format.c_str();
      ImGui::SliderScalar( label, ImGuiDataType_U16, get_value_ptr(), get_min_value_ptr(), get_max_value_ptr(), fmt );
    }
    else if constexpr ( std::is_same_v<T, uint32_t> )
    {
      const char *fmt = format.empty() ? "%u" : format.c_str();
      ImGui::SliderScalar( label, ImGuiDataType_U32, get_value_ptr(), get_min_value_ptr(), get_max_value_ptr(), fmt );
    }
    else if constexpr ( std::is_same_v<T, uint64_t> )
    {
      const char *fmt = format.empty() ? "%llu" : format.c_str();
      // ImGui requires max_value <= IM_U64_MAX / 2 for sliders
      constexpr uint64_t imgui_max_limit = std::numeric_limits<uint64_t>::max() / 2;
      uint64_t clamped_max = std::min( max_value, imgui_max_limit );
      ImGui::SliderScalar( label, ImGuiDataType_U64, get_value_ptr(), get_min_value_ptr(), &clamped_max, fmt );
    }
    else if constexpr ( std::is_same_v<T, int32_t> || std::is_same_v<T, int> )
    {
      const char *fmt = format.empty() ? "%d" : format.c_str();
      ImGui::SliderScalar( label, ImGuiDataType_S32, get_value_ptr(), get_min_value_ptr(), get_max_value_ptr(), fmt );
    }
    else if constexpr ( std::is_same_v<T, int64_t> )
    {
      const char *fmt = format.empty() ? "%lld" : format.c_str();
      // ImGui requires max_value <= IM_S64_MAX / 2 for sliders
      constexpr int64_t imgui_max_limit = std::numeric_limits<int64_t>::max() / 2;
      int64_t clamped_max = std::min( max_value, imgui_max_limit );
      ImGui::SliderScalar( label, ImGuiDataType_S64, get_value_ptr(), get_min_value_ptr(), &clamped_max, fmt );
    }
    else
    {
      // Fallback for unsupported types - just show the value as text
      ImGui::Text( "%s: (unsupported type)", label );
    }

    ImGui::PopID();
  }

protected:
  T value{};
  T min_value{};
  T max_value{};
  std::string detail{ "" };
  std::string format{ "" };
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_BASEPERSISTENT_HPP__
