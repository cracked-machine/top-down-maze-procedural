uniform sampler2D texture;  // Base water texture
uniform float time;         // For animation
uniform vec2 resolution;    // Screen dimensions
uniform float waterLevel;   // Current water height

void main() {
    // Calculate position relative to screen
    vec2 pos = gl_FragCoord.xy / resolution;
    
    // Create wave effect using time
    float wave1 = sin(pos.x * 10.0 + time * 0.5) * 0.03;
    float wave2 = cos(pos.x * 15.0 - time * 0.3) * 0.02;
    float waves = wave1 + wave2;
    
    // Sample texture with wave distortion
    vec2 distortedPos = vec2(pos.x, pos.y + waves);
    vec4 color = texture2D(texture, distortedPos);
    
    // Add highlight effect on top of waves
    float highlight = max(0.0, sin(pos.x * 20.0 + time * 2.0) * 0.1);
    color.rgb += vec3(highlight, highlight, highlight * 1.5);
    
    // Set transparency (50% as specified in TODO)
    color.a = 0.5;
    
    gl_FragColor = color;
}