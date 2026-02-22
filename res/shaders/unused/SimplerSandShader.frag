#version 330 compatibility

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform float sandIntensity;
uniform float timeScale;

// Simple hash function
float hash( vec2 p ) { return fract( sin( dot( p, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 ); }

void main()
{
  vec2 texCoord = gl_FragCoord.xy;
  vec2 worldPos = texCoord * screenSize;

  // Sample original texture
  vec4 originalColor = texture2D( texture, texCoord );

  // Simple grain pattern - much less computation
  float grain = hash( floor( worldPos + time * timeScale ) );

  // Simple color variation
  float brightness = 0.9 + grain * 0.2;
  vec3 sandTint = vec3( 1.05, 1.02, 0.98 );

  // Apply simple sand effect
  vec3 finalColor = originalColor.rgb * brightness * sandTint;

  // Blend with original based on intensity
  finalColor = mix( originalColor.rgb, finalColor, sandIntensity );

  gl_FragColor = vec4( finalColor, originalColor.a );
}