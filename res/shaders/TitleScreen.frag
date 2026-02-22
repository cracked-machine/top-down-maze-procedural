#version 330

uniform sampler2D texture;
uniform vec2 mouse_cursor;
uniform float pixel_threshold;
uniform float time;
uniform vec2 resolution;

out vec4 outColor;

// Mandelbrot-like fractal function
float mandelbrot( vec2 c, int max_iter )
{
  vec2 z = vec2( 0.0 );
  for ( int i = 0; i < max_iter; i++ )
  {
    if ( dot( z, z ) > 4.0 ) return float( i ) / float( max_iter );
    z = vec2( z.x * z.x - z.y * z.y, 2.0 * z.x * z.y ) + c;
  }
  return 0.0;
}

// Julia set function
float julia( vec2 z, vec2 c, int max_iter )
{
  for ( int i = 0; i < max_iter; i++ )
  {
    if ( dot( z, z ) > 4.0 ) return float( i ) / float( max_iter );
    z = vec2( z.x * z.x - z.y * z.y, 2.0 * z.x * z.y ) + c;
  }
  return 0.0;
}

// Sierpinski triangle approximation
float sierpinski( vec2 p )
{
  float d = 1.0;
  for ( int i = 0; i < 8; i++ )
  {
    p = abs( p ) - 0.5;
    p *= 1.5;
    d *= 1.5;
  }
  return length( p ) / d;
}

