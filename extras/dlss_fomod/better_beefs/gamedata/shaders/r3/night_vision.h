#include "common.h"
uniform float4 shader_param_8;
uniform float4 shader_param_7;

//      BEEF'S SHADER BASED NIGHT VISION EFFECT       //
//                                                    //
// BEEF          - Original shader design             //
// LVutner       - Core architecture & mentorship     //
// Sky4Ace       - Blur function basis                //
// Meltac        - Advice & inspiration               //
// Mattias       - CRT effect (ShaderToy)             //
// xwize         - Luma sharpen (ShaderToy)           //
// indiesunpraiser - Panoramic GPNVG-18, tonemap,     //
//                   bloom, halo, distance falloff,   //
//                   per-gen pipeline params          //
//                                                    //
// Free to distribute and adapt with credit.          //

// USER SETTINGS (PER GENERATION)
#define sky_brightness_factor float(1.0f)

// GEN 1 — Vintage phosphor (noisy, CRT, full NVG character)
#define gen_1_crt_effect_factor      float(0.05)
#define gen_1_nvg_noise_factor       float(0.075)
#define gen_1_scintillation_constant float(0.999f)
#define gen_1_luma_sharpen_factor    float(0.0f)
#define gen_1_bloom_strength         float(1.0f)    // 0=none, 1=full
#define gen_1_contrast               float(1.3f)    // 1.0=off, 1.5=punchy, 2.0=harsh
#define gen_1_mid_lift               float(0.15f)   // 0=off, 0.2=strong
#define gen_1_dist_att_strength      float(1.0f)    // 0=crystal clear, 1=full falloff
#define gen_1_halo_intensity         float(0.6f)    // 0=off, 1=bright halos around lights
#define gen_1_image_blur             float(0.0f)    // 0=sharp, 1=full half-res blur

// GEN 2 — Modern military (cleaner, slight CRT)
#define gen_2_crt_effect_factor      float(0.07)
#define gen_2_nvg_noise_factor       float(0.05)
#define gen_2_scintillation_constant float(0.9991f)
#define gen_2_luma_sharpen_factor    float(0.25f)
#define gen_2_bloom_strength         float(1.0f)
#define gen_2_contrast               float(1.3f)
#define gen_2_mid_lift               float(0.15f)
#define gen_2_dist_att_strength      float(1.0f)
#define gen_2_halo_intensity         float(0.6f)
#define gen_2_image_blur             float(0.0f)

// GEN 3 — Digital panoramic (crystal clear, minimal processing)
#define gen_3_crt_effect_factor      float(0.0)
#define gen_3_nvg_noise_factor       float(0.0)
#define gen_3_scintillation_constant float(1.0f)
#define gen_3_luma_sharpen_factor    float(0.5f)
#define gen_3_bloom_strength         float(0.4f)
#define gen_3_contrast               float(1.0f)
#define gen_3_mid_lift               float(0.0f)
#define gen_3_dist_att_strength      float(0.0f)
#define gen_3_halo_intensity         float(0.0f)
#define gen_3_image_blur             float(0.0f)

// FALLBACK COLORS (used when MCM color is not set)
#define gen_1_saturation_color float3(0.0, 0.75, 0.0)
#define gen_2_saturation_color float3(0.35, 0.65, 0.5)
#define gen_3_saturation_color float3(0.55, 0.75, 0.75)

// INTERNAL CONSTANTS
#define tube_radius float(frac(shader_param_7.y))

#define single_tube_centered    float2(0.5f,  -0.5f + (floor(shader_param_7.x) / 100))
#define single_tube_offset_left float2(0.25f, -0.5f + (floor(shader_param_7.x) / 100))
#define single_tube_offset_right float2(0.75f,-0.5f + (floor(shader_param_7.x) / 100))
#define dual_tube_offset_1      float2(0.25f, -0.5f + (floor(shader_param_7.x) / 100))
#define dual_tube_offset_2      float2(0.75f, -0.5f + (floor(shader_param_7.x) / 100))
#define quad_tube_offset_1      float2(0.05f, -0.5f + (floor(shader_param_7.x) / 100))
#define quad_tube_offset_2      float2(0.3f,  -0.5f + (floor(shader_param_7.x) / 100))
#define quad_tube_offset_3      float2(0.7f,  -0.5f + (floor(shader_param_7.x) / 100))
#define quad_tube_offset_4      float2(0.95f, -0.5f + (floor(shader_param_7.x) / 100))

