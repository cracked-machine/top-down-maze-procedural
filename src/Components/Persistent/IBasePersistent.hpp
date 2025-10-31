#ifndef __CMP_IBASEPERSISTENT_HPP__
#define __CMP_IBASEPERSISTENT_HPP__

#include <nlohmann/json.hpp>
#include <string>

namespace ProceduralMaze::Cmp::Persistent {

/**
 * @brief Abstract base interface for persistent components that can be serialized and deserialized.
 *
 * IBasePersistent defines the contract for components that need to maintain their state
 * across application sessions through serialization. This interface provides the foundation
 * for a persistent component system where objects can save their state to JSON format
 * and restore it later.
 *
 * Classes implementing this interface must provide:
 * - A unique class name identifier for type identification
 * - Serialization capability to convert object state to JSON
 * - Deserialization capability to restore object state from JSON
 *
 * This interface is typically used in game engines, configuration systems, or any
 * application requiring persistent state management.
 *
 * @note All derived classes must implement the three pure virtual methods to ensure
 *       proper serialization/deserialization functionality.
 * @note The virtual destructor ensures proper cleanup in polymorphic scenarios.
 */
class IBasePersistent
{
public:
  virtual ~IBasePersistent() = default;

  /**
   * @brief Returns the class name as a string for identification purposes.
   *
   * This pure virtual method must be implemented by derived classes to provide
   * a string representation of their class name. This is typically used for
   * serialization, debugging, or runtime type identification in persistent
   * component systems.
   *
   * @return std::string The name of the implementing class
   */
  virtual std::string class_name() const = 0;

  /**
   * @brief Deserializes object state from JSON data.
   *
   * This pure virtual method must be implemented by derived classes to restore
   * the object's state from the provided JSON data. The implementation should
   * parse the JSON and set the appropriate member variables to reconstruct
   * the object's previous state.
   *
   * @param json_data The JSON object containing the serialized data to be loaded
   * @throws May throw exceptions if the JSON data is malformed or incompatible
   */
  virtual void deserialize( const nlohmann::json &json_data ) = 0;

  /**
   * @brief Serializes the persistent component to JSON format.
   *
   * This pure virtual method must be implemented by derived classes to convert
   * the component's data into a JSON representation for persistence purposes.
   * The serialized data should contain all necessary information to reconstruct
   * the component's state during deserialization.
   *
   * @return nlohmann::json A JSON object containing the serialized component data.
   */
  virtual nlohmann::json serialize() const = 0;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_IBASEPERSISTENT_HPP__