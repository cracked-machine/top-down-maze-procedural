#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;
  vec2 worldPos = texCoord * screenSize;

  // Convert to centered coordinates (-1 to 1 range)
  vec2 uv = ( texCoord - 0.5 ) * 2.0;

  // Time-based animation
  float t = time * 0.3;

  // Calculate distance and angle from center
  float dist = length( uv );
  float angle = atan( uv.y, uv.x );

  // Create multiple wave distortions (concentric circles)
  float wave1 = sin( dist * 12.0 - t * 2.0 ) * 0.15;
  float wave2 = sin( dist * 20.0 + t * 1.5 ) * 0.08;
  float wave3 = cos( dist * 30.0 - t * 3.0 ) * 0.05;

  // Radial distortion that varies with angle and time
  float radial_distort = sin( angle * 4.0 + t * 1.5 ) * 0.12;

  // Spiral distortion
  float spiral_distort = sin( angle * 3.0 + dist * 10.0 + t * 2.0 ) * 0.10;

  // Combine all distortions
  vec2 distortion = vec2( wave1 + wave2 + radial_distort + spiral_distort, wave1 + wave3 - radial_distort + spiral_distort );

  // Apply polar coordinate distortion
  float polar_distort = sin( dist * 8.0 + angle * 2.0 + t * 2.0 ) * 0.08;
  distortion += vec2( cos( angle ), sin( angle ) ) * polar_distort;

  // Apply distortion to texture coordinates
  vec2 distorted_texCoord = texCoord + distortion * 0.1;

  // Sample texture with distortion
  vec4 color;
  if ( distorted_texCoord.x >= 0.0 && distorted_texCoord.x <= 1.0 && distorted_texCoord.y >= 0.0 && distorted_texCoord.y <= 1.0 )
  {
    color = texture2D( texture, distorted_texCoord );
  }
  else
  {
    color = texture2D( texture, texCoord ); // Fallback to original
  }

  gl_FragColor = color;
}