#define luma_conversion_coeff float3(0.299, 0.587, 0.114)
#define farthest_depth float(25.0f)

// SHADER_PARAM PACKING REFERENCE (all components must stay in [-100, 100])
// shader_param_8:
//   .x  floor = generation (1-3)        frac*10 = num_tubes (1,1.1,1.2,2,4,4.1)
//   .y  floor/10 = gain (0.1-3.0)       frac = washout_thresh (digit 0-9 → 0.05-0.50)
//   .z  floor/100 = vignette (0.0-1.0)  frac = glitch_power (0.0-0.9)
//   .w  floor/10 = gain_offset (0.5-3)  frac*10 = mode (0-9)
//
// shader_param_7:
//   .x  floor = flip_down (1-99)        frac*10/9 = bloom (0-9 -> 0.0-1.0)
//   .y  floor = color_hi (0-92)         frac*10 = tube_radius (0-9)
//   .z  floor = color_lo (0-99)         frac = near_blur_range (1-9 meters)
//   .w  floor = halo*10+imgblur (0-99)  frac*100/99 = near_blur (00-99 -> 0.0-1.0)
//        halo: floor(w_floor/10)/9       imgblur: (w_floor%10)/9
//   Color: P = color_hi*100 + color_lo, r=floor(P/441)/20, g=floor(P%441/21)/20, b=(P%21)/20

// GPNVG-18 PANORAMIC GEOMETRY
#define pano_cy (-0.5f + (floor(shader_param_7.x) / 100))
#define pano_rx 0.4
#define pano_ry 0.5
#define pano_radii float2(pano_rx, pano_ry)

// Tube center X-coordinates (adjust for panoramic FOV)
#define tube_far_left_x    -0.25
#define tube_center_left_x  0.30
#define tube_center_right_x 0.70
#define tube_far_right_x    1.25

// Auto-derived geometry
#define pano_seam_left  ((tube_far_left_x + tube_center_left_x) / 2.0)
#define pano_seam_right ((tube_center_right_x + tube_far_right_x) / 2.0)
#define pano_side_rect_outer_l -2.0
#define pano_side_rect_outer_r  3.0

// PER-GENERATION PARAMETER STRUCT

struct NVGParams
{
	float crt_factor;
	float noise_factor;
	float scintillation;
	float sharpen_factor;
	float3 saturation;
	float gamma_adjust;
	float bloom_strength;
	float contrast;
	float mid_lift;
	float dist_att_strength;
	float halo_intensity;
	float image_blur;
};
float get_flip_down_scale()
{
	float lua_param_flip_down = floor(shader_param_7.x);
	return clamp(5 - (lua_param_flip_down / 20.0f), 1.0f, 5.0f);
}

float ellipse_dist(float2 pos, float2 center, float2 radii)
{
	float2 d = (pos - center) / radii;
	return dot(d, d);
}

// Stadium distance field: clamp x to [capL, capR], then ellipse distance to nearest cap point.
// One call replaces rect+cap pairs — the clamp handles the straight section implicitly.
float stadium_dist(float2 tc, float capL, float capR, float cy, float2 radii)
{
	float nearest_x = clamp(tc.x, capL, capR);
	return ellipse_dist(tc, float2(nearest_x, cy), radii);
}

// Unpack RGB from floor(y)*100 + floor(z) → P, then base-21 decode (0..20 each → /20)
float3 unpack_nvg_color()
{
	float hi = floor(shader_param_7.y);   // 0..92
	float lo = floor(shader_param_7.z);   // 0..99
	float P  = hi * 100.0 + lo;           // 0..9260
	float r  = floor(P / 441.0);
	float g  = floor(fmod(P, 441.0) / 21.0);
	float b  = fmod(P, 21.0);
	return float3(r / 20.0, g / 20.0, b / 20.0);
}

