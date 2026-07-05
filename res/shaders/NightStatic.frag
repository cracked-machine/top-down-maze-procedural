#version 330

// Sampled texture (passed in by SFML)
uniform sampler2D texture;
// elapsed time
uniform float time;

// screen dimensions
uniform vec2 resolution;

// world-space top-left of the current view
uniform vec2 view_top_left;
// world-space size of the current view
uniform vec2 view_size;

// The overal size of the torchlight radius
uniform float torch_radius;

// player position in world space
const int MAX_TORCH_COUNT = 40;
uniform int torch_count;
uniform vec2 torch_world_pos[MAX_TORCH_COUNT];

const float M_PI = 3.1415926535897932384626433832795;
const float M_2PI = 6.28318;

// Note these should be normalized not 8bit RGB.
const vec3 WARM_YELLOW = vec3( 1.0, 0.92, 0.6 );
const vec3 COOL_WHITE = vec3( 0.8, 0.85, 1.0 );
const vec3 WARM_ORANGE = vec3( 1.0, 0.6, 0.2 );
const vec3 EERIE_BLUE = vec3( 0.1, 0.15, 1.0 );
const vec3 TORCH_COLOR = WARM_ORANGE;
const vec3 NIGHT_COLOR = vec3( 0.1, 0.1, 0.44 );

const float TORCH_ALPHA = 0.25;
const float NIGHT_ALPHA = 0.75;

// The max frequency of the torchlight flicker
const float TORCH_EDGE_FLICKER_FREQ = 2.0;
// The percentage of the radius that is flickering
const float TORCH_EDGE_FLICKER_PERCENT = 0.1;
// Transition band width inside/outside of radius

// night static graininess
const float NIGHT_STATIC_CONTRAST = 17.0;

// // NPC lighting
// uniform int npc_count;
// uniform vec2 npc_positions[8];
// uniform vec2 npc_directions[8];
// uniform float npc_torch_lengths[8];
// uniform float npc_torch_angle; // half-angle in radians e.g. 0.4

// final output pixel sampled_color
out vec4 out_color;

float noise( float a, float b )
{
  uvec2 p = uvec2( uint( a ) * 2654435761u, uint( b ) * 2246822519u );
  p.x ^= p.y;
  p.x *= 2654435761u;
  p.x ^= p.x >> 16u;
  return float( p.x ) / float( 0xFFFFFFFFu );
}

// Create a normalised (0..1) sine wave position from t
float norm_sin( float t ) { return sin( t * M_2PI ) * 0.5 + 0.5; }

// Determine if the current frag is within the player torch radius
float player_torch_pixel( vec2 frag_coord, int torch_idx )
{

  // Create a natural looking flicker on the radius. Multipliers are for
  // Speed/Dominance. Speed multiplers should have Relative Irrationality.
  // Dominanace multiplers should have Relative Sum of 1.0
  float flicker1 = norm_sin( time * TORCH_EDGE_FLICKER_FREQ * 1.0 ) * 0.5;
  float flicker2 = norm_sin( time * TORCH_EDGE_FLICKER_FREQ * 2.3 ) * 0.3;
  float flicker3 = norm_sin( time * TORCH_EDGE_FLICKER_FREQ * 0.7 ) * 0.2;
  float flickered_radius = torch_radius * ( 1.0 + ( flicker1 + flicker2 + flicker3 ) * TORCH_EDGE_FLICKER_PERCENT );

  float dist_to_player = length( frag_coord - torch_world_pos[torch_idx] );

  // return 0..1 between flickered_radius and 0
  // return smoothstep( flickered_radius, 0.0, dist_to_player );
  return clamp( 1.0 - ( dist_to_player / flickered_radius ), 0.0, 1.0 );
}

float npc_light_pixel()
{
  // NPC cones: apex at NPC, spreads outward in facing direction
  float in_npc_light = 0.0;
  // for ( int i = 0; i < npc_count; i++ )
  // {
  //   vec2 to_pixel = frag_coord - npc_positions[i];
  //   float dist = length( to_pixel );
  //   float cos_angle = dot( normalize( to_pixel ), npc_directions[i] );

  //   if ( dist < npc_torch_lengths[i] && cos_angle > cos( npc_torch_angle ) )
  //   {
  //     // Fade toward edges of cone and toward tip
  //     float angular_fade = smoothstep( cos( npc_torch_angle ), cos(
  //     npc_torch_angle * 0.5 ), cos_angle ); float dist_fade = smoothstep(
  //     npc_torch_lengths[i], npc_torch_lengths[i] * 0.2, dist ); in_npc_light
  //     = max( in_npc_light, angular_fade * dist_fade );
  //   }
  // }
  return in_npc_light;
}

vec2 convert_to_world_space( vec2 screen_coord )
{
  // Normalise screen coords to UV coords for the current pixel.
  vec2 uv_coord = screen_coord.xy / resolution;
  // Flip the y-axis for SFML/OpenGL.
  uv_coord.y = 1.0 - uv_coord.y;
  // Convert to world space so shader is anchored to the world, not the screen.
  return view_top_left + uv_coord * view_size;
}

void main()
{

  vec2 frag_coord = convert_to_world_space( gl_FragCoord.xy );
  vec4 sampled_color = texture2D( texture, frag_coord );

  // ── Torch / lighting ────────────────────────────────────────────────────────
  // Accumulate the strongest light contribution from all torches
  float frag_coord_light_amount = 0.0;
  for ( int i = 0; i < torch_count; i++ )
  {
    frag_coord_light_amount = max( frag_coord_light_amount, player_torch_pixel( frag_coord, i ) );
  }
  // ── Static effect ────────────────────────────────────────────────────────────

  sampled_color.rgb = mix( sampled_color.rgb * NIGHT_COLOR, sampled_color.rgb, frag_coord_light_amount );

  // Sample the noise to create moving static.
  // using large prime, cap the offset before it gets large enough to lose precision, while avoiding obvious periodicity
  float static_noise_refresh_rate = mod( floor( time * 120.0 ), 997.0 );

  // Sample the noise used for night static
  float static_noise = noise( frag_coord.x + static_noise_refresh_rate * 7919.0, frag_coord.y + static_noise_refresh_rate * 6271.0 );

  // Apply night static only outside the torch radius, fade out quickly at the edge
  float noise_mask = 1.0 - smoothstep( 0.0, 0.2, frag_coord_light_amount );
  float noise_factor = mix( 1.0, static_noise * NIGHT_STATIC_CONTRAST, noise_mask );
  sampled_color.r *= noise_factor;
  sampled_color.g *= noise_factor;
  sampled_color.b *= noise_factor;

  // Apply the colors for inside/outside the torch radius
  sampled_color.rgb = mix( sampled_color.rgb * NIGHT_COLOR, sampled_color.rgb + TORCH_COLOR, frag_coord_light_amount );

  // ── Output ───────────────────────────────────────────────────────────────────
  sampled_color.a *= mix( NIGHT_ALPHA, TORCH_ALPHA, frag_coord_light_amount );

  out_color = sampled_color;
}