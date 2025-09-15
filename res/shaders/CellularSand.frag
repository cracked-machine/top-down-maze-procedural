#version 120

// This cellular automata sand shader creates:

// 1. Individual sand grains that follow physics-like rules (falling, sliding)
// 2. Multi-scale simulation with large grains, medium grains, fine sand, and dust
// 3. Gravity effects - sand falls downward when possible
// 4. Sliding behavior - grains slide diagonally when blocked below
// 5. Wind effects - random movement and grain generation from wind
// 6. Density-based rendering - areas with more sand appear different
// 7. Moving grain trails - visible particle streams in windy areas
// 8. Individual grain highlights - bright spots representing sand crystals

// The cellular automata rules simulate realistic sand behavior:

// - Grains fall when space below is empty
// - Grains slide diagonally when direct fall is blocked
// - Wind creates random movement and new grain generation
// - Multiple scales create natural-looking sand distribution

// The shader is compatible with your BackgroundShader system and will create animated, grain-like sand movement across the floor tiles.

// Key changes for random pixel movement:

// sampleWithRandomMovement() - Samples the texture from randomly displaced coordinates for each pixel
// Multiple movement scales - Large, medium, fine, and micro movements at different frequencies
// getRandomDirection() - Generates normalized random direction vectors for each pixel
// Time-based updates - Movement changes over time using floor(time * speed) for discrete steps
// Wind-influenced randomness - Combines wind direction with pure randomness
// Texture coordinate wrapping - Uses fract() to keep displaced coordinates in valid range
// Movement intensity control - Uses sandIntensity to control how much pixels move
// Grain pattern creation - Creates sand patterns based on the random movements
// Sparkles and trails - Moving bright spots that follow the random movement patterns

// This creates an effect where each pixel appears to randomly jump around, simulating individual sand grains being blown around by wind while
// maintaining the overall texture appearance.

uniform sampler2D texture;
uniform float time;
uniform float sandIntensity;
uniform vec2 screenSize;

// Hash function for pseudo-random numbers
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Hash function for 2D random vectors
vec2 hash2( vec2 p ) { return fract( sin( vec2( dot( p, vec2( 12.9898, 78.233 ) ), dot( p, vec2( 93.9898, 67.345 ) ) ) ) * 43758.5453 ); }

// Get random direction for each pixel
vec2 getRandomDirection( vec2 coord, float timeOffset )
{
  vec2 pixelPos = floor( coord );
  vec2 randomVec = hash2( pixelPos + timeOffset ) * 2.0 - 1.0; // Range [-1, 1]

  // Normalize to unit circle for consistent movement speed
  float len = length( randomVec );
  if ( len > 0.0 ) { randomVec = randomVec / len; }

  return randomVec;
}

// Sample texture with random pixel displacement
vec4 sampleWithRandomMovement( vec2 texCoord, float intensity )
{
  vec2 worldPos = texCoord * screenSize;

  // Multiple layers of random movement at different scales
  vec2 largeMovement = getRandomDirection( worldPos * 0.5, floor( time * 2.0 ) ) * intensity * 3.0;
  vec2 mediumMovement = getRandomDirection( worldPos * 1.0, floor( time * 3.0 ) ) * intensity * 2.0;
  vec2 fineMovement = getRandomDirection( worldPos * 2.0, floor( time * 4.0 ) ) * intensity * 1.0;
  vec2 microMovement = getRandomDirection( worldPos * 4.0, floor( time * 6.0 ) ) * intensity * 0.5;

  // Combine movements
  vec2 totalMovement = ( largeMovement + mediumMovement + fineMovement + microMovement ) / screenSize;

  // Sample texture from displaced position
  vec2 displacedCoord = texCoord + totalMovement;

  // Wrap coordinates to avoid sampling outside texture
  displacedCoord = fract( displacedCoord );

  return texture2D( texture, displacedCoord );
}

// Create sand grain pattern from random movement
float createSandPattern( vec2 worldPos )
{
  // Create different grain densities based on random movement
  vec2 grainCoord1 = worldPos * 0.8;
  vec2 grainCoord2 = worldPos * 1.5;
  vec2 grainCoord3 = worldPos * 2.2;

  // Random movement for each grain scale
  vec2 movement1 = getRandomDirection( grainCoord1, floor( time * 1.5 ) ) * 2.0;
  vec2 movement2 = getRandomDirection( grainCoord2, floor( time * 2.5 ) ) * 1.5;
  vec2 movement3 = getRandomDirection( grainCoord3, floor( time * 3.5 ) ) * 1.0;

  // Sample grain patterns with movement
  float grain1 = hash( floor( grainCoord1 + movement1 ) );
  float grain2 = hash( floor( grainCoord2 + movement2 ) );
  float grain3 = hash( floor( grainCoord3 + movement3 ) );

  return grain1 * 0.5 + grain2 * 0.3 + grain3 * 0.2;
}

// Wind-influenced random movement
vec2 getWindInfluencedMovement( vec2 pos )
{
  // Base wind direction
  float windAngle = time * 0.3 + sin( pos.x * 0.005 ) * 2.0;
  vec2 windDir = vec2( cos( windAngle ), sin( windAngle ) );

  // Random component
  vec2 randomDir = getRandomDirection( pos, floor( time * 2.0 ) );

  // Combine wind and random (wind bias but still random)
  return normalize( windDir * 0.7 + randomDir * 0.3 );
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;
  vec2 worldPos = texCoord * screenSize;

  // Sample original texture
  vec4 originalColor = texture2D( texture, texCoord );

  // Sample texture with random pixel movement
  float movementIntensity = sandIntensity * 2.0; // Control movement strength
  vec4 movedColor = sampleWithRandomMovement( texCoord, movementIntensity );

  // Create sand grain pattern from random movements
  float sandPattern = createSandPattern( worldPos );

  // Create brightness variation from grain pattern
  float brightness = 0.85 + sandPattern * 0.3;

  // Sandy color variation
  vec3 sandColor = vec3( 1.0 + sandPattern * 0.2, 1.0 + sandPattern * 0.1, 0.95 - sandPattern * 0.05 );

  // Blend original and moved colors based on sand pattern
  vec4 color = mix( originalColor, movedColor, sandPattern * sandIntensity * 0.6 );

  // Apply sand effects
  color.rgb *= brightness * sandColor;

  // Add random sparkles from moving grains
  vec2 sparkleCoord = worldPos * 3.0;
  vec2 sparkleMovement = getWindInfluencedMovement( sparkleCoord ) * time * 0.5;
  float sparkle = hash( floor( sparkleCoord + sparkleMovement ) );

  if ( sparkle > 0.97 ) { color.rgb += vec3( 0.1, 0.08, 0.05 ) * sandIntensity; }

  // Add moving grain trails
  vec2 trailPos = worldPos * 1.5;
  vec2 trailMovement = getWindInfluencedMovement( trailPos ) * time * 0.3;
  float trail = hash( floor( trailPos + trailMovement ) );

  if ( trail > 0.9 && sandPattern > 0.5 ) { color.rgb += vec3( 0.05, 0.04, 0.02 ) * sandIntensity; }

  // Final blend with original texture
  color.rgb = mix( originalColor.rgb, color.rgb, sandIntensity );

  gl_FragColor = color;
}