#ifndef __EVENTS_SAVE_SETTINGS_EVENT_HPP__
#define __EVENTS_SAVE_SETTINGS_EVENT_HPP__

namespace Game::Events
{

struct SaveSettingsEvent
{
  bool save_to_file;
  explicit SaveSettingsEvent( bool save = true )
      : save_to_file( save )
  {
  }
};

} // namespace Game::Events

#endif // __EVENTS_SAVE_SETTINGS_EVENT_HPP__