// GPNVG-18 panoramic mask (3 stadiums via stadium_dist)
float compute_pano_mask(float2 tc)
{
	float in_center = step(stadium_dist(tc, tube_center_left_x, tube_center_right_x, pano_cy, pano_radii), 1.0);
	float in_left   = step(stadium_dist(tc, pano_side_rect_outer_l, tube_far_left_x, pano_cy, pano_radii), 1.0);
	float in_right  = step(stadium_dist(tc, tube_far_right_x, pano_side_rect_outer_r, pano_cy, pano_radii), 1.0);
	return max(in_center, max(in_left, in_right));
}

// Seam profile: center side fades, side tube dips and recovers
float compute_seam_profile(float dx, float fade_width, float dip_width)
{
	float center_grad = saturate(-dx / fade_width);
	float t = max(dx, 0.0) / dip_width;
	float side_dip = 1.0 - 0.5 * t * exp(1.0 - t);
	return (dx >= 0.0) ? side_dip : center_grad;
}

// GPNVG-18 panoramic vignette (3 stadium smoothsteps + seams)
float calc_pano_vignette(float2 tc, float vignette_amount)
{
	float soft = max(vignette_amount * 1.5, 0.08);

	float vig_center = smoothstep(1.0, 1.0 - soft,
		stadium_dist(tc, tube_center_left_x, tube_center_right_x, pano_cy, pano_radii));
	float vig_left = smoothstep(1.0, 1.0 - soft,
		stadium_dist(tc, pano_side_rect_outer_l, tube_far_left_x, pano_cy, pano_radii));
	float vig_right = smoothstep(1.0, 1.0 - soft,
		stadium_dist(tc, tube_far_right_x, pano_side_rect_outer_r, pano_cy, pano_radii));

	// Screen-blend the three stadium vignettes
	float vig = 1.0 - (1.0 - vig_center) * (1.0 - vig_left) * (1.0 - vig_right);

	// Seam bars
	float dx_l = pano_seam_left - tc.x;
	float dx_r = tc.x - pano_seam_right;

	vig *= lerp(1.0, compute_seam_profile(dx_l, 0.08, 0.05), 0.10);
	vig *= lerp(1.0, compute_seam_profile(dx_r, 0.08, 0.05), 0.10);

	return vig;
}
// Lens mask — branchless tube selection (Credit: LVutner)
float compute_lens_mask(float2 masktc, float num_tubes)
{
	masktc.y *= get_flip_down_scale();

	float is_single_center = step(abs(num_tubes - 1.0), 0.02);
	float is_single_left   = step(abs(num_tubes - 1.1), 0.02);
	float is_single_right  = step(abs(num_tubes - 1.2), 0.02);
	float is_dual          = step(abs(num_tubes - 2.0), 0.02);
	float is_quad          = step(abs(num_tubes - 4.0), 0.02);
	float is_quad_classic  = step(abs(num_tubes - 4.1), 0.02);

	float single_c = step(distance(masktc, single_tube_centered),    tube_radius);
	float single_l = step(distance(masktc, single_tube_offset_left), tube_radius);
	float single_r = step(distance(masktc, single_tube_offset_right),tube_radius);

	float dual = max(
		step(distance(masktc, dual_tube_offset_1), tube_radius),
					 step(distance(masktc, dual_tube_offset_2), tube_radius));

	float quad = compute_pano_mask(masktc);

	// Classic (original Beef's) quad: four discrete circular tubes
	float quad_classic = max(
		max(step(distance(masktc, quad_tube_offset_1), tube_radius),
			step(distance(masktc, quad_tube_offset_2), tube_radius)),
		max(step(distance(masktc, quad_tube_offset_3), tube_radius),
			step(distance(masktc, quad_tube_offset_4), tube_radius)));

	return is_single_center * single_c
	+ is_single_left   * single_l
	+ is_single_right  * single_r
	+ is_dual          * dual
	+ is_quad          * quad
	+ is_quad_classic  * quad_classic;
}
// Aspect ratio correction (Credit: LVutner)

