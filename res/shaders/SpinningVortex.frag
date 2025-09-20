#version 120

uniform sampler2D texture;
uniform float time;
uniform vec2 screenSize;
uniform vec2 centerPosition;
uniform float vortexStrength;
uniform float vortexRadius;
uniform float spinSpeed;

void main()
{
  vec2 texCoord = gl_TexCoord[0].xy;

  // Work in texture coordinate space (0.0 to 1.0)
  vec2 center = vec2( 0.5, 0.5 ); // Always center of texture
  vec2 offset = texCoord - center;
  float distance = length( offset );
  float angle = atan( offset.y, offset.x );

  // Convert vortexRadius from world units to texture coordinate units
  float normalizedRadius = vortexRadius / max( screenSize.x, screenSize.y );

  // Create vortex effect - stronger near center, weaker at edges
  float vortexFactor = 1.0 - smoothstep( 0.0, normalizedRadius, distance );

  // Apply spinning rotation based on distance from center
  float angleOffset = vortexFactor * vortexStrength * time * spinSpeed;

  // Calculate new rotated position in texture coordinates
  float newAngle = angle + angleOffset;
  vec2 rotatedOffset = vec2( cos( newAngle ), sin( newAngle ) ) * distance;
  vec2 newTexCoord = center + rotatedOffset;

  // Sample texture with vortex distortion
  vec4 color;
  if ( newTexCoord.x >= 0.0 && newTexCoord.x <= 1.0 && newTexCoord.y >= 0.0 &&
       newTexCoord.y <= 1.0 )
  {
    color = texture2D( texture, newTexCoord );
  }
  else
  {
    color = texture2D( texture, texCoord ); // Fallback to original position
  }

  // Add glowing center effect
  float centerGlow = 1.0 - smoothstep( 0.0, normalizedRadius * 0.3, distance );
  float pulse = sin( time * 4.0 ) * 0.5 + 0.5;                   // Pulsing effect
  color.rgb += centerGlow * pulse * 0.3 * vec3( 1.0, 0.4, 0.8 ); // Pink/magenta glow

  gl_FragColor = color;
}