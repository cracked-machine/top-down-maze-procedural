#version 330

// The already-rendered game frame, captured into the shader's render texture by RenderGameSystem
// before this pass runs, and re-sampled here with a displaced UV to warp the whole screen.
uniform sampler2D texture;
// elapsed time
uniform float time;
// screen dimensions
uniform vec2 resolution;
// normalised player fear (0..1), drives how large/frequent/intense the hallucination patches are
uniform float fear;

out vec4 out_color;

// Sibling of FearDistortion.frag: that version warps the *entire* frame, which reads as a general
// "queasy" wobble rather than a hallucination. This variant instead warps one or two small,
// randomly-placed patches at a time - like a heat-haze shimmer flickering somewhere in the player's
// vision - while the rest of the frame stays sharp. Both take the same uniforms, so switching between
// them is just the .frag path passed to Factory::Shader::add_fear_distortion.

// How many haze patches are active at fear=0: zero, so there's no hallucination at all until fear
// rises (see MAX_EXTRA_HAZE_REGIONS).
const int BASE_HAZE_REGIONS = 0;
// Additional patches become active as fear rises, capping out at MAX_EXTRA_HAZE_REGIONS at fear=100%.
// Kept low and scaled linearly with fear (same shape as visible_fraction below) on purpose: count and
// duration both grow proportionally to fear together, so their ratio - roughly how often some patch
// is appearing/disappearing anywhere on screen - stays even across the whole range instead of one
// racing ahead of the other and reading as the whole effect "speeding up" with fear.
const int MAX_EXTRA_HAZE_REGIONS = 4;
const int MAX_HAZE_REGIONS = BASE_HAZE_REGIONS + MAX_EXTRA_HAZE_REGIONS;

// UV displacement (as a fraction of screen size) inside a patch. Bigger than FearDistortion.frag's
// MAX_AMPLITUDE since only a small area carries it, not the whole screen. Fixed regardless of fear:
// amplitude is a multiplier on the ripple's sine wave, so scaling it with fear would scale the wave's
// on-screen velocity too (d(offset)/dt is proportional to amplitude) - i.e. it would visibly speed
// the ripple up even though WAVE_FREQUENCY/WAVE_SPEED never changed. Only patch count and duration
// carry the fear signal now.
const float AMPLITUDE = 0.018;
// Ripple frequency across a patch. Fixed regardless of fear - only how many patches appear, how
// strongly they displace, and how long each one lingers scale with fear, not the ripple itself.
const float WAVE_FREQUENCY = 8.0;
// Fixed animation rate. This must NOT depend on `fear`, for the same reason as FearDistortion.frag's
// WAVE_SPEED: fear changes in discrete steps mid-game, and `speed` scales the ever-growing `time`
// rather than an independently accumulated phase, so any fear-dependent multiplier would retroactively
// snap the whole ripple to a different point in its cycle. Fear only scales amplitude/size/timing below.
const float WAVE_SPEED = 3.4;

// Patch radius (as a fraction of the shorter screen dimension). Fixed regardless of fear - patches
// stay small, glimpsed "in the corner of your eye" rather than growing into obvious set-pieces.
const float REGION_RADIUS = 0.10;
// Width of the soft feathered edge around a patch, as a fraction of its own radius
const float REGION_FEATHER = 0.6;
// Fixed length of a haze slot's recurring window. This must NOT depend on `fear`, for the same
// reason as WAVE_SPEED above: (wrapped_time + seed) / HAZE_CYCLE_LENGTH is used below to derive
// both which window we're in and how far through it we are, and wrapped_time keeps growing. If the
// divisor itself changed with fear, every fear change would retroactively reinterpret the *entire*
// elapsed wrapped_time under a new divisor, not just time going forward - local_t would swing wildly
// for a frame or two (worse the larger wrapped_time already is), reading as duration/frequency
// briefly spiking before settling. Keeping it constant makes local_t a clean, continuous function of
// time no matter how fear moves. Fear instead controls VISIBLE_FRACTION below: how much of each
// fixed-length window a patch actually spends visible, not the window's own length.
const float HAZE_CYCLE_LENGTH = 4.0;
// Fraction of each HAZE_CYCLE_LENGTH window a patch spends visible: a brief, barely-there flicker at
// zero fear, growing to almost the whole window at full fear, so patches linger and become harder to
// dismiss the more afraid the player is. Scaled linearly with fear, same shape as active_regions
// above, for the same reason (see MAX_EXTRA_HAZE_REGIONS).
const float MIN_VISIBLE_FRACTION = 0.08;
const float MAX_VISIBLE_FRACTION = 0.85;
// Fraction of HAZE_CYCLE_LENGTH spent easing a patch's strength in, and again easing it out, at the
// edges of its visible window
const float REGION_FADE_FRACTION = 0.12;
// How far a patch's center wanders from its spawn point over its lifetime, as a fraction of screen size
const float REGION_DRIFT = 0.035;

// Strength of the darkened, reddish vignette at full fear
const float MAX_VIGNETTE = 0.55;
// Curve steepness for the fear -> vignette ramp (see vignette_curve in main()): higher means more
// of the range is reached early, i.e. a faster rise that then tapers off (logarithmic, not linear)
const float VIGNETTE_LOG_K = 9.0;
const float TWO_PI = 6.28318530718;
// `time` grows unbounded for as long as this shader instance stays alive (see BaseShaderSprite's
// m_clock) and feeds directly into `speed` below - scaled again by up to 1.3x for the y term. Left
// unbounded, float32 precision degrades as `time` grows: the per-frame phase step gets coarser,
// which over a long enough session makes the ripple look increasingly jerky/faster, independent of
// `fear`. Wrapping it into an hour-long window keeps that precision effectively perfect for any
// realistic session; the single-frame seam this introduces once an hour is far smaller than the
// churn already happening from patches relocating every few seconds.
const float TIME_WRAP_PERIOD = 3600.0;

