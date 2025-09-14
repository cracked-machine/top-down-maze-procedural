#version 120

uniform sampler2D texture;
uniform float time;
uniform float sandIntensity;
uniform vec2 screenSize; // New uniform for actual screen dimensions

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

// Fractal Brownian Motion combining multiple octaves of noise
float fbm( vec2 st )
{
  float value = 0.0;
  float amplitude = 0.5;

  for ( int i = 0; i < 5; i++ )
  {
    value += amplitude * smoothNoise( st );
    st *= 2.0;
    amplitude *= 0.5;
  }
  return value;
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;

  // Sample the texture normally
  vec4 color = texture2D( texture, texCoord );

  // Use actual screen size for consistent density across the entire display
  vec2 pixelCoord = texCoord * screenSize;

  // Scale to maintain same density as before (adjust 0.007 as needed)
  vec2 noiseCoord = pixelCoord * 0.007; // Use non-divisible scale

  // Time-based animation with slowly changing direction
  float t = time * 0.2;

  // Create slowly rotating wind direction
  float windAngle1 = time * 0.1;         // Very slow rotation
  float windAngle2 = time * 0.15 + 10.5; // Different rotation speed and offset
  float windAngle3 = time * 0.08 + 3.0;  // Another variation

  // Convert angles to direction vectors
  vec2 windDir1 = vec2( cos( windAngle1 ), sin( windAngle1 ) );
  vec2 windDir2 = vec2( cos( windAngle2 ), sin( windAngle2 ) );
  vec2 windDir3 = vec2( cos( windAngle3 ), sin( windAngle3 ) );

  // Create flowing sand patterns with rotating wind directions
  float wave1 = fbm( noiseCoord + vec2( 37.3, 73.7 ) + windDir1 * t * 2.0 );
  float wave2 = fbm( noiseCoord * 1.31 + vec2( 19.4, 41.2 ) + windDir2 * t * 0.8 );
  float wave3 = fbm( noiseCoord * 0.73 + vec2( 67.1, 23.9 ) + windDir3 * t * 0.6 );

  // High frequency sand grain with rotating directions
  float grain1 = smoothNoise( noiseCoord * 3.11 + vec2( 11.7, 29.3 ) + windDir1 * t * 3.0 );
  float grain2 = smoothNoise( noiseCoord * 11.37 + vec2( 47.2, 83.1 ) + windDir2 * t * 2.8 );
  float grain3 = smoothNoise( noiseCoord * 13.91 + vec2( 59.8, 17.4 ) + windDir3 * t * 2.5 );

  // Combine all patterns
  float sandPattern = ( wave1 + wave2 * 0.7 + wave3 * 0.5 ) / 2.2;
  float sandGrain = ( grain1 + grain2 * 0.8 + grain3 * 0.6 ) / 2.4;

  // Create sand-like brightness variation
  float brightness = 0.85 + sandPattern * 0.2 + sandGrain * 0.15;
  color.rgb *= brightness;

  // Sandy color variation with rotating direction
  float colorNoise1 = fbm( noiseCoord * 0.11 + vec2( 31.5, 97.2 ) + windDir1 * time * 0.1 );

  // Warm sandy color shifts
  vec3 sandColor =
      vec3( 1.0 + colorNoise1 * 0.15 + sandGrain * 0.1, 1.0 + colorNoise1 * 0.08 + sandGrain * 0.05, 1.0 - colorNoise1 * 0.05 + sandGrain * 0.02 );

  // Apply sand coloring
  color.rgb *= mix( vec3( 1.0 ), sandColor, sandIntensity * 0.5 );

  // Add sand grain overlay
  float grainOverlay = 0.9 + sandGrain * 0.2;
  color.rgb *= mix( vec3( 1.0 ), vec3( grainOverlay ), sandIntensity * 0.4 );

  // Add sand particle details with rotating direction
  float particleDetail = smoothNoise( noiseCoord * 17.43 + vec2( 71.6, 13.8 ) + windDir2 * time * 4.0 );
  float shadows = mix( 1.0, 0.85 + particleDetail * 0.3, sandIntensity * 0.3 );
  color.rgb *= shadows;

  gl_FragColor = color;
}