float2 aspect_ratio_correction (float2 tc)
{
	tc.x -= 0.5f;
	tc.x *= (screen_res.x / screen_res.y);
	tc.x += 0.5f;
	return tc;
}

// CRT effect (Credit: Mattias, ShaderToy)

float2 curve_texturecoords(float2 curved_tc)
{
	curved_tc = (curved_tc - 0.5) * 2.0;
	curved_tc *= 1.1;
	curved_tc.x *= 1.0 + pow((abs(curved_tc.y) / 5.0), 2.0);
	curved_tc.y *= 1.0 + pow((abs(curved_tc.x) / 4.0), 2.0);
	curved_tc  = (curved_tc / 2.0) + 0.5;
	curved_tc =  curved_tc *0.92 + 0.04;
	return curved_tc;
}

float3 make_crt_ified(float3 fragColor, float2 tc )
{
	float2 uv = curve_texturecoords(tc);
	float3 col;
	float x =  sin(0.3*timers.x+uv.y*21.0)*sin(0.7*timers.x+uv.y*29.0)*sin(0.3+0.33*timers.x+uv.y*31.0)*0.0017;

	col.r = fragColor.r;
	col.g = fragColor.g;
	col.b = fragColor.b;

	col.r += 0.08f * fragColor.r;
	col.g += 0.05f * fragColor.g;
	col.b += 0.08f * fragColor.b;

	col = clamp(col*0.6+0.4*col*col*1.0,0.0,1.0);

	float vig = (0.0 + 1.0*16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y));
	col *= float(pow(abs(vig),0.3));

	col *= float3(0.95,1.05,0.95);
	col *= 2.8;

	float scans = clamp( 0.35+0.35*sin(uv.y*screen_res.y*2.0), 0.0, 1.0);

	float s = pow(scans,1.7);
	col = col*float( 0.4+0.7*s) ;

	col *= 1.0+0.01*sin(110.0*timers.x);

	float in_bounds = step(0.0, uv.x) * step(uv.x, 1.0) * step(0.0, uv.y) * step(uv.y, 1.0);
	col *= in_bounds;

	col*=1.0-0.65*float(clamp(((tc.x % 2.0)-1.0)*2.0,0.0f,1.0f));

	float comp = smoothstep( 0.1, 0.9, sin(timers.x) );

	fragColor = col;
	return fragColor;
}
// Vignette calculator — branchless

float calc_vignette (float num_tubes, float2 tc, float vignette_amount)
{
	float2 corrected_texturecoords = aspect_ratio_correction(tc);
	corrected_texturecoords.y *= get_flip_down_scale();

	float is_single_center = step(abs(num_tubes - 1.0), 0.02);
	float is_single_left   = step(abs(num_tubes - 1.1), 0.02);
	float is_single_right  = step(abs(num_tubes - 1.2), 0.02);
	float is_dual          = step(abs(num_tubes - 2.0), 0.02);
	float is_quad          = step(abs(num_tubes - 4.0), 0.02);
	float is_quad_classic  = step(abs(num_tubes - 4.1), 0.02);

	#define TUBE_VIG(center) pow(smoothstep(tube_radius, tube_radius - vignette_amount, distance(corrected_texturecoords, center)), 3)

	float vig_sc = TUBE_VIG(single_tube_centered);
	float vig_sl = TUBE_VIG(single_tube_offset_left);
	float vig_sr = TUBE_VIG(single_tube_offset_right);

	float vig_d1 = TUBE_VIG(dual_tube_offset_1);
	float vig_d2 = TUBE_VIG(dual_tube_offset_2);
	float vig_dual = 1.0 - (1.0 - vig_d1) * (1.0 - vig_d2);

	float vig_quad = calc_pano_vignette(corrected_texturecoords, vignette_amount);

	// Classic (original Beef's) quad vignette: four circular tubes, screen-blended
	float vig_q1 = TUBE_VIG(quad_tube_offset_1);
	float vig_q2 = TUBE_VIG(quad_tube_offset_2);
	float vig_q3 = TUBE_VIG(quad_tube_offset_3);
	float vig_q4 = TUBE_VIG(quad_tube_offset_4);
	float vig_quad_classic = 1.0 - (1.0 - vig_q1) * (1.0 - vig_q2) * (1.0 - vig_q3) * (1.0 - vig_q4);

	#undef TUBE_VIG

	return is_single_center * vig_sc
	+ is_single_left   * vig_sl
	+ is_single_right  * vig_sr
	+ is_dual          * vig_dual
	+ is_quad          * vig_quad
	+ is_quad_classic  * vig_quad_classic;
}

