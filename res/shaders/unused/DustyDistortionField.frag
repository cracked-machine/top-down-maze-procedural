#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;
uniform float dustDensity;

// Hash function for pseudo-random numbers
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

// Smooth hash for circular patterns
float smoothHash( vec2 p )
{
  vec2 i = floor( p );
  vec2 f = fract( p );

  // Sample hash at grid corners
  float a = hash( i );
  float b = hash( i + vec2( 1.0, 0.0 ) );
  float c = hash( i + vec2( 0.0, 1.0 ) );
  float d = hash( i + vec2( 1.0, 1.0 ) );

  // Smooth interpolation
  vec2 u = f * f * ( 3.0 - 2.0 * f );
  return mix( a, b, u.x ) + ( c - a ) * u.y * ( 1.0 - u.x ) + ( d - b ) * u.x * u.y;
}

// Circular dust pattern
float createDustPattern( vec2 worldPos )
{
  // Convert to polar coordinates for circular patterns
  vec2 center = worldPos;
  float dist = length( worldPos - floor( worldPos / 32.0 ) * 32.0 - 16.0 ); // Create repeating circular cells
  float angle = atan( worldPos.y, worldPos.x );

  // Scale coordinates based on dust density
  vec2 polarCoord1 = vec2( dist, angle * 4.0 ) * 0.8 * dustDensity;
  vec2 polarCoord2 = vec2( dist, angle * 6.0 ) * 1.5 * dustDensity;
  vec2 polarCoord3 = vec2( dist, angle * 8.0 ) * 2.2 * dustDensity;

  // Simple time-based movement
  vec2 movement1 = vec2( sin( time * 1.5 ), cos( time * 1.2 ) ) * 2.0;
  vec2 movement2 = vec2( cos( time * 2.5 ), sin( time * 2.0 ) ) * 1.5;
  vec2 movement3 = vec2( sin( time * 3.5 ), cos( time * 3.2 ) ) * 1.0;

  // Create circular dust grains using smooth interpolation
  float dust1 = smoothHash( ( polarCoord1 + movement1 ) * 0.1 );
  float dust2 = smoothHash( ( polarCoord2 + movement2 ) * 0.08 );
  float dust3 = smoothHash( ( polarCoord3 + movement3 ) * 0.06 );

  // Add some radial variation
  float radialPattern = sin( dist * 0.5 + time * 2.0 ) * 0.5 + 0.5;

  return ( dust1 * 0.5 + dust2 * 0.3 + dust3 * 0.2 ) * radialPattern;
}

void main()
{
  vec2 texCoord = gl_FragCoord.xy;
  vec2 worldPos = texCoord * screenSize;

  // Convert to centered coordinates (-1 to 1 range)
  vec2 uv = ( texCoord - 0.5 ) * 2.0;

  // Time-based animation
  float t = time * 0.3;

  // Calculate distance and angle from center
  float dist = length( uv );
  float angle = atan( uv.y, uv.x );

  // Create multiple wave distortions (concentric circles)
  float wave1 = sin( dist * 12.0 - t * 2.0 ) * 0.08;
  float wave2 = sin( dist * 20.0 + t * 1.5 ) * 0.04;
  float wave3 = cos( dist * 30.0 - t * 3.0 ) * 0.02;

  // Radial distortion that varies with angle and time
  float radial_distort = sin( angle * 4.0 + t * 1.5 ) * 0.06;

  // Spiral distortion
  float spiral_distort = sin( angle * 3.0 + dist * 10.0 + t * 2.0 ) * 0.05;

  // Combine all distortions
  vec2 distortion = vec2( wave1 + wave2 + radial_distort + spiral_distort, wave1 + wave3 - radial_distort + spiral_distort );

  // Apply polar coordinate distortion
  float polar_distort = sin( dist * 8.0 + angle * 2.0 + t * 2.0 ) * 0.04;
  distortion += vec2( cos( angle ), sin( angle ) ) * polar_distort;

  // Apply distortion to texture coordinates
  vec2 distorted_texCoord = texCoord + distortion * 0.1;

  // Sample texture with distortion
  vec4 color;
  if ( distorted_texCoord.x >= 0.0 && distorted_texCoord.x <= 1.0 && distorted_texCoord.y >= 0.0 && distorted_texCoord.y <= 1.0 )
  {
    color = texture2D( texture, distorted_texCoord );
  }
  else
  {
    color = texture2D( texture, texCoord ); // Fallback to original
  }

  // Apply the same distortion to dust coordinates
  vec2 distorted_worldPos = ( distorted_texCoord * screenSize );

  // Add dust effects using the distorted position
  float dustPattern = createDustPattern( distorted_worldPos );

  // Create brightness variation from dust pattern
  float brightness = 0.9 + dustPattern * 0.2;

  // Apply dust effects
  color.rgb *= brightness;

  // Add subtle dust sparkles (also affected by density)
  float sparkleThreshold = mix( 0.9, 0.6, dustDensity * 0.5 );
  if ( dustPattern > sparkleThreshold ) { color.rgb += vec3( 0.1, 0.08, 0.05 ) * 0.3; }

  gl_FragColor = color;
}