// Cheap 1D/2D hash returning pseudo-random values in [0, 1). GLSL 330 has no built-in noise/random.
float hash11( float n ) { return fract( sin( n ) * 43758.5453123 ); }
vec2 hash21( float n ) { return vec2( hash11( n ), hash11( n + 17.17 ) ); }

void main()
{
  // No y-flip needed here: `texture` was itself rendered (via gl_FragCoord-based rasterization)
  // into the same kind of target this pass is now drawing into, so both use the same orientation.
  vec2 uv = gl_FragCoord.xy / resolution;
  float aspect = resolution.x / resolution.y;
  float wrapped_time = mod( time, TIME_WRAP_PERIOD );

  // Linear in fear, matching active_regions below exactly in shape, so the two grow in step
  // rather than one racing ahead of the other (see MAX_EXTRA_HAZE_REGIONS for why that matters).
  float visible_fraction = mix( MIN_VISIBLE_FRACTION, MAX_VISIBLE_FRACTION, fear );
  float half_visible = visible_fraction * 0.5;

  // How many patches are active at the current fear level, as a continuous (not stepped) value: each
  // patch beyond BASE_HAZE_REGIONS ramps in/out smoothly across its own fraction of the fear range via
  // `region_existence` below, rather than snapping into existence the instant a threshold is crossed
  // - the same reason FearDistortionShader::update smooths `fear` itself before it reaches this uniform.
  float active_regions = float( BASE_HAZE_REGIONS ) + fear * float( MAX_EXTRA_HAZE_REGIONS );

  vec2 total_offset = vec2( 0.0 );

  for ( int i = 0; i < MAX_HAZE_REGIONS; ++i )
  {
    float region_existence = clamp( active_regions - float( i ), 0.0, 1.0 );
    if ( region_existence <= 0.0 ) continue;

    float seed = float( i ) * 91.7;

    // Which fixed-length window of this region slot we're currently in, and how far through it we are.
    float region_index = floor( ( wrapped_time + seed ) / HAZE_CYCLE_LENGTH );
    float local_t = fract( ( wrapped_time + seed ) / HAZE_CYCLE_LENGTH );

    // Stable random spawn point + drift phase for this window: only changes when region_index does.
    vec2 spawn_point = hash21( region_index + seed );
    float drift_phase = hash11( region_index * 3.1 + seed ) * TWO_PI;

    // Small wander around the spawn point over the patch's lifetime, so it doesn't sit perfectly still.
    vec2 drift = vec2( sin( local_t * TWO_PI + drift_phase ), cos( local_t * TWO_PI * 1.3 + drift_phase ) ) * REGION_DRIFT;
    vec2 region_center = spawn_point + drift;

    // Ease the patch's strength in, hold, then ease it out across a visible window centered in the
    // middle of this fixed-length cycle (width set by visible_fraction, fear-scaled - see above); the
    // patch sits invisible for the rest of the cycle. Adjacent cycles both start/end at envelope 0,
    // so patches relocate with no visible pop even though `region_center` itself jumps between them.
    float fade_in = smoothstep( 0.5 - half_visible - REGION_FADE_FRACTION, 0.5 - half_visible, local_t );
    float fade_out = 1.0 - smoothstep( 0.5 + half_visible, 0.5 + half_visible + REGION_FADE_FRACTION, local_t );
    float envelope = fade_in * fade_out;

    // Distance in aspect-corrected space so patches read as circular, not stretched ovals.
    vec2 delta = uv - region_center;
    delta.x *= aspect;
    float dist = length( delta );
    float radial_falloff = 1.0 - smoothstep( REGION_RADIUS * ( 1.0 - REGION_FEATHER ), REGION_RADIUS, dist );

    float strength = radial_falloff * envelope * AMPLITUDE * region_existence;

    // Wrap each trig call's full argument (after its own scaling) into [0, 2*PI) independently, for
    // the same reason as FearDistortion.frag: sin(x) == sin(mod(x, 2*PI)) exactly, so this introduces
    // no seam, but only because the wrap is applied after the term-specific multiplier.
    float speed = ( wrapped_time + seed ) * WAVE_SPEED;
    total_offset.x += sin( mod( uv.y * WAVE_FREQUENCY + speed, TWO_PI ) ) * strength;
    total_offset.y += cos( mod( uv.x * WAVE_FREQUENCY * 0.85 + speed * 1.3, TWO_PI ) ) * strength;
  }

  vec2 sample_uv = uv + total_offset;

  vec4 color;
  if ( sample_uv.x >= 0.0 && sample_uv.x <= 1.0 && sample_uv.y >= 0.0 && sample_uv.y <= 1.0 ) { color = texture2D( texture, sample_uv ); }
  else { color = texture2D( texture, uv ); }

  // subtle reddish vignette that intensifies with fear: logarithmic ease-out from 0 at fear=0 to 1
  // at fear=1, rising quickly at first then tapering off as fear approaches its max
  float dist_from_center = length( uv - 0.5 );
  float vignette_curve = log( 1.0 + VIGNETTE_LOG_K * fear ) / log( 1.0 + VIGNETTE_LOG_K );
  float vignette = smoothstep( 0.2, 0.9, dist_from_center ) * vignette_curve * MAX_VIGNETTE;
  color.rgb = mix( color.rgb, vec3( 0.2, 0.0, 0.0 ), vignette );

  out_color = color;
}