void main()
{
  //
  vec2 coord = gl_FragCoord.xy / resolution;
  vec2 uv = ( coord - 0.5 ) * 2.0; // Convert to -1 to 1 range

  // Mouse influence on zoom and offset
  float mouse_zoom = 1.0 + pixel_threshold * 0.1;
  vec2 mouse_offset = ( mouse_cursor - 0.5 ) * 0.5;

  // Time-based animation
  float t = time * 0.3;

  // Calculate circular distortion fields
  float dist = length( uv );
  float angle = atan( uv.y, uv.x );

  // Create multiple circular wave distortions
  float wave1 = sin( dist * 15.0 - t * 3.0 ) * 0.1;
  float wave2 = sin( dist * 25.0 + t * 2.0 ) * 0.05;
  float wave3 = cos( dist * 35.0 - t * 4.0 ) * 0.03;

  // Radial distortion that varies with angle and time
  float radial_distort = sin( angle * 6.0 + t * 2.0 ) * 0.08;

  // Mouse-influenced circular distortion
  vec2 mouse_pos = ( mouse_cursor - 0.5 ) * 2.0;
  float mouse_dist = distance( uv, mouse_pos );
  float mouse_wave = sin( mouse_dist * 20.0 - t * 5.0 ) * 0.05 * pixel_threshold / 30.0;

  // Combine all circular distortions
  vec2 distortion = vec2( wave1 + wave2 + radial_distort + mouse_wave, wave1 + wave3 - radial_distort + mouse_wave );

  // Apply polar coordinate distortion
  float polar_distort = sin( dist * 10.0 + angle * 3.0 + t * 2.5 ) * 0.06;
  distortion += vec2( cos( angle ), sin( angle ) ) * polar_distort;

  // Animated Julia set parameters with circular influence
  vec2 julia_c = vec2( cos( t * 0.7 + dist * 2.0 ) * 0.7, sin( t * 0.5 + dist * 3.0 ) * 0.7 );

  // Scale and translate coordinates with distortion applied
  vec2 fractal_coord = ( uv + mouse_offset + distortion ) * mouse_zoom;

  // Pattern 1: Mandelbrot with circular distortion
  vec2 mandel_coord = fractal_coord * ( 2.0 + sin( t + dist * 5.0 ) * 0.5 ) + vec2( cos( t * 0.3 ), sin( t * 0.4 ) ) * 0.3;
  // Add more distortion to mandelbrot coordinates
  mandel_coord += vec2( sin( dist * 12.0 + t * 3.0 ), cos( dist * 8.0 - t * 2.0 ) ) * 0.1;
  float mandel = mandelbrot( mandel_coord, 32 );

  // Pattern 2: Julia set with radial distortion
  vec2 julia_coord = fractal_coord * ( 1.5 + cos( t * 0.8 + dist * 4.0 ) * 0.3 );
  // Apply spiral distortion to julia coordinates
  float spiral_distort = sin( angle * 4.0 + dist * 15.0 + t * 3.0 ) * 0.08;
  julia_coord += vec2( cos( angle + spiral_distort ), sin( angle + spiral_distort ) ) * spiral_distort;
  float jul = julia( julia_coord, julia_c, 24 );

  // Pattern 3: Sierpinski with wave distortion
  vec2 sierp_coord = fractal_coord * ( 3.0 + sin( t * 1.2 + dist * 6.0 ) * 0.5 );
  sierp_coord = mod( sierp_coord + vec2( cos( t * 0.6 ), sin( t * 0.9 ) ) * 0.2, 2.0 ) - 1.0;
  // Add circular wave distortion to sierpinski
  sierp_coord += vec2( sin( dist * 20.0 + t * 4.0 ), cos( dist * 18.0 - t * 3.5 ) ) * 0.05;
  float sierp = sierpinski( sierp_coord );
  sierp = smoothstep( 0.02, 0.08, sierp );

  // Pattern 4: Fractal noise with circular modulation
  vec2 noise_coord = fractal_coord * 8.0 + vec2( cos( t * 0.5 ), sin( t * 0.7 ) ) * 2.0;
  // Modulate noise frequency with circular data
  noise_coord *= ( 1.0 + sin( dist * 8.0 + t * 2.0 ) * 0.3 );
  float noise = 0.0;
  float amplitude = 1.0;
  for ( int i = 0; i < 4; i++ )
  {
    // Apply circular distortion to each noise octave
    vec2 distorted_coord = noise_coord + vec2( sin( dist * 10.0 + t ), cos( dist * 12.0 - t ) ) * 0.1;
    noise += sin( distorted_coord.x * amplitude ) * cos( distorted_coord.y * amplitude ) / amplitude;
    noise_coord *= 2.0;
    amplitude *= 2.0;
  }
  noise = abs( noise ) * 0.5;

  // Combine fractals with circular-influenced blending
  float blend1 = sin( t * 1.5 + dist * 3.0 ) * 0.5 + 0.5;
  float blend2 = cos( t * 1.2 + dist * 4.0 ) * 0.5 + 0.5;
  float blend3 = sin( t * 0.8 + dist * 2.0 + 1.57 ) * 0.5 + 0.5;

  // float fractal_mix = mix( mandel, jul, blend1 );
  // float fractal_mix = mix( sierp, blend2 * 0.7 );
  float fractal_mix = mix( sierp, noise, blend3 * 0.3 );

  // Create interesting patterns from fractal data with circular modulation
  float pattern = sierp;
  pattern += sin( fractal_mix * 20.0 + t * 3.0 + dist * 5.0 ) * 0.1;
  pattern = smoothstep( 0.1, 0.9, pattern );

  // // Grid overlay with circular distortion
  // vec2 grid_coord = ( uv + distortion * 2.0 ) * 16.0 + t * 0.5;
  // float grid = abs( sin( grid_coord.x ) ) * abs( sin( grid_coord.y ) );
  // grid = smoothstep( 0.8, 0.95, grid );

  // Combine pattern with grid
  // float final_pattern = max( pattern, grid * 0.5 );
  float final_pattern = pattern;

  // Create monochrome output with circular brightness modulation
  float brightness = 0.3 + final_pattern * 0.7;
  // Modulate brightness with circular waves instead of displaying them
  brightness *= ( 0.9 + sin( t * 2.0 + dist * 8.0 ) * 0.1 );

  // High contrast edges with circular influence
  brightness = smoothstep( 0.2 + sin( dist * 15.0 + t ) * 0.05, 0.8, brightness );

  vec3 base_color = vec3( 0.7, 0.7, 1.0 ); // Blue tint
  vec3 final_color = base_color * brightness;

  outColor = vec4( final_color, 1.0 );
}