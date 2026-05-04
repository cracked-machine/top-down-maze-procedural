#version 330

// Sampled texture (passed in by SFML)
uniform sampler2D texture;
// elapsed time
uniform float time;
// effect opacity
uniform float alpha;
// screen dimensions
uniform vec2 resolution;

// final output pixel color
out vec4 outColor;

// pseudo-random noise using a hash function
float noise( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

void main()
{
  // Normalise screen coords to UV coords for the current pixel
  vec2 texCoord = gl_FragCoord.xy / resolution;

  // Sample the base texture color at current pixel
  vec4 color = texture2D( texture, texCoord );

  // Create a mighnight blue color tint (39,39,87)
  vec4 tint = vec4( 0.11, 0.11, 0.34, 0.0 ) * 0.75;
  color += tint;

  // Create dynamic static effect:
  // = number of granules + time * movement speed
  vec2 simpleCoord = texCoord * 5.0 + time * 0.1;
  float simpleNoise = noise( simpleCoord );

  // mix noise: brightness + noise * intensity
  color.rgb *= 0.1 + simpleNoise * 0.8;

  // Apply the alpha uniform
  color.a *= alpha;

  outColor = color;
}