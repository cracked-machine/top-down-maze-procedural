#version 120

uniform sampler2D texture;
uniform float time;
uniform float sandIntensity;

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

// Fractal Brownian Motion with domain warping to break repetition
float fbm( vec2 st )
{
  float value = 0.0;
  float amplitude = 0.5;

  // Add domain warping to break repetition
  st += smoothNoise( st * 0.1 ) * 2.0;

  for ( int i = 0; i < 4; i++ )
  {
    value += amplitude * smoothNoise( st );
    st *= 2.07; // Use non-power-of-2 scaling
    amplitude *= 0.5;
  }
  return value;
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;

  // Sample the texture normally
  vec4 color = texture2D( texture, texCoord );

  // Use pixel coordinates with better scaling to reduce repetition
  vec2 pixelCoord = texCoord * vec2( 3200.0, 1568.0 );

  // Use multiple prime number scales to avoid alignment
  vec2 noiseCoord1 = pixelCoord * 0.00723; // Prime-based scaling
  vec2 noiseCoord2 = pixelCoord * 0.00541;
  vec2 noiseCoord3 = pixelCoord * 0.00613;

  // Time-based animation
  float t = time * 0.15;

  // Create wind directions with different phases
  float windAngle1 = time * 0.083 + 2.147;
  float windAngle2 = time * 0.127 + 5.283;

  vec2 windDir1 = vec2( cos( windAngle1 ), sin( windAngle1 ) );
  vec2 windDir2 = vec2( cos( windAngle2 ), sin( windAngle2 ) );

  // Domain warping coordinates to break repetition
  vec2 warp1 = vec2( smoothNoise( noiseCoord1 * 0.3 + time * 0.02 ), smoothNoise( noiseCoord1 * 0.3 + time * 0.02 + 100.0 ) ) * 3.0;
  vec2 warp2 = vec2( smoothNoise( noiseCoord2 * 0.2 + time * 0.015 ), smoothNoise( noiseCoord2 * 0.2 + time * 0.015 + 200.0 ) ) * 2.0;

  // Create sand patterns with domain warping
  float wave1 = fbm( noiseCoord1 + warp1 + windDir1 * t * 1.3 );
  float wave2 = fbm( noiseCoord2 + warp2 + windDir2 * t * 0.7 );

  // MASSIVELY increased grain frequency for much smaller grains
  float grain1 = smoothNoise( noiseCoord1 * 45.73 + warp1 * 0.5 + windDir1 * t * 2.1 ); // was 7.19
  float grain2 = smoothNoise( noiseCoord2 * 78.41 + warp2 * 0.3 + windDir2 * t * 1.8 ); // was 11.73
  float grain3 = smoothNoise( noiseCoord3 * 112.89 + windDir1 * t * 2.3 );              // was 13.41

  // Add even finer grain details
  float veryFineGrain1 = smoothNoise( noiseCoord1 * 156.23 + warp1 * 0.2 + windDir1 * t * 3.1 );
  float veryFineGrain2 = smoothNoise( noiseCoord2 * 203.47 + warp2 * 0.15 + windDir2 * t * 2.9 );
  float ultraFineGrain = smoothNoise( noiseCoord3 * 287.91 + windDir1 * t * 3.5 );

  // Combine patterns with better weighting
  float sandPattern = ( wave1 * 0.6 + wave2 * 0.4 );
  float sandGrain = ( grain1 * 0.3 + grain2 * 0.25 + grain3 * 0.2 + veryFineGrain1 * 0.15 + veryFineGrain2 * 0.1 + ultraFineGrain * 0.05 );

  // Create sand-like brightness variation with enhanced grain effect
  float brightness = 0.88 + sandPattern * 0.15 + sandGrain * 0.25; // Increased grain influence
  color.rgb *= brightness;

  // Subtle color variation
  float colorNoise = fbm( noiseCoord1 * 0.13 + windDir1 * time * 0.08 );

  vec3 sandColor =
      vec3( 1.0 + colorNoise * 0.08 + sandGrain * 0.12, 1.0 + colorNoise * 0.04 + sandGrain * 0.08, 1.0 - colorNoise * 0.02 + sandGrain * 0.04 );

  // Apply effects with enhanced grain visibility
  color.rgb *= mix( vec3( 1.0 ), sandColor, sandIntensity * 0.6 ); // Increased from 0.4

  // Enhanced grain overlay for more visible texture
  float grainOverlay = 0.88 + sandGrain * 0.35;                               // Increased grain effect
  color.rgb *= mix( vec3( 1.0 ), vec3( grainOverlay ), sandIntensity * 0.5 ); // Increased from 0.3

  gl_FragColor = color;
}