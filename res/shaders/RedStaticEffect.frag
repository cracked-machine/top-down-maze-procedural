#version 330

uniform sampler2D texture;
uniform float time;
uniform float alpha;
uniform vec2 resolution;

out vec4 outColor;

float noise( vec2 st ) { return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) + time * 10.0 ) * 43758.5453123 ); }

void main()
{
  vec2 uv = gl_FragCoord.xy / resolution;
  float n = noise( uv * 100.0 );
  vec3 staticColor = vec3( 0.7 + 0.3 * n, 0.05 + 0.1 * n, 0.05 + 0.1 * n );
  vec4 color = texture2D( texture, uv );
  color.rgb = mix( color.rgb, staticColor, 0.7 );
  color.a *= alpha;
  outColor = color;
}