///// blur functions
float IGN_calc(float2 uv)
{
	uv = frac(uv);
	float f = 0.06711056 * uv.x + 0.00583715 * uv.y;
	return 6.28*(frac(52.9829189 * frac(f)));
}

float2x2 rot(float a)
{
	float c;
	float s;
	sincos(a,s,c);
	return float2x2 (c, s, -s, c);
}

// Luma sharpen (Credit: xwize, ShaderToy)
float3 YUVFromRGB( float3 color)
{
	return float3 (color.r * 0.299 + color.g * 0.587 + color.b * 0.114,
				   color.r * -0.147 + color.g * -0.289 + color.b * 0.436,
				color.r * 0.615 + color.g * -0.515 + color.b * -0.100);
}

float3 RGBFromYUV ( float3 color)
{
	return float3 (color.r * 1.0 + color.b * 1.140,
				   color.r * 1.0 + color.g * -0.395 + color.b * 0.581 ,
				color.r * 1.0 + color.g * 2.032);
}

float extractLuma(float3 c)
{
	return (c.r * 0.299 + c.g * 0.587 + c.b * 0.114);
}

float3 luma_sharpen(float3 image, float2 uv)
{
	float3 yuv = YUVFromRGB(image);
	float2 imgSize = screen_res.xy;
	float accumY = 0.0;
	for(int i = -1; i <= 1; ++i) {
		for(int j = -1; j <= 1; ++j) {
			float2 offset = float2(i,j) / imgSize;
			float s = extractLuma(s_blur_2.SampleLevel(smp_rtlinear,uv + offset,0).rrr);
			float notCentre = min(float(i*i + j*j),1.0);
			accumY += s * (9.0 - notCentre*10.0);
		}
	}
	accumY /= 9.0;
	float gain = 0.9;
	accumY = (accumY + yuv.x)*gain;
	image = RGBFromYUV (float3(accumY,yuv.y,yuv.z));
	return image;
}
// Blurred depth sampling

float blurred_depth (float2 tc)
{
	// NV colour is display-sized after DLSS/FSR; depth remains scene-sized.
	uint depth_width, depth_height;
	s_position.GetDimensions(depth_width, depth_height);
	float2 depth_size = float2(depth_width, depth_height);
	float Pi = 6.28318530718;
	float Directions = 12.0;
	float Quality = 4.0;
	float Size = 6;
	float2 Radius = Size/screen_res.xy;

	float center_depth = s_position.Load( int3( (tc)*depth_size, 0 ), 0 ).z;
	center_depth = (center_depth < 0.001) ? 10000.0 : center_depth;

	float depth_average = 0;
	float depth_sample;
	float weight = 0.0;
	float total_weight = 0.0;

	for(float i=1.0; i<=Quality; i++)
	{
		for( float d=0.0; d<Pi; d+=Pi/Directions)
		{
			depth_sample = s_position.Load( int3( ((tc+float2(cos(d),sin(d))*Radius*i) * depth_size), 0 ), 0 ).z;
			depth_sample = (depth_sample < 0.001) ? 10000.0 : depth_sample;

			float sigma_depth = 2.0;
			float depth_diff = abs(depth_sample - center_depth);
			float bilateral = exp(-(depth_diff * depth_diff) / (2.0 * sigma_depth * sigma_depth));
			float spatial = Quality - pow((i - 1), 0.5);
			weight = bilateral * spatial;

			depth_average += depth_sample * weight;
			total_weight += weight;
		}
	}
	depth_average /= total_weight;
	return depth_average;
}
////////////////////////////////////////////////////////////////////
//////// GLITCH EFFECT /////////////////

