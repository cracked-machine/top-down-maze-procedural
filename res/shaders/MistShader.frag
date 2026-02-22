#version 330

uniform sampler2D texture;
uniform float time;
uniform float alpha;
uniform vec2 resolution;

out vec4 outColor;

// Improved noise function for smoother clouds
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 127.1, 311.7 ) ) ) * 43758.5453123 ); }

float noise( vec2 p )
{
  vec2 i = floor( p );
  vec2 f = fract( p );

  f = f * f * ( 3.0 - 2.0 * f );

  float a = hash( i );
  float b = hash( i + vec2( 1.0, 0.0 ) );
  float c = hash( i + vec2( 0.0, 1.0 ) );
  float d = hash( i + vec2( 1.0, 1.0 ) );

  return mix( mix( a, b, f.x ), mix( c, d, f.x ), f.y );
}

// Fractal Brownian Motion for cloud-like appearance
float fbm( vec2 p )
{
  float value = 0.0;
  float amplitude = 0.5;
  float frequency = 1.0;

  for ( int i = 0; i < 5; i++ )
  {
    value += amplitude * noise( p * frequency );
    amplitude *= 0.5;
    frequency *= 2.0;
  }

  return value;
}

void main()
{
  // vec2 texCoord = gl_TexCoord[0].xy;
  vec2 texCoord = gl_FragCoord.xy / resolution;

  // Sample the base texture
  vec4 color = texture2D( texture, texCoord );

  // Create horizontal scrolling right to left (slower)
  // Scale factor controls diffuseness: higher = more diffuse, lower = tighter
  vec2 mistCoord = texCoord * 3.0;
  mistCoord.x -= time * 0.05;  // Slowed down from 0.12
  mistCoord.y += time * 0.008; // Slowed down vertical drift

  // Create rippled pattern with anisotropic scaling
  vec2 rippleCoord = mistCoord;
  rippleCoord.x *= 1.5; // Stretch horizontally for elongated clouds

  // Primary cloud layer with ripple pattern
  float cloud1 = fbm( rippleCoord );

  // Secondary layer for variation
  float cloud2 = fbm( rippleCoord * 1.3 + vec2( 50.0, 25.0 ) );

  // Add fine detail layer
  float detail = fbm( rippleCoord * 3.0 + vec2( 100.0, 50.0 ) );

  // Combine layers to create rippled texture
  float cloudBase = cloud1 * 0.5 + cloud2 * 0.3 + detail * 0.2;

  // Create distinct cloud formations with sharp edges
  float cloudDensity = smoothstep( 0.42, 0.58, cloudBase );

  // Add variation to create gaps between cloud patches
  float patchiness = smoothstep( 0.45, 0.55, cloud2 );
  cloudDensity *= patchiness;

  // Sharpen edges
  cloudDensity = pow( cloudDensity, 1.5 );

  // Apply bright cloud color with slight blue tint
  vec3 mistColor = vec3( 0.95, 0.96, 0.98 );

  // Blend with higher opacity for more visible clouds
  color.rgb = mix( color.rgb, mistColor, cloudDensity * 0.65 );

  // Apply alpha uniform
  color.a *= alpha;

  outColor = color;
}