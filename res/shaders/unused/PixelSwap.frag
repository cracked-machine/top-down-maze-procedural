#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 textureSize;
uniform float swapIntensity; // Controls how many pixels swap simultaneously (0.0 to 1.0)

// Simple random function
float random( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

// Better random with seed
float random( vec2 st, float seed ) { return fract( sin( dot( st.xy + seed, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

void main()
{
  vec2 texCoord = gl_FragCoord.xy;
  vec2 pixelSize = 1.0 / textureSize;

  // Get current pixel position
  vec2 pixelPos = floor( texCoord * textureSize );

  // Create time-based animation cycles that change every few frames
  float timeScale = floor( time * 10.0 ); // Change swap pattern 10 times per second

  // Determine if this pixel should swap based on swapIntensity
  float shouldSwap = step( 1.0 - swapIntensity, random( pixelPos, timeScale ) );

  vec2 finalTexCoord = texCoord;

  if ( shouldSwap > 0.5 )
  {
    // Choose random adjacent direction
    float dirRandom = random( pixelPos + vec2( timeScale * 0.7, timeScale * 1.3 ) );
    vec2 swapDirection = vec2( 0.0 );

    if ( dirRandom < 0.25 )
    {
      swapDirection = vec2( 1.0, 0.0 ); // right
    }
    else if ( dirRandom < 0.5 )
    {
      swapDirection = vec2( -1.0, 0.0 ); // left
    }
    else if ( dirRandom < 0.75 )
    {
      swapDirection = vec2( 0.0, 1.0 ); // down
    }
    else
    {
      swapDirection = vec2( 0.0, -1.0 ); // up
    }

    // Apply the swap offset
    vec2 swapOffset = swapDirection * pixelSize;
    finalTexCoord += swapOffset;

    // Ensure we stay within texture bounds
    finalTexCoord = clamp( finalTexCoord, vec2( 0.0 ), vec2( 1.0 ) );
  }

  // Sample the texture at the final coordinate
  vec4 color = texture2D( texture, finalTexCoord );

  gl_FragColor = color;
}