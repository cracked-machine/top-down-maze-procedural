#version 120

// A hybrid sand effect where:

// 1. Large particles (grain1 & grain2) - Use fract() for smooth, dust-like continuous movement
// 2. Small particles (grain3) - Use floor() for discrete, wind-blown grain steps
// 3. Wave amplitude - Acts as a "line breaker" that disrupts the linear step artifacts

// This mimics real sand behavior where:
// - Fine dust moves smoothly and continuously
// - Larger grains jump in discrete steps when pushed by wind
// - Strong wind (high amplitude) breaks up visible patterns

// This preserves realistic sand physics (different particle sizes behave differently)
// Uses the wave amplitude strategically to mask artifacts and creates visual complexity with the
// mixed movement types

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform float sandIntensity;
uniform float timeScale;     // Add this uniform
uniform float waveAmplitude; // Wave influence amplitude

// Hash function for pseudo-random numbers
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Hash function for 2D random vectors
vec2 hash2( vec2 p )
{
  return fract(
      sin( vec2( dot( p, vec2( 12.9898, 78.233 ) ), dot( p, vec2( 93.9898, 67.345 ) ) ) ) *
      43758.5453 );
}

vec2 getRandomDirectionWaveSmooth( vec2 coord, float timeOffset )
{
  vec2 pixelPos = floor( coord );

  // Use smaller time steps for more frequent interpolation
  float timeStep = 0.1; // Instead of 1.0, use 0.1 for 10x more interpolation steps
  float currentTime = floor( timeOffset / timeStep ) * timeStep;
  float nextTime = currentTime + timeStep;
  float t = fract( timeOffset / timeStep ); // Interpolation factor [0,1]

  // Calculate angles for current time
  vec2 randomVec1 = hash2( pixelPos + currentTime ) * 2.0 - 1.0;
  float randomAngle1 = atan( randomVec1.y, randomVec1.x );
  float waveAngle1 = sin( pixelPos.x * 0.01 + currentTime * 0.5 ) * 2.0 * waveAmplitude +
                     cos( pixelPos.y * 0.008 + currentTime * 0.3 ) * 1.5 * waveAmplitude;
  float finalAngle1 = randomAngle1 * 0.3 + waveAngle1 * 0.7;

  // Calculate angles for next time
  vec2 randomVec2 = hash2( pixelPos + nextTime ) * 2.0 - 1.0;
  float randomAngle2 = atan( randomVec2.y, randomVec2.x );
  float waveAngle2 = sin( pixelPos.x * 0.01 + nextTime * 0.5 ) * 2.0 * waveAmplitude +
                     cos( pixelPos.y * 0.008 + nextTime * 0.3 ) * 1.5 * waveAmplitude;
  float finalAngle2 = randomAngle2 * 0.3 + waveAngle2 * 0.7;

  // Smoothly interpolate between angles
  float smoothT = smoothstep( 0.0, 1.0, t );
  float finalAngle = mix( finalAngle1, finalAngle2, smoothT );

  return vec2( cos( finalAngle ), sin( finalAngle ) );
}

// Sample texture with random pixel displacement
vec4 sampleWithRandomMovement( vec2 texCoord, float intensity )
{
  vec2 worldPos = texCoord * screenSize;

  // Multiple layers of random movement at different scales
  vec2 largeMovement =
      getRandomDirectionWaveSmooth( worldPos * 0.5, floor( time * timeScale * 2.0 ) ) * intensity *
      3.0;
  vec2 mediumMovement =
      getRandomDirectionWaveSmooth( worldPos * 1.0, floor( time * timeScale * 3.0 ) ) * intensity *
      2.0;
  vec2 fineMovement =
      getRandomDirectionWaveSmooth( worldPos * 2.0, floor( time * timeScale * 4.0 ) ) * intensity *
      1.0;
  vec2 microMovement =
      getRandomDirectionWaveSmooth( worldPos * 4.0, floor( time * timeScale * 6.0 ) ) * intensity *
      0.5;

  // Combine movements
  vec2 totalMovement =
      ( largeMovement + mediumMovement + fineMovement + microMovement ) / screenSize;

  // Sample texture from displaced position
  vec2 displacedCoord = texCoord + totalMovement;

  // Wrap coordinates to avoid sampling outside texture
  displacedCoord = fract( displacedCoord );

  return texture2D( texture, displacedCoord );
}

// Create sand grain pattern from random movement
float createSandPattern( vec2 worldPos )
{
  vec2 grainCoord1 = worldPos * 0.8;
  vec2 grainCoord2 = worldPos * 1.5;
  vec2 grainCoord3 = worldPos * 2.2;

  // Smooth movement calculations
  vec2 movement1 = getRandomDirectionWaveSmooth( grainCoord1, time * timeScale * 1.5 ) * 2.0;
  vec2 movement2 = getRandomDirectionWaveSmooth( grainCoord2, time * timeScale * 2.5 ) * 1.5;
  vec2 movement3 = getRandomDirectionWaveSmooth( grainCoord3, time * timeScale * 3.5 ) * 1.0;

  // PROBLEM: These floor() calls create discrete grid jumps that form lines
  // Try using fract() and scaling instead of floor() for smoother transitions
  float grain1 = hash( fract( ( grainCoord1 + movement1 ) * 0.01 ) );
  float grain2 = hash( fract( ( grainCoord2 + movement2 ) * 0.01 ) );
  float grain3 = hash( floor( ( grainCoord3 + movement3 ) ) );

  return grain1 * 0.5 + grain2 * 0.3 + grain3 * 0.2;
}

