#ifndef SRC_SCENECONTROL_SCENEMAP_HPP_
#define SRC_SCENECONTROL_SCENEMAP_HPP_

namespace ProceduralMaze::Scene
{

class SceneMap
{
public:
  struct MapData
  {
    int voididx;
    int wallidx;
    int openidx;
    int spawnidx;
    int width;
    int height;
    std::vector<int> map;
  };
  SceneMap( std::filesystem::path map_file );

  int get_height() { return m_map_data.height; }
  int get_width() { return m_map_data.width; }
  int get_voididx() { return m_map_data.voididx; }
  int get_wallidx() { return m_map_data.wallidx; }
  int get_openidx() { return m_map_data.openidx; }
  int get_spawnidx() { return m_map_data.spawnidx; }
  std::vector<int> get_map() { return m_map_data.map; }

private:
  void load_config( const std::filesystem::path &config_path );

  MapData m_map_data;
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENEMAP_HPP_