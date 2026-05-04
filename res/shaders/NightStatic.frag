#version 330

// Sampled texture (passed in by SFML)
uniform sampler2D texture;
// elapsed time
uniform float time;
// effect opacity
uniform float alpha;
// screen dimensions
uniform vec2 resolution;

// world-space top-left of the current view
uniform vec2 viewTopLeft;
// world-space size of the current view
uniform vec2 viewSize;

// final output pixel color
out vec4 outColor;

// pseudo-random noise using a hash function
// float noise( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

float noise( float a, float b )
{
  uvec2 p = uvec2( uint( a ) * 2654435761u, uint( b ) * 2246822519u );
  p.x ^= p.y;
  p.x *= 2654435761u;
  p.x ^= p.x >> 16u;
  return float( p.x ) / float( 0xFFFFFFFFu );
}

void main()
{
  // Normalise screen coords to UV coords for the current pixel
  vec2 normalizedScreen = gl_FragCoord.xy / resolution;
  normalizedScreen.y = 1.0 - normalizedScreen.y;
  //
  vec2 worldPos = viewTopLeft + normalizedScreen * viewSize;

  // Sample the base texture color at current pixel
  vec4 color = texture2D( texture, worldPos );

  // Create a mighnight blue color tint (39,39,87)
  vec4 tint = vec4( 0.11, 0.11, 0.34, 0.0 ) * 0.75;
  color += tint;

  // Create dynamic static effect:
  // floor(time * speed) gives a discrete integer tick the hash can use
  // higher speed multiplier = faster flickering static
  float timeTick = floor( time * 30.0 );

  vec2 worldOffsetScreen = viewTopLeft * ( resolution / viewSize );

  // Hash timeTick into a random per-frame offset
  // This replaces the pattern each frame without shifting it in any direction
  float frameOffsetX = noise( timeTick, 0.0 ) * 10000.0;
  float frameOffsetY = noise( timeTick, 1.0 ) * 10000.0;

  float simpleNoise = noise( gl_FragCoord.x + floor( worldOffsetScreen.x ) + frameOffsetX,
                             gl_FragCoord.y + floor( worldOffsetScreen.y ) + frameOffsetY );

  // mix noise: brightness + noise * intensity
  color.rgb *= 0.1 + simpleNoise * 0.8;

  // Apply the alpha uniform
  color.a *= alpha;

  outColor = color;
}