// Alternative: Even smoother rotation with interpolation
vec2 getAlternatingRotatingWindDirectionSmooth( vec2 pos, float timeOffset )
{
  vec2 pixelPos = floor( pos );

  // Create slower, smoother interval changes
  float intervalLength = 500.0; // Change direction every 50 time units
  float currentInterval = floor( timeOffset / intervalLength );
  float nextInterval = currentInterval + 1.0;
  float t = fract( timeOffset / intervalLength ); // Interpolation factor

  // Get wind directions for current and next intervals
  float angle1 = hash( vec2( currentInterval, 0.0 ) ) * 6.28318;
  float angle2 = hash( vec2( nextInterval, 0.0 ) ) * 6.28318;

  // Handle angle wrapping for shortest rotation path
  float angleDiff = angle2 - angle1;
  if ( angleDiff > 3.14159 ) angleDiff -= 6.28318;
  if ( angleDiff < -3.14159 ) angleDiff += 6.28318;
  angle2 = angle1 + angleDiff;

  // Smoothly interpolate between angles
  float smoothT = smoothstep( 0.0, 1.0, t );
  float baseAngle = mix( angle1, angle2, smoothT );

  // Add continuous slow rotation
  float continuousRotation = timeOffset * 0.25;
  float finalAngle =
      baseAngle + sin( continuousRotation ) * 1.2 + cos( continuousRotation * 0.8 ) * 0.9;

  // Add position-based flow variation
  finalAngle += sin( pos.x * 0.002 + timeOffset * 0.05 ) * 0.5;
  finalAngle += cos( pos.y * 0.0015 + timeOffset * 0.07 ) * 0.4;

  return vec2( cos( finalAngle ), sin( finalAngle ) );
}

vec2 getSimpleRotatingWindDirectionSmooth( vec2 pos, float timeOffset )
{
  // Simple linear rotation - adjust speed with the multiplier
  float rotationSpeed = 0.1; // Slower rotation
  float angle = timeOffset * rotationSpeed;

  // Optional: Add slight position-based variation for natural flow
  angle += sin( pos.x * 0.001 ) * 0.2 + cos( pos.y * 0.0008 ) * 0.15;

  return vec2( cos( angle ), sin( angle ) );
}

// Update the main wind movement function to use smooth rotation
vec2 getWindInfluencedMovement( vec2 pos )
{
  // Use the smooth rotating wind direction
  vec2 windDir = getAlternatingRotatingWindDirectionSmooth( pos, time * timeScale );

  // Random component for natural variation
  vec2 randomDir = getRandomDirectionWaveSmooth( pos, time * timeScale * 1.8 );

  // Combine with stronger wind bias for more directional flow
  return normalize( windDir * 0.85 + randomDir * 0.15 );
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;
  vec2 worldPos = texCoord * screenSize;

  // Sample original texture
  vec4 originalColor = texture2D( texture, texCoord );

  // Sample texture with random pixel movement
  float movementIntensity = sandIntensity; // Control movement strength
  vec4 movedColor = sampleWithRandomMovement( texCoord, movementIntensity );

  // Create sand grain pattern from random movements
  float sandPattern = createSandPattern( worldPos );

  // Create brightness variation from grain pattern
  float brightness = 0.85 + sandPattern * 0.3;

  // Sandy color variation
  vec3 sandColor =
      vec3( 1.0 + sandPattern * 0.2, 1.0 + sandPattern * 0.1, 0.95 - sandPattern * 0.05 );

  // Blend original and moved colors based on sand pattern
  vec4 color = mix( originalColor, movedColor, sandPattern * sandIntensity * 0.6 );

  // Apply sand effects
  color.rgb *= brightness * sandColor;

  // Add random sparkles from moving grains
  vec2 sparkleCoord = worldPos * 3.0;
  vec2 sparkleMovement = getWindInfluencedMovement( sparkleCoord ) * time * timeScale * 0.5;
  float sparkle = hash( floor( sparkleCoord + sparkleMovement ) );

  if ( sparkle > 0.97 ) { color.rgb += vec3( 0.1, 0.08, 0.05 ) * sandIntensity; }

  // Add moving grain trails
  vec2 trailPos = worldPos * 1.5;
  vec2 trailMovement = getWindInfluencedMovement( trailPos ) * time * timeScale * 0.3;
  float trail = hash( floor( trailPos + trailMovement ) );

  if ( trail > 0.9 && sandPattern > 0.5 ) { color.rgb += vec3( 0.05, 0.04, 0.02 ) * sandIntensity; }

  // Final blend with original texture
  color.rgb = mix( originalColor.rgb, color.rgb, sandIntensity );

  gl_FragColor = color;
}