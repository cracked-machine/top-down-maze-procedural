#version 330

uniform sampler2D texture;
uniform float time;
uniform vec2 resolution;
uniform float waterLevel;
uniform vec2 viewTopLeft;
uniform vec2 viewSize;
uniform vec2 mapSize;

out vec4 outColor;

float rand( float a, float b )
{
  uvec2 p = uvec2( uint( a ) * 2654435761u, uint( b ) * 2246822519u );
  p.x ^= p.y;
  p.x *= 2654435761u;
  p.x ^= p.x >> 16u;
  return float( p.x ) / float( 0xFFFFFFFFu );
}
float driftNoise( float layerSeed, float t )
{
  float i = floor( t );
  float f = fract( t );
  f = f * f * ( 3.0 - 2.0 * f );
  float a = fract( sin( dot( vec2( layerSeed, i ), vec2( 127.1, 311.7 ) ) ) * 43758.5453 );
  float b = fract( sin( dot( vec2( layerSeed, i + 1.0 ), vec2( 127.1, 311.7 ) ) ) * 43758.5453 );
  return mix( a, b, f ) * 2.0 - 1.0;
}

vec3 waveColor( int layer, int totalLayers )
{
  float t = float( layer ) / float( totalLayers - 1 );
  vec3 dark = vec3( 0.05, 0.35, 0.55 );
  vec3 mid = vec3( 0.18, 0.60, 0.78 );
  vec3 light = vec3( 0.55, 0.82, 0.90 );
  if ( t < 0.5 )
    return mix( dark, mid, t * 2.0 );
  else
    return mix( mid, light, ( t - 0.5 ) * 2.0 );
}

void main()
{
  vec2 normalizedScreen = gl_FragCoord.xy / resolution;
  normalizedScreen.y = 1.0 - normalizedScreen.y;
  vec2 worldPos = viewTopLeft + normalizedScreen * viewSize;

  float normX = worldPos.x / mapSize.x;
  float normY = worldPos.y / mapSize.y;

  const int NUM_LAYERS = 160;
  float layerHeight = 1.0 / float( NUM_LAYERS );

  vec3 finalColor = vec3( 0.05, 0.35, 0.55 );

  for ( int i = 0; i < NUM_LAYERS; i++ )
  {
    float fi = float( i ); // was missing!

    float staticBand = 1.0 - fi * layerHeight;
    float driftSpeed = 0.02 + rand( fi, 8.0 ) * 0.08;
    float driftPhase = rand( fi, 9.0 ) * 6.28318;
    float driftFreq = 1.0 + rand( fi, 80.0 ) * 4.0; // 1..5 oscillations per drift cycle
    float driftAmp = layerHeight;
    float bandTop = staticBand + sin( time * driftSpeed * driftFreq + driftPhase ) * driftAmp;

    float speed = ( rand( fi, 1.0 ) * 2.0 - 1.0 ) * 0.6;
    float freq = 50.0 + rand( fi, 2.0 ) * 40.0;
    float amp = 0.002 + rand( fi, 3.0 ) * 0.002;
    float phase = rand( fi, 4.0 ) * 6.28318;
    float phase2 = rand( fi, 5.0 ) * 6.28318;
    float phase3 = rand( fi, 6.0 ) * 6.28318;

    float crest = bandTop + sin( normX * freq * 6.28 + time * speed + phase ) * amp +
                  sin( normX * freq * 4.71 - time * speed * 0.7 + phase2 ) * amp * 0.15 +
                  sin( normX * freq * 9.42 + time * speed * 1.3 + phase3 ) * amp * 0.08;

    if ( normY < crest )
    {
      vec3 layerColor = waveColor( i, NUM_LAYERS );
      float distToCrest = crest - normY;
      float highlight = smoothstep( 0.01, 0.0, distToCrest );
      layerColor += vec3( 0.35, 0.35, 0.35 ) * highlight;

      float layerAlpha = 0.4 + rand( fi, 5.7 ) * 0.3;
      finalColor = mix( finalColor, layerColor, layerAlpha );
    }
  }

  outColor = vec4( finalColor, 0.85 );
}