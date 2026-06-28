#ifndef SRC_EVENTS_SAVESETTINGSEVENT_HPP__
#define SRC_EVENTS_SAVESETTINGSEVENT_HPP__

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

#endif // SRC_EVENTS_SAVESETTINGSEVENT_HPP__
