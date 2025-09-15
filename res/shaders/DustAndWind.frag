#version 120

uniform sampler2D texture;
uniform float time;
uniform float dustIntensity;
uniform vec2 screenSize;
uniform vec2 windDirection; // Wind direction vector
uniform float windStrength; // Wind strength multiplier

// Improved noise functions
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 127.1, 311.7 ) ) ) * 43758.5453123 ); }

float noise( vec2 p )
{
  vec2 i = floor( p );
  vec2 f = fract( p );

  float a = hash( i );
  float b = hash( i + vec2( 1.0, 0.0 ) );
  float c = hash( i + vec2( 0.0, 1.0 ) );
  float d = hash( i + vec2( 1.0, 1.0 ) );

  vec2 u = f * f * ( 3.0 - 2.0 * f );
  return mix( a, b, u.x ) + ( c - a ) * u.y * ( 1.0 - u.x ) + ( d - b ) * u.x * u.y;
}

// Fractal Brownian Motion for more natural patterns
float fbm( vec2 p )
{
  float value = 0.0;
  float amplitude = 0.5;
  float frequency = 1.0;

  for ( int i = 0; i < 4; i++ )
  {
    value += amplitude * noise( p * frequency );
    amplitude *= 0.5;
    frequency *= 2.0;
  }
  return value;
}

// Wind turbulence function
vec2 windTurbulence( vec2 pos, float t )
{
  vec2 turbulence = vec2( fbm( pos * 0.01 + t * 0.1 ), fbm( pos * 0.01 + vec2( 100.0, 0.0 ) + t * 0.12 ) );
  return ( turbulence - 0.5 ) * 2.0;
}

// Particle density function
float particleDensity( vec2 pos, float t )
{
  // Multiple octaves of noise for particle distribution
  float density = 0.0;

  // Large swirls
  density += fbm( pos * 0.005 + windDirection * t * 0.02 ) * 0.4;

  // Medium dust clouds
  density += fbm( pos * 0.02 + windDirection * t * 0.05 ) * 0.3;

  // Fine particles
  density += noise( pos * 0.08 + windDirection * t * 0.1 ) * 0.2;

  // Very fine dust
  density += noise( pos * 0.15 + windDirection * t * 0.15 ) * 0.1;

  return density;
}

// Individual particle simulation
float dustParticles( vec2 pos, float t )
{
  vec2 windOffset = windDirection * windStrength * t * 0.1;

  float particles = 0.0;

  // Multiple particle layers with different sizes and speeds
  for ( int i = 0; i < 3; i++ )
  {
    float scale = 8.0 + float( i ) * 16.0;
    float speed = 0.5 + float( i ) * 0.3;

    vec2 particlePos = pos * scale / screenSize.x;
    particlePos += windOffset * speed;
    particlePos += windTurbulence( pos, t ) * 0.1;

    vec2 cellPos = floor( particlePos );
    vec2 localPos = fract( particlePos );

    // Random particle position within cell
    vec2 randomOffset = vec2( hash( cellPos + vec2( 0.0, float( i ) ) ), hash( cellPos + vec2( 100.0, float( i ) ) ) );

    vec2 particleCenter = randomOffset;
    float particleSize = 0.1 + hash( cellPos + vec2( 200.0, float( i ) ) ) * 0.4;

    // Distance to particle
    float dist = length( localPos - particleCenter );

    // Particle visibility based on time and position
    float visibility = hash( cellPos + vec2( 300.0, float( i ) ) + floor( t * 2.0 ) );
    visibility = smoothstep( 0.3, 0.7, visibility );

    // Soft particle falloff
    float particle = 1.0 - smoothstep( 0.0, particleSize, dist );
    particle *= particle; // Squared for softer edges

    particles += particle * visibility * ( 0.5 + 0.5 / ( float( i ) + 1.0 ) );
  }

  return particles;
}

// Atmospheric perspective
float atmosphericDepth( vec2 pos )
{
  float depth = fbm( pos * 0.003 + time * 0.01 ) * 0.5 + 0.5;
  return depth;
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;
  vec4 originalColor = texture2D( texture, texCoord );

  vec2 worldPos = texCoord * screenSize;

  // Calculate wind effects
  vec2 windVelocity = windDirection * windStrength;
  vec2 turbulence = windTurbulence( worldPos, time );
  vec2 effectiveWind = windVelocity + turbulence * 0.5;

  // Dust density calculation
  float density = particleDensity( worldPos + effectiveWind * time * 10.0, time );

  // Individual particles
  float particles = dustParticles( worldPos, time );

  // Atmospheric depth for layering
  float depth = atmosphericDepth( worldPos );

  // Combine effects
  float totalDust = density * 0.6 + particles * 0.4;
  totalDust *= depth; // Apply atmospheric perspective

  // Wind streaks - elongated particles in wind direction
  vec2 streakDir = normalize( effectiveWind + vec2( 0.001 ) ); // Avoid division by zero
  float streakNoise = 0.0;

  for ( int i = 0; i < 3; i++ )
  {
    float offset = float( i ) * 0.5;
    vec2 streakPos = worldPos + streakDir * offset * 2.0;
    streakNoise += noise( streakPos * 0.05 + time * 0.2 ) * ( 1.0 / ( float( i ) + 1.0 ) );
  }

  // Apply wind streaks
  totalDust += streakNoise * 0.3 * length( effectiveWind ) * 0.1;

  // Color the dust
  vec3 dustColor = vec3( 0.9, 0.8, 0.6 ); // Sandy/dusty color

  // Vary dust color based on density and wind
  dustColor *= 0.8 + totalDust * 0.4;
  dustColor.r += totalDust * 0.1;
  dustColor.g += totalDust * 0.05;

  // Apply dust effect
  vec3 finalColor = originalColor.rgb;

  // Additive dust particles
  finalColor += dustColor * totalDust * dustIntensity * 0.5;

  // Atmospheric scattering - reduce contrast in dusty areas
  float scattering = totalDust * dustIntensity * 0.3;
  finalColor = mix( finalColor, dustColor, scattering );

  // Darken areas with heavy dust concentration
  float occlusion = 1.0 - totalDust * dustIntensity * 0.2;
  finalColor *= occlusion;

  // Add subtle movement blur in wind direction
  vec2 blurOffset = normalize( effectiveWind ) * totalDust * 0.001;
  vec3 blurSample = texture2D( texture, texCoord + blurOffset ).rgb;
  finalColor = mix( finalColor, blurSample, totalDust * dustIntensity * 0.1 );

  gl_FragColor = vec4( finalColor, originalColor.a );
}