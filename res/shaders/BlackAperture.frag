#version 120

uniform sampler2D texture;
uniform vec2 playerPos;
uniform float visibilityRadius;

void main()
{
  vec2 pixelPos = gl_FragCoord.xy;
  float distance = length( pixelPos - playerPos );

  if ( distance <= visibilityRadius ) { gl_FragColor = texture2D( texture, gl_TexCoord[0].xy ); }
  else { gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 ); }
}