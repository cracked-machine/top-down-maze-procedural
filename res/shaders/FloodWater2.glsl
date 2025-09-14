uniform sampler2D texture;    // Base water texture
uniform float time;           // For animation
uniform vec2 resolution;      // Screen dimensions
uniform float waterLevel;     // Current water height

void main() {
    // Calculate position relative to screen
    vec2 pos = gl_FragCoord.xy / resolution;
    
    // Create primary wave patterns with different frequencies and phases
    float wave1 = sin(pos.x * 10.0 + time * 0.5) * 0.03;
    float wave2 = cos(pos.x * 15.0 - time * 0.3) * 0.02;
    
    // Add diagonal wave patterns for more natural water movement
    float wave3 = sin((pos.x + pos.y) * 8.0 + time * 0.7) * 0.015;
    float wave4 = cos((pos.x - pos.y) * 12.0 - time * 0.4) * 0.01;
    
    // Combine waves with distance-based amplitude falloff
    // (waves get smaller near the edges)
    float distFromCenter = abs(pos.x - 0.5) * 2.0;
    float edgeFalloff = 1.0 - (distFromCenter * distFromCenter);
    
    // Sum all wave components with edge falloff
    float waves = (wave1 + wave2 + wave3 + wave4) * edgeFalloff;
    
    // Create small ripples for surface detail
    float ripples = sin(pos.x * 40.0 + pos.y * 30.0 + time * 3.0) * 0.005;
    waves += ripples;
    
    // Sample texture with wave distortion
    vec2 distortedPos = vec2(pos.x + waves * 0.2, pos.y + waves);
    vec4 color = texture2D(texture, distortedPos);
    
    // Add highlight effect on wave crests
    float highlight = max(0.0, sin(pos.x * 20.0 + time * 2.0 + waves * 30.0) * 0.15);
    color.rgb += vec3(highlight, highlight, highlight * 1.3) * edgeFalloff;
    
    // Add dark shadow areas in wave troughs
    float shadow = max(0.0, -sin(pos.x * 20.0 + time * 2.0 + waves * 30.0) * 0.05);
    color.rgb -= vec3(shadow, shadow, shadow);
    
    // Set transparency (50% as specified in original TODO)
    color.a = 0.5;
    
    gl_FragColor = color;
}