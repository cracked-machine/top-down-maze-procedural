#version 330

// Sampled texture (passed in by SFML)
uniform sampler2D texture;
// elapsed time
uniform float time;

// screen dimensions
uniform vec2 resolution;

// world-space top-left of the current view
uniform vec2 viewTopLeft;
// world-space size of the current view
uniform vec2 viewSize;

// player position in world space
uniform vec2 playerWorldPos;

// // NPC lighting
// uniform int npcCount;
// uniform vec2 npcPositions[8];
// uniform vec2 npcDirections[8];
// uniform float npcTorchLengths[8];
// uniform float npcTorchAngle; // half-angle in radians e.g. 0.4

// final output pixel color
out vec4 outColor;

float noise_sin( float t ) { return sin( t * 6.28318 ) * 0.5 + 0.5; }

float noise( float a, float b )
{
  uvec2 p = uvec2( uint( a ) * 2654435761u, uint( b ) * 2246822519u );
  p.x ^= p.y;
  p.x *= 2654435761u;
  p.x ^= p.x >> 16u;
  return float( p.x ) / float( 0xFFFFFFFFu );
}

float player_torch_pixel( vec2 worldPos )
{
  // radius of torch ring
  float playerTorchRadius = 32.0;

  // Player torch: soft circular ring
  float distToPlayer = length( worldPos - playerWorldPos );

  float flicker_speed = 3.0;
  float flicker_intensity = 0.05;
  float flicker = noise_sin( time * flicker_speed ) * 0.5 + noise_sin( time * flicker_speed * 2.3 ) * 0.3 +
                  noise_sin( time * flicker_speed * 0.7 ) * 0.2;
  float flickered_radius = playerTorchRadius * ( 1.0 + flicker * flicker_intensity );
  float blur_width = 10.0;
  return smoothstep( flickered_radius + blur_width, flickered_radius - blur_width, distToPlayer );
}

void main()
{
  // effect opacity
  float alpha = 0.75;

  vec3 mainColor = vec3( 1, 1, 5 );

  // torch tint
  vec3 warm_yellow = vec3( 2.0, 1.85, 1.4 );
  vec3 cool_white = vec3( 0.9, 0.9, 1.0 );
  vec3 warm_orange = vec3( 1.0, 0.6, 0.2 );
  vec3 eerie_green = vec3( 0.4, 1.0, 0.4 );
  vec3 torchTint = warm_yellow;

  // Normalise screen coords to UV coords for the current pixel
  vec2 normalizedScreen = gl_FragCoord.xy / resolution;
  normalizedScreen.y = 1.0 - normalizedScreen.y;

  vec2 worldPos = viewTopLeft + normalizedScreen * viewSize;

  // Sample the base texture color at current pixel
  vec4 color = texture2D( texture, worldPos );

  // ── Torch / lighting ────────────────────────────────────────────────────────
  float inPlayerTorch = player_torch_pixel( worldPos );

  // // NPC cones: apex at NPC, spreads outward in facing direction
  // float inNpcLight = 0.0;
  // for ( int i = 0; i < npcCount; i++ )
  // {
  //   vec2 toPixel = worldPos - npcPositions[i];
  //   float dist = length( toPixel );
  //   float cosAngle = dot( normalize( toPixel ), npcDirections[i] );

  //   if ( dist < npcTorchLengths[i] && cosAngle > cos( npcTorchAngle ) )
  //   {
  //     // Fade toward edges of cone and toward tip
  //     float angularFade = smoothstep( cos( npcTorchAngle ), cos( npcTorchAngle * 0.5 ), cosAngle );
  //     float distFade = smoothstep( npcTorchLengths[i], npcTorchLengths[i] * 0.2, dist );
  //     inNpcLight = max( inNpcLight, angularFade * distFade );
  //   }
  // }

  // Combined lit amount (player or NPC, take strongest)
  // float litAmount = max( inPlayerTorch, inNpcLight );
  float litAmount = inPlayerTorch;

  // ── Static effect ────────────────────────────────────────────────────────────

  // Bell curve: peaks at the edge of the torch ring, zero at centre and outside
  float edgeTint = litAmount * ( 1.0 - litAmount ) * 4.0;

  color.rgb = mix( color.rgb * mainColor, color.rgb, litAmount );

  float timeTick = floor( time * 30.0 );
  vec2 worldOffsetScreen = viewTopLeft * ( resolution / viewSize );

  float frameOffsetX = noise( timeTick, 0.0 ) * 10000.0;
  float frameOffsetY = noise( timeTick, 1.0 ) * 10000.0;

  float simpleNoise = noise( gl_FragCoord.x + floor( worldOffsetScreen.x ) + frameOffsetX,
                             gl_FragCoord.y + floor( worldOffsetScreen.y ) + frameOffsetY );

  // Dark static — full effect outside, no effect at centre
  float staticBrightness = mix( 0.02 + simpleNoise * 0.15, 1.0, litAmount );
  color.rgb *= staticBrightness;

  // Add static grain on top in dark areas — keeps background dark but grain is visible
  float staticGrain = simpleNoise * 0.12 * ( 1.0 - litAmount );
  color.rgb += staticGrain;

  color.rgb = mix( color.rgb, color.rgb * torchTint, edgeTint * 0.6 );

  // ── Output ───────────────────────────────────────────────────────────────────

  color.a *= alpha;
  outColor = color;
}