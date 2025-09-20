#version 120

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;

// Hash function for pseudo-random values
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Generate footprint positions based on time in texture coordinate space
vec2 getFootprintPosition( float timeOffset )
{
  // Create a predictable path pattern that looks like walking
  float walkTime = time - timeOffset;

  // Simple figure-8 or circular movement pattern in texture space
  float pathRadius = 0.15;            // Radius in texture coordinates (0.0 to 1.0)
  vec2 pathCenter = vec2( 0.5, 0.5 ); // Always center in texture space

  // Create walking motion with some randomness
  float walkSpeed = 2.0;
  vec2 walkDirection = vec2( cos( walkTime * walkSpeed + sin( walkTime * 0.5 ) * 0.5 ),
                             sin( walkTime * walkSpeed * 0.8 + cos( walkTime * 0.3 ) * 0.3 ) );

  return pathCenter + walkDirection * pathRadius;
}

// Create star shape in texture coordinate space
float starShape( vec2 texCoord, vec2 starCenter, float rotation )
{
  // Translate to star center
  vec2 offset = texCoord - starCenter;

  // Apply rotation
  float s = sin( rotation );
  float c = cos( rotation );
  vec2 rotated = vec2( offset.x * c - offset.y * s, offset.x * s + offset.y * c );

  // Convert star size from world units to texture coordinate units
  float starSize = 2.0 / max( screenSize.x, screenSize.y );

  // Calculate distance and angle from star center
  float dist = length( rotated ) / starSize;
  float angle = atan( rotated.y, rotated.x );

  // Create 5-pointed star shape
  int numPoints = 5;
  float angleStep = 6.28318 / float( numPoints ); // 2*PI / numPoints

  // Calculate star radius based on angle
  float starRadius = 1.0;
  for ( int i = 0; i < numPoints; i++ )
  {
    float pointAngle = float( i ) * angleStep;
    float angleDiff = abs( mod( angle - pointAngle + 3.14159, 6.28318 ) - 3.14159 );

    // Create star points (sharp inner/outer radius variation)
    float innerRadius = 0.4;
    float outerRadius = 1.0;

    if ( angleDiff < angleStep * 0.5 )
    {
      // Near a point - interpolate between inner and outer radius
      float t = angleDiff / ( angleStep * 0.5 );
      float currentRadius = mix( outerRadius, innerRadius, abs( sin( t * 3.14159 ) ) );
      starRadius = min( starRadius, currentRadius );
    }
  }

  // Create smooth star edge
  float starIntensity = 1.0 - smoothstep( starRadius * 0.7, starRadius * 1.2, dist );

  // Add some sparkle variation
  float sparkle = hash( starCenter * 100.0 );
  starIntensity *= ( 0.8 + sparkle * 0.4 );

  return starIntensity;
}

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;

  // Sample original texture
  vec4 color = texture2D( texture, texCoord );

  float starIntensity = 0.0;

  // Generate multiple stars along the trail
  float numStars = 15.0;
  float trailLength = 8.0; // Time span of the trail

  for ( float i = 0.0; i < numStars; i += 1.0 )
  {
    float timeOffset = ( i / numStars ) * trailLength;
    vec2 starPos = getFootprintPosition( timeOffset );

    // Add some random offset to each star
    float randomSeed = i * 17.0;
    vec2 randomOffset = vec2( hash( vec2( randomSeed, randomSeed + 1.0 ) ) - 0.5,
                              hash( vec2( randomSeed + 2.0, randomSeed + 3.0 ) ) - 0.5 ) *
                        0.08; // Small random spread

    vec2 finalStarPos = starPos + randomOffset;

    // Random rotation for each star
    float rotation = hash( vec2( randomSeed + 4.0, randomSeed + 5.0 ) ) * 6.28318;

    // Create star shape
    float star = starShape( texCoord, finalStarPos, rotation );

    // Fade stars over time (newer = more visible)
    float ageFade = 1.0 - ( timeOffset / trailLength );
    star *= ageFade * ageFade; // Quadratic fade

    starIntensity += star;
  }

  // Clamp intensity to avoid oversaturation
  starIntensity = min( starIntensity, 1.0 );

  // Apply star effect to the texture
  if ( starIntensity > 0.0 )
  {
    // Brighten the texture where stars are (opposite of footprints)
    float brightenFactor = 1.3;
    color.rgb = mix( color.rgb, color.rgb * brightenFactor, starIntensity * 0.7 );

    // Add golden sparkle tint to stars
    vec3 starTint = vec3( 1.2, 1.0, 0.6 ); // Golden color
    color.rgb = mix( color.rgb, color.rgb * starTint, starIntensity * 0.5 );

    // Add some white highlights for sparkle effect
    color.rgb += vec3( 1.0, 1.0, 0.8 ) * starIntensity * 0.3;
  }

  gl_FragColor = color;
}