#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;
uniform vec2 playerPosition; // Add player position uniform

// Hash function for pseudo-random values
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Generate footprint positions based on player movement history
vec2 getFootprintPosition( float timeOffset )
{
  float walkTime = time - timeOffset;

  // Calculate where the player was in the past based on current position and time
  // This creates a simple backward-projected trail
  vec2 movementVector = vec2( sin( walkTime * 1.2 ) * 8.0, // X movement pattern
                              cos( walkTime * 0.8 ) * 6.0  // Y movement pattern
  );

  // Convert player position to texture coordinate space
  vec2 playerTexCoord = ( playerPosition - centerPosition ) / screenSize + vec2( 0.5, 0.5 );

  // Apply movement offset (scaled to texture space)
  vec2 movementTexCoord = movementVector / screenSize;

  return playerTexCoord - movementTexCoord;
}

// Create realistic footprint shape
float footprintShape( vec2 texCoord, vec2 footCenter, float rotation, bool isLeftFoot )
{
  // Check if footprint is within texture bounds
  if ( footCenter.x < 0.0 || footCenter.x > 1.0 || footCenter.y < 0.0 || footCenter.y > 1.0 ) { return 0.0; }

  // Translate to foot center
  vec2 offset = texCoord - footCenter;

  // Apply rotation
  float s = sin( rotation );
  float c = cos( rotation );
  vec2 rotated = vec2( offset.x * c - offset.y * s, offset.x * s + offset.y * c );

  // Convert foot dimensions from world units to texture coordinates
  float footLength = 8.0 / max( screenSize.x, screenSize.y );
  float footWidth = 4.0 / max( screenSize.x, screenSize.y );
  float heelWidth = 3.5 / max( screenSize.x, screenSize.y );
  float toeWidth = 2.5 / max( screenSize.x, screenSize.y );

  // Create foot outline (heel + arch + toes)
  float footprint = 0.0;

  // Heel (circular back part)
  vec2 heelCenter = vec2( 0.0, -footLength * 0.35 );
  float heelDist = length( rotated - heelCenter ) / ( heelWidth * 0.5 );
  float heel = 1.0 - smoothstep( 0.8, 1.2, heelDist );

  // Arch (elongated middle part)
  vec2 archCenter = vec2( 0.0, 0.0 );
  float archDist = length( vec2( rotated.x / ( footWidth * 0.4 ), ( rotated.y - archCenter.y ) / ( footLength * 0.4 ) ) );
  float arch = 1.0 - smoothstep( 0.7, 1.1, archDist );

  // Toes (front oval part)
  vec2 toeCenter = vec2( 0.0, footLength * 0.3 );
  float toeDist = length( vec2( rotated.x / ( toeWidth * 0.5 ), ( rotated.y - toeCenter.y ) / ( footLength * 0.25 ) ) );
  float toes = 1.0 - smoothstep( 0.8, 1.2, toeDist );

  // Individual toe marks
  for ( int i = 0; i < 5; i++ )
  {
    float toeX = ( float( i ) - 2.0 ) * footWidth * 0.15;
    float toeY = footLength * 0.4;
    vec2 toePos = vec2( toeX, toeY );
    float toeSize = mix( 0.4, 0.7, abs( float( i ) - 2.0 ) / 2.0 ); // Big toe larger

    float toeDist = length( rotated - toePos ) / ( footWidth * 0.1 * toeSize );
    float toe = 1.0 - smoothstep( 0.5, 1.0, toeDist );
    footprint = max( footprint, toe * 0.8 );
  }

  // Combine all parts
  footprint = max( footprint, max( heel, max( arch * 0.6, toes ) ) );

  // Add some texture variation
  float noise = hash( footCenter * 50.0 + rotated * 20.0 );
  footprint *= ( 0.8 + noise * 0.4 );

  return footprint;
}

void main()
{
  vec2 texCoord = gl_FragCoord.xy;

  // Sample original texture
  vec4 color = texture2D( texture, texCoord );

  float footprintIntensity = 0.0;

  // Generate footprints along the trail
  float numFootprints = 20.0;
  float trailLength = 8.0; // Time span of the trail in seconds

  for ( float i = 0.0; i < numFootprints; i += 1.0 )
  {
    float timeOffset = ( i / numFootprints ) * trailLength;
    vec2 footPos = getFootprintPosition( timeOffset );

    // Alternate left and right foot
    bool isLeftFoot = mod( i, 2.0 ) < 1.0;
    float footSide = isLeftFoot ? -1.0 : 1.0;

    // Side offset in texture coordinate space
    float sideOffsetDistance = 3.0 / max( screenSize.x, screenSize.y );
    vec2 sideOffset = vec2( footSide * sideOffsetDistance, 0.0 );

    // Calculate walking direction for footprint rotation
    vec2 walkDir = normalize( getFootprintPosition( timeOffset ) - getFootprintPosition( timeOffset + 0.2 ) );
    float rotation = atan( walkDir.y, walkDir.x );

    // Apply side offset with rotation
    float s = sin( rotation );
    float c = cos( rotation );
    vec2 rotatedSideOffset = vec2( sideOffset.x * c - sideOffset.y * s, sideOffset.x * s + sideOffset.y * c );

    vec2 finalFootPos = footPos + rotatedSideOffset;

    // Create footprint shape
    float footprint = footprintShape( texCoord, finalFootPos, rotation, isLeftFoot );

    // Fade footprints over time (exponential fade)
    float ageFade = exp( -timeOffset / 4.0 ); // Fade over 4 seconds
    footprint *= ageFade;

    footprintIntensity += footprint;
  }

  // Clamp intensity to avoid oversaturation
  footprintIntensity = min( footprintIntensity, 1.0 );

  // Apply footprint effect - darken the sand/ground
  if ( footprintIntensity > 0.0 )
  {
    // Create the depression effect by darkening
    float darkenFactor = 0.5;
    color.rgb = mix( color.rgb, color.rgb * darkenFactor, footprintIntensity * 0.8 );

    // Add slight brown tint to simulate wet/compressed sand
    vec3 footprintTint = vec3( 0.7, 0.6, 0.4 );
    color.rgb = mix( color.rgb, color.rgb * footprintTint, footprintIntensity * 0.4 );

    // Add subtle shadow effect around edges
    color.rgb -= vec3( 0.1, 0.08, 0.05 ) * footprintIntensity * 0.3;
  }

  gl_FragColor = color;
}