float3 glitchEffect( float3 image, float2 tc, float power)
{
	float time = fmod(timers.x, 32.0);
	float GLITCH = saturate(power);

	float rnd0 = rand(timers.x);
	float r0 = saturate((1.0-GLITCH)*0.7 + rnd0);
	float rnd1 = rand(floor(tc.x*10.0f * r0) / (10.0f * r0) * time);
	float r1 = 0.5 - (0.5 * GLITCH) + rnd1;
	r1 = 1.0 - max( 0.0, r1);
	float rnd2 = rand(floor(tc.y*40.0f * r1) / (40.0f * r1) * time);
	float r2 = saturate(rnd2);
	float rnd3 = rand(floor(tc.y*10.0f * r0) / (10.0f * r0) * time);
	float r3 = (1.0 - saturate(rnd3 + 0.8)) - 0.1f;

	float pxrnd = rand( dot(tc.x,tc.y) + time );
	float test_result = 2.0 * step(0.5, rnd0) - 1.0;
	float ofs = 0.05 * r2 * GLITCH * test_result;
	ofs += 0.5 * pxrnd * ofs;
	tc.y += 0.1 * r3 * GLITCH;

	int NUM_SAMPLES = 10;
	float RCP_NUM_SAMPLES_F = 1.0 / (float)NUM_SAMPLES;
	float3 sum = 0.0f;
	float3 wsum = 0.0f;

	for( int i=0; i<NUM_SAMPLES; ++i )
	{
		float t = (float)i * RCP_NUM_SAMPLES_F;
		tc.x = saturate(tc.x + ofs * t);
		float3 samplecol = s_blur_2.SampleLevel(smp_rtlinear, tc, 0).rgb;
		sum += samplecol;
		wsum += 1.0f;
	}
	return sum.rgb / wsum;
}

// Bloom gather: gaussian-weighted rings over the quarter and eighth res buffers.
// A single thresholded sample of s_blur_8 gave a flat disc with a hard rim; a wide
// weighted gather falls off smoothly instead. Radius scales with resolution.

float3 bloom_gather(float2 tc)
{
	float tau = 6.28318530718;
	float px_scale = screen_res.y / 1080.0;
	float ring_step = 10.0 * px_scale;	// pixels between rings (outer ring = 3x)

	float3 sum = 0.5 * (s_blur_8.SampleLevel(smp_rtlinear, tc, 0).rgb
					  + s_blur_4.SampleLevel(smp_rtlinear, tc, 0).rgb);
	float wsum = 1.0;

	for (int i = 1; i <= 3; i++)
	{
		float w = exp(-0.5 * float(i * i) / 2.56);	// sigma = 1.6 rings
		float a0 = float(i) * 0.39;					// rotate each ring, avoids spokes

		for (int d = 0; d < 8; d++)
		{
			float s, c;
			sincos(a0 + float(d) * tau / 8.0, s, c);
			float2 offset = float2(c, s) * ring_step * float(i) / screen_res.xy;

			sum += w * 0.5 * (s_blur_8.SampleLevel(smp_rtlinear, tc + offset, 0).rgb
							+ s_blur_4.SampleLevel(smp_rtlinear, tc + offset, 0).rgb);
			wsum += w;
		}
	}

	return sum / wsum;
}

