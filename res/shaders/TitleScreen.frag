#version 120

uniform sampler2D texture;
uniform vec2 mouse_cursor;
uniform float pixel_threshold;
uniform float time;

void main()
{
  vec2 coord = gl_TexCoord[0].xy;
  vec2 uv = coord;

  // Pattern 1: Animated circular waves
  vec2 center = mouse_cursor;
  float dist = distance( uv, center );
  float wave1 = sin( dist * 15.0 - time * 3.0 ) * 0.5 + 0.5;
  float wave2 = sin( dist * 25.0 + time * 2.0 ) * 0.3 + 0.7;

  // Pattern 2: Moving grid pattern
  vec2 grid_coord = uv * 8.0 + vec2( sin( time * 0.5 ), cos( time * 0.7 ) ) * 0.3;
  float grid = abs( sin( grid_coord.x * 3.14159 ) ) * abs( sin( grid_coord.y * 3.14159 ) );
  grid = smoothstep( 0.3, 0.7, grid );

  // Pattern 3: Rotating spiral pattern
  vec2 pos = uv - center;
  float angle = atan( pos.y, pos.x ) + time * 0.8;
  float radius = length( pos );
  float spiral = sin( angle * 4.0 + radius * 20.0 ) * 0.5 + 0.5;

  // Pattern 4: Flowing noise pattern
  vec2 noise_coord = uv * 12.0 + vec2( cos( time * 0.6 ), sin( time * 0.9 ) ) * 2.0;
  float noise = fract( sin( dot( noise_coord, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 );
  noise = smoothstep( 0.2, 0.8, noise );

  // Pattern 5: Pulsing rings
  float ring_dist = mod( dist * 20.0 - time * 4.0, 2.0 );
  float rings = smoothstep( 0.0, 0.3, ring_dist ) * smoothstep( 2.0, 1.7, ring_dist );

  // Mouse influence factor
  float mouse_influence = clamp( pixel_threshold / 50.0, 0.0, 1.0 );

  // Time-based color shifts
  float color_shift = sin( time * 1.2 ) * 0.5 + 0.5;

  // Dynamic colors with time-based hue shifting
  vec3 color1 = vec3( 0.3 + wave1 * 0.4 + sin( time * 1.5 ) * 0.2, 0.1 + wave2 * 0.6, 0.8 + wave1 * 0.2 );

  vec3 color2 = vec3( 0.8 + grid * 0.2, 0.4 + grid * 0.3 + cos( time * 1.8 ) * 0.2, 0.1 + grid * 0.4 );

  vec3 color3 = vec3( 0.9 + sin( time * 2.1 ) * 0.1, 0.2 + spiral * 0.5, 0.7 + spiral * 0.3 + cos( time * 1.3 ) * 0.2 );

  vec3 color4 = vec3( 0.2 + noise * 0.6, 0.8 + sin( time * 1.7 ) * 0.2, 0.3 + noise * 0.4 );

  // Animate the blending with time
  float blend_factor1 = sin( time * 1.5 + mouse_influence * 2.0 ) * 0.5 + 0.5;
  float blend_factor2 = cos( time * 1.2 + mouse_influence * 3.0 ) * 0.5 + 0.5;

  // Mix patterns based on time and mouse position
  vec3 blend1 = mix( color1, color2, blend_factor1 );
  vec3 blend2 = mix( color3, color4, blend_factor2 );

  // Final color mixing with ring overlay
  vec3 final_color = mix( blend1, blend2, color_shift );
  final_color = mix( final_color, vec3( 1.0, 0.8, 0.4 ), rings * 0.3 );

  // Time-based brightness pulsing influenced by mouse
  float pulse = sin( time * 2.5 ) * 0.15 + 0.85;
  float mouse_brightness = 0.6 + mouse_influence * 0.6;

  final_color *= pulse * mouse_brightness;

  // Add subtle color temperature shift over time
  final_color.r *= 1.0 + sin( time * 0.8 ) * 0.1;
  final_color.b *= 1.0 + cos( time * 0.6 ) * 0.1;

  gl_FragColor = vec4( final_color, 1.0 );
}