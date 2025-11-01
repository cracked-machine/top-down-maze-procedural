#version 120

uniform sampler2D texture;
uniform float time;
uniform float alpha;

// Improved noise function for smoother clouds
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion for cloud-like appearance
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

void main() {
    vec2 texCoord = gl_TexCoord[0].xy;
    
    // Sample the base texture
    vec4 color = texture2D(texture, texCoord);
    
    // Create horizontal scrolling effect (right to left)
    vec2 mistCoord = texCoord * 3.0;
    mistCoord.x -= time * 0.15; // Negative = right to left movement
    
    // Layer multiple noise octaves for depth
    float mist1 = fbm(mistCoord);
    float mist2 = fbm(mistCoord * 1.5 + vec2(100.0, 50.0));
    
    // Combine layers
    float mistValue = (mist1 * 0.6 + mist2 * 0.4);
    
    // Create wispy cloud effect
    mistValue = smoothstep(0.3, 0.7, mistValue);
    
    // Apply mist color (light gray/white)
    vec3 mistColor = vec3(0.9, 0.95, 1.0);
    
    // Blend mist with base texture
    color.rgb = mix(color.rgb, mistColor, mistValue * 0.5);
    
    // Apply alpha uniform
    color.a *= alpha;
    
    gl_FragColor = color;
}