// helpers: rgb <-> hsv
float3 rgb2hsv(float3 c) {
	// from: common fast implementation
	float4 K = float4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
	float4 p = (c.g < c.b) ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
	float4 q = (c.r < p.x) ? float4(p.xyw, c.r) : float4(c.r, p.yzx);
	float d = q.x - min(q.w, q.y);
	float e = 1e-10;
	float h = abs(q.z + (q.w - q.y) / (6.0 * d + e));
	float s = d / (q.x + e);
	return float3(h, s, q.x);
}
float3 hsv2rgb(float3 c) {
	float3 K = float3(1.0, 2.0/3.0, 1.0/3.0);
	float3 p = abs(frac(c.x + K) * 6.0 - 3.0);
	return c.z * lerp(float3(1.0,1.0,1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

// Main NVG processing

float4 nvg_process(p_screen I, NVGParams params)
{
	// Setup
	float2 corrected_texturecoords = aspect_ratio_correction(I.tc0);
	float3 image = s_image.Sample(smp_rtlinear, I.tc0).xyz;
	float3 half_res_blur = s_blur_2.Sample(smp_rtlinear, I.tc0).rgb;
	float3 quarter_res_blur = s_blur_4.Sample(smp_rtlinear, I.tc0).rgb;
	float3 eighth_res_blur = s_blur_8.Sample(smp_rtlinear, I.tc0).rgb;

	float lua_param_nvg_num_tubes = frac(shader_param_8.x) * 10.0f;

	// Unpack MCM-driven params from shader_param_7
	// .x frac = bloom(0..9), .w floor = halo*10+imgblur(0..99), .w frac = near_blur(00..99)
	float lua_bloom_strength = frac(shader_param_7.x) * 10.0 / 9.0;
	float packed_w = floor(shader_param_7.w);
	float lua_halo_intensity = floor(packed_w / 10.0) / 9.0;
	float lua_image_blur = fmod(packed_w, 10.0) / 9.0;

	if (compute_lens_mask(corrected_texturecoords, lua_param_nvg_num_tubes) == 1)
	{

		float4 jitter = float4(
			frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 12345.0),
							   frac(sin(dot(I.tc0-fmod(I.tc0,float2(3.0,3.0)/screen_res.xy), float2(12.0, 78.0) + (timers.x) )) * 22738.0),
							   frac(sin(dot(I.tc0-fmod(I.tc0,float2(3.0,3.0)/screen_res.xy), float2(12.0, 78.0) + (timers.x) )) * 78372.0),
							   frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 37857.0));

		float depth = blurred_depth(I.tc0);

		// IMAGE BLUR — overall softness (0=sharp, 1=very soft)
		float img_blur = saturate(lua_image_blur);
		float3 blur_target = lerp(half_res_blur, quarter_res_blur, img_blur);
		image = lerp(image, blur_target, img_blur);

		// NEAR BLUR — additional depth-based blur for close objects (stacks on image blur)
		float lua_near_blur_strength = round(frac(shader_param_7.w) * 100.0) / 99.0;
		float lua_near_blur_range = max(round(frac(shader_param_7.z) * 10.0), 1.0);
		float near_factor = 1.0f - smoothstep(0, lua_near_blur_range, depth);
		float blur_mix = saturate(near_factor * lua_near_blur_strength);
		float3 near_blur_source = lerp(half_res_blur, quarter_res_blur, saturate(img_blur + 0.5));
		image = lerp(image, near_blur_source, blur_mix);

		// GAMMA ADJUST
		image = pow(image, params.gamma_adjust);

		// DISTANCE ATTENUATION
		if (depth < 1000 && params.dist_att_strength > 0.001)
		{
			float light_att = pow(1.0f - (smoothstep(5, 500, depth)), 0.5);
			float att_mask = 1.0 - smoothstep(0.7, 1.0, image.r + image.g);
			image.r *= lerp(1.0, light_att + 0.4f, att_mask * params.dist_att_strength);
		}

		// GLITCH EFFECT
		float lua_param_glitch_power = fmod(shader_param_8.z, 1.0f);
		image = lerp(image, glitchEffect(image, I.tc0, lua_param_glitch_power), 0.9 * step(0.001, lua_param_glitch_power));

		// CRT
		image = lerp(image, make_crt_ified(half_res_blur, I.tc0), params.crt_factor);

		// NOISE
		float lua_param_nvg_gain_current = floor(shader_param_8.y) / 10.0f;
		image.r += jitter.y * (params.noise_factor * pow(lua_param_nvg_gain_current, 0.5));

		// SCINTILLATION
		float scint_threshold = params.scintillation - (1.0 - params.scintillation) * (lua_param_nvg_gain_current - 1.0f);
		image.r = max(image.r, step(scint_threshold, jitter.z));

		float washout_thresh = (round(frac(shader_param_8.y) * 10.0) + 1.0) / 20.0; // 0.05-0.50
		float threshold = 1.0 - (washout_thresh);
		// --- COLOR TINT ---
		// Convert to monochrome luminance, apply NVG color
		float luma = saturate((image.r * 3.0f + image.b) / 4.0f + image.g); // MY TONEMAP
		float3 packed_color = unpack_nvg_color();
		float3 nvg_active_color = saturate(packed_color);
		image.rgb = luma * nvg_active_color;

		// --- WASHOUT (white-hot pixels) ---
		float washout_softness = 0.05;
		float washout_mask = smoothstep(saturate(threshold - washout_softness), threshold, luma);
		image.rgb = lerp(image.rgb, float3(1.0, 1.0, 1.0), washout_mask);

		// --- CRUSH BLACK ---
		//float crush_black = 1.0 - smoothstep(0.0, 0.10, luma);
		//image.rgb *= (1.0 - crush_black);

		// --- BLOOM (soft glow)
		// Wide gather, long quadratic ramp below the washout threshold, screen blend:
		// the glow fades out with no visible rim and never clips to a flat colour.
		float3 bloom_buf = bloom_gather(I.tc0);
		float bloom_luma = saturate((bloom_buf.r * 3.0f + bloom_buf.b) / 4.0f + bloom_buf.g); // MY TONEMAP
		float bloom_softness = 0.45; // width of the ramp below threshold (was 0.2: narrow ramp = hard rim)
		float bloom_lo = max(threshold - bloom_softness, 0.0);
		float bloom_wide = saturate((bloom_luma - bloom_lo) / max(threshold - bloom_lo, 0.001));
		bloom_wide *= bloom_wide;
		float3 hsv = rgb2hsv(nvg_active_color);
		hsv.y = lerp(hsv.y, 1.0, lua_halo_intensity);
		float targetValue = 0.75;
		hsv.z = lerp(hsv.z, targetValue, lua_halo_intensity * 0.8);
		float3 tint = hsv2rgb(hsv);
		float3 bloom_add = saturate(lua_bloom_strength * bloom_wide * tint);
		image.rgb = 1.0 - (1.0 - saturate(image.rgb)) * (1.0 - bloom_add);

		// CONTRAST
		float3 midpoint = 0.5 * nvg_active_color;
		image.rgb = saturate((image.rgb - midpoint) * params.contrast + midpoint);

		if (params.sharpen_factor > 0.001)
		{
			float3 sharpened = luma_sharpen(image, I.tc0);
			image = lerp(image, sharpened, saturate(params.sharpen_factor));
		}

		// VIGNETTE
		float lua_param_vignette_current = floor(shader_param_8.z) / 100.0f;
		float vignette = calc_vignette(lua_param_nvg_num_tubes, I.tc0, lua_param_vignette_current);
		image = saturate(image * vignette);

		return float4(image, 1.0);
	}
	else
	{
		// Outside tubes — branchless mode select
		float mode = round(frac(shader_param_8.w) * 10.0);

		float3 result_blur  = clamp(half_res_blur, 0.0, 1.0);
		float3 result_black = float3(0.0, 0.0, 0.0);
		float3 result_clear = clamp(image, 0.0, 1.0);

		float is_blur  = step(mode, 0.5) * step(-0.5, mode);
		float is_black = step(0.5, mode) * step(mode, 1.5);
		float is_clear = step(1.5, mode);

		image = is_blur * result_blur + is_black * result_black + is_clear * result_clear;

		return float4(image, 1.0);
	}
}
