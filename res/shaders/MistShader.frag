#version 330

uniform sampler2D tex;
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
  vec2 texCoord = gl_FragCoord.xy / resolution;

  vec2 mistCoord = texCoord * 3.0;
  mistCoord.x -= time * 0.05;
  mistCoord.y += time * 0.008;

  vec2 rippleCoord = vec2( mistCoord.x * 1.5, mistCoord.y );

  float cloud1 = fbm( rippleCoord );
  float cloud2 = fbm( rippleCoord * 1.3 + vec2( 50.0, 25.0 ) );
  float detail = fbm( rippleCoord * 3.0 + vec2( 100.0, 50.0 ) );

  float cloudBase = cloud1 * 0.5 + cloud2 * 0.3 + detail * 0.2;
  float cloudDensity = smoothstep( 0.42, 0.58, cloudBase );
  cloudDensity *= smoothstep( 0.45, 0.55, cloud2 );
  cloudDensity = pow( cloudDensity, 1.5 );

  // No tex sampling - pure procedural noise fixed to screen space
  outColor = vec4( vec3( 0.95, 0.96, 0.98 ), cloudDensity * 0.65 * alpha );
}