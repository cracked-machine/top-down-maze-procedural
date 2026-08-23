#ifndef SRC_EVENTS_SAVESETTINGSEVENT_HPP__
#define SRC_EVENTS_SAVESETTINGSEVENT_HPP__

namespace Game::Events
{

//! @brief Requests that current settings be saved, optionally persisting them to disk.
struct SaveSettingsEvent
{
  //! @brief True to persist the settings to file; false to save in-memory only.
  bool save_to_file;
  //! @brief Construct a new SaveSettingsEvent object
  //! @param save True to persist the settings to file; false to save in-memory only. Defaults to true.
  explicit SaveSettingsEvent( bool save = true )
      : save_to_file( save )
  {
  }
};

} // namespace Game::Events

#endif // SRC_EVENTS_SAVESETTINGSEVENT_HPP__
