#version 130

uniform vec2 local_resolution;
uniform vec2 aperture_half_size;
uniform vec2 display_resolution;
uniform float time;

void main()
{
  vec2 fragCoord = gl_FragCoord.xy;

  // This shader is drawn in "local view" coord system, but uses world coords input (player position, etc...).
  // So we must transform fragment coordinates from display space to local view space
  vec2 normalized_coord = fragCoord / display_resolution;
  vec2 local_coord = normalized_coord * local_resolution;

  vec2 center = local_resolution * 0.5;

  vec2 dist_from_center = abs( local_coord - center );
  // Red outline thickness
  float outline_thickness = 3.0;

  // Check if we're on the border of the shader
  bool on_border = ( local_coord.x < outline_thickness || local_coord.x > local_resolution.x - outline_thickness ||
                     local_coord.y < outline_thickness || local_coord.y > local_resolution.y - outline_thickness );

  if ( on_border )
  {
    // Red outline
    gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
  }
  else if ( dist_from_center.x <= aperture_half_size.x && dist_from_center.y <= aperture_half_size.y )
  {
    // Inside window - make transparent
    gl_FragColor = vec4( 0.0, 0.0, 0.0, 0.0 );
  }
  else
  {
    // Outside window - make black
    gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
  }
}