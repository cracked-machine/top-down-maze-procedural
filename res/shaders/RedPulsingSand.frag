#version 330

uniform sampler2D texture;
uniform float time;
uniform float alpha;
uniform vec2 resolution;

out vec4 outColor;

// pseudo-random noise using a hash function
float noise( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

// Bilinear interpolation between noise values for smooth transitions
float smoothNoise( vec2 st )
{
  vec2 i = floor( st );
  vec2 f = fract( st );

  float a = noise( i );
  float b = noise( i + vec2( 1.0, 0.0 ) );
  float c = noise( i + vec2( 0.0, 1.0 ) );
  float d = noise( i + vec2( 1.0, 1.0 ) );

  vec2 u = f * f * ( 3.0 - 2.0 * f );

  return mix( a, b, u.x ) + ( c - a ) * u.y * ( 1.0 - u.x ) + ( d - b ) * u.x * u.y;
}

void main()
{
  vec2 texCoord = gl_FragCoord.xy / resolution;

  // Sample the texture normally
  vec4 color = texture2D( texture, texCoord );

  // DEBUGGING: Add visible effect to confirm shader is working
  float pulse = sin( time * 2.0 ) * 0.5 + 0.5;
  color.b += pulse * 0.2;

  vec2 simpleCoord = texCoord * 50.0 + time * 0.1;
  float simpleNoise = noise( simpleCoord );

  // Apply brightness variation
  color.rgb *= 0.8 + simpleNoise * 0.4;

  // Apply the alpha uniform
  color.a *= alpha;

  outColor = color;
}