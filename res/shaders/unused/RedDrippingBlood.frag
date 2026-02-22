#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform float alpha;

// Simple noise for jagged edge
float noise( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 ); }

// Top blood edge, thick and jagged
float topEdge( vec2 uv )
{
  float edgeY = 0.13 + 0.04 * noise( vec2( uv.x * 8.0, 0.0 ) );
  return uv.y < edgeY ? 1.0 : 0.0;
}

// Drip shape, animated to descend
float drip( vec2 uv, float dripX, float t, float length, float thickness )
{
  float edgeY = 0.13 + 0.04 * noise( vec2( dripX * 8.0, 0.0 ) );
  float dripProgress = clamp( t, 0.0, 1.0 );
  float dripY = edgeY + length * dripProgress;
  // Only draw drip below the edge
  if ( uv.y < edgeY ) return 0.0;
  // Narrow vertical drip
  float vertical = smoothstep( edgeY, dripY, uv.y ) * ( abs( uv.x - dripX ) < thickness ? 1.0 : 0.0 );
  // Drip head (circle at the tip)
  float head = smoothstep( thickness * 1.5, thickness, distance( uv, vec2( dripX, dripY ) ) );
  return max( vertical, head );
}

// Small droplet below each drip
float droplet( vec2 uv, float dripX, float dripY, float t )
{
  float dropY = dripY + 0.08 + 0.12 * t;
  if ( uv.y < dripY ) return 0.0;
  float size = 0.012 + 0.008 * noise( vec2( dripX, dripY ) );
  return smoothstep( size * 1.5, size, distance( uv, vec2( dripX, dropY ) ) );
}

void main()
{
  vec2 uv = gl_FragCoord.xy;
  vec4 color = texture2D( texture, uv );

  // Draw the jagged top edge
  float blood = topEdge( uv );

  float t = mod( time * 0.18, 1.0 );

  // Drips
  for ( int i = 0; i < 10; ++i )
  {
    float dripX = 0.08 + 0.08 * float( i ) + 0.02 * noise( vec2( i, 0.0 ) );
    float length = 0.25 + 0.25 * noise( vec2( i, 1.0 ) );
    float thickness = 0.012 + 0.008 * noise( vec2( i, 2.0 ) ); // Make drips thinner
    float dripT = clamp( t + float( i ) * 0.07, 0.0, 1.0 );

    float edgeY = 0.13 + 0.04 * noise( vec2( dripX * 8.0, 0.0 ) );
    float dripY = edgeY + length * dripT;

    blood += drip( uv, dripX, dripT, length, thickness );

    // Add a droplet below each drip
    if ( dripT > 0.8 ) { blood += droplet( uv, dripX, dripY, ( dripT - 0.8 ) * 5.0 ); }
  }

  blood = clamp( blood, 0.0, 1.0 );

  vec3 bloodColor = vec3( 0.7, 0.05, 0.05 );

  color.rgb = mix( color.rgb, bloodColor, blood * 0.95 );
  color.a *= alpha * ( 1.0 - 0.3 * blood );

  gl_FragColor = color;
}