#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;
uniform float vortexStrength;
uniform float vortexRadius;
uniform float spinSpeed;

// Hash function for pseudo-random values
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Generate particle position based on ID and time
vec2 getParticlePosition( float particleId, float timeOffset )
{
  vec2 seed = vec2( particleId, particleId * 1.618 );

  // Initial random position
  vec2 initialPos = vec2( hash( seed ), hash( seed + vec2( 1.0, 0.0 ) ) );

  // Convert to world coordinates relative to center
  vec2 worldPos = ( initialPos - 0.5 ) * 2.0; // -1 to 1 range

  // Apply vortex motion
  float distance = length( worldPos );
  float angle = atan( worldPos.y, worldPos.x );

  // Particles closer to center spin faster
  float spinRate = ( 1.0 - distance ) * spinSpeed + 0.5;
  float newAngle = angle + timeOffset * spinRate;

  // Add some radial movement (particles spiral inward)
  float spiralFactor = 0.1;
  distance -= timeOffset * spiralFactor * ( 1.0 - distance );
  distance = max( distance, 0.1 ); // Don't let particles reach exact center

  // Calculate new position
  vec2 newPos = vec2( cos( newAngle ), sin( newAngle ) ) * distance;

  // Convert back to texture coordinates
  return ( newPos * 0.5 + 0.5 );
}

void main()
{
  vec2 texCoord = gl_FragCoord.xy;

  // Sample original texture
  vec4 color = texture2D( texture, texCoord );

  // Convert vortexRadius from world units to texture coordinate units
  float normalizedRadius = vortexRadius / max( screenSize.x, screenSize.y );

  // Particle system parameters
  float numParticles = 200.0;
  float particleSize = 0.008; // Size in texture coordinates
  float particleIntensity = 0.0;

  // Check for particles at this pixel
  for ( float i = 0.0; i < numParticles; i += 1.0 )
  {
    float particleId = i / numParticles;

    // Calculate particle position with time-based animation
    vec2 particlePos = getParticlePosition( particleId, time * vortexStrength );

    // Check if this pixel is within the particle
    float distToParticle = length( texCoord - particlePos );

    if ( distToParticle < particleSize )
    {
      // Calculate particle alpha based on distance from center
      float alpha = 1.0 - ( distToParticle / particleSize );
      alpha = smoothstep( 0.0, 1.0, alpha );

      // Particle color based on distance from vortex center
      float distFromCenter = length( particlePos - vec2( 0.5, 0.5 ) );
      vec3 particleColor = mix( vec3( 1.0, 0.8, 0.2 ), // Yellow at center
                                vec3( 0.8, 0.2, 1.0 ), // Purple at edges
                                distFromCenter / normalizedRadius );

      // Add particle brightness
      particleIntensity += alpha * 0.8;
      color.rgb += particleColor * alpha * 0.6;
    }
  }

  // Add some glow around particles
  if ( particleIntensity > 0.0 ) { color.rgb += particleIntensity * 0.3 * vec3( 1.0, 0.9, 0.7 ); }

  gl_FragColor = color;
}