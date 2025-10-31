#ifndef __EVENTS_SAVE_SETTINGS_EVENT_HPP__
#define __EVENTS_SAVE_SETTINGS_EVENT_HPP__

namespace ProceduralMaze::Events {

struct SaveSettingsEvent
{
  bool save_to_file;
  explicit SaveSettingsEvent( bool save = true )
      : save_to_file( save )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_SAVE_SETTINGS_EVENT_HPP__