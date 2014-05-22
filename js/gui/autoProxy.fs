// #extension EXT_frag_depth : enable
#extension GL_EXT_frag_depth : enable
// #extension WEBKIT_EXT_frag_depth : enable
// #extension WEBKIT_GL_EXT_frag_depth : enable

// #extension ANGLE_instanced_arrays : enable
// #extension EXT_texture_filter_anisotropic : enable
// #extension WEBKIT_EXT_texture_filter_anisotropic : enable
// #extension OES_element_index_uint : enable
// #extension OES_standard_derivatives : enable
// #extension OES_texture_float : enable
// #extension OES_texture_float_linear : enable
// #extension OES_texture_half_float : enable
// #extension OES_texture_half_float_linear : enable
// #extension OES_vertex_array_object : enable
// #extension WEBGL_compressed_texture_s3tc : enable
// #extension WEBKIT_WEBGL_compressed_texture_s3tc : enable
// #extension WEBGL_depth_texture : enable
// #extension WEBKIT_WEBGL_depth_texture : enable
// #extension WEBGL_draw_buffers : enable
// #extension WEBGL_lose_context : enable
// #extension WEBKIT_WEBGL_lose_context : enable
// #extension WEBGL_debug_renderer_info : enable



uniform sampler2D rgbImage;
uniform sampler2D uSampler;

varying highp vec2 vTextureCoord;
varying highp float depth;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatOverlap;
uniform int splatSetIndex;

#define PI 3.1415926535

// For debugging
uniform int debugSplatCol;
uniform int decayMode;
uniform int roundSplats;
uniform int transpBackground;



void main(void)
{
    highp float src_alpha = 1.0;
    
    highp vec2 c = gl_PointCoord-vec2(0.5);   // c in [-0.5, 0.5]^2
    highp float r_squared = dot(c, c);        // r_squared in [0, 0.5], radius squared for the largest inscribed circle is 0.25
    					      // radius squared for the smallest circle containing the 'square splat' is 0.5

    if ( (roundSplats>0) && (r_squared>0.25) ) {
        discard;
    }

    // Decay factor = 1.0 in splat center, tending toward 0 at circular rim.
    // In the distance sqrt(2*(0.5/overlap)^2), i.e., r_squared=2*(0.5/overlap)^2, we get decay=0.25,
    // which corresponds to the maximum "depth-disabled accumulated splat value" of 1.0 for a regular grid of splats.
    highp float decay = exp(splatOverlap*splatOverlap*log(1.0/16.0)*r_squared);

    // This is useful if blending is enabled with blend_func(0.5, 0.5) and depth testing disabled, and will
    // cause the accumulated color to not be less than 1
    // decay = decay * 10.0;
    
#if 0
    // Decay factor < 1.0 only outside of normal-sized splat
    highp vec2 c2 = splatOverlap*c;   // c in splatOverlap*[-0.5, 0.5]^2, [-0.5, 0.5]^2 is the 'non-overlap' part
    highp float r_squared2 = dot(c2, c2);
    decay = exp(-1.0*r_squared2);
    if ( (abs(c2.x)<=0.5) && (abs(c2.y)<=0.5) ) {
        // We are inside the non-overlap part
        decay = 1.0;
    }
#endif

    // Adjusting for intra-splat texture coordinate
    highp vec2 tc = vTextureCoord + vec2( c.x/splats_x*splatOverlap, c.y/splats_y*splatOverlap );

    // Discarding fragments that would look up depth and color outside the rendered scene
    if ( transpBackground > 0 ) {
        highp float depth2 = ( texture2D( uSampler, tc ).r +
                               texture2D( uSampler, tc ).g / 255.0 +
                               texture2D( uSampler, tc ).b / (255.0*255.0) );
        if ( depth2 > 0.9999 )
            discard;
    }

    if (decayMode==0) {
        decay = 1.0;
    }

    gl_FragColor = vec4( decay * texture2D( rgbImage, tc ).xyz, src_alpha );

    if (debugSplatCol>0) {
        highp float al = atan(-c.y, c.x);
        if (al<0.0)
            al = al + 2.0*PI;
        if (splatSetIndex==0) {
            gl_FragColor = vec4(decay, 0.0, 0.0, src_alpha);
        }
        if (splatSetIndex==1) {
            gl_FragColor = vec4(0.0, decay, 0.0, src_alpha);
        }
        if (splatSetIndex==2) {
            gl_FragColor = vec4(0.0, 0.0, decay, src_alpha);
        }
        if (splatSetIndex==3) {
            gl_FragColor = vec4(decay, decay, 0.0, src_alpha); // 3) yellow?
        }
        if (splatSetIndex==4)
            gl_FragColor = vec4(0.0, decay, decay, src_alpha); // 4) cyan?
        if (splatSetIndex==5)
            gl_FragColor = vec4(decay, 0.0, decay, src_alpha); // 5) magenta?
        if (splatSetIndex==6)
            gl_FragColor = vec4(0.5*decay, 0.0, 0.0, src_alpha);
        if (splatSetIndex==7)
            gl_FragColor = vec4(0.0, 0.5*decay, 0.0, src_alpha);
        if (splatSetIndex==8)
            gl_FragColor = vec4(0.0, 0.0, 0.5*decay, src_alpha);
        if (splatSetIndex==9)
            gl_FragColor = vec4(0.5*decay, 0.5*decay, 0.0, src_alpha);
        if (splatSetIndex==-1)
	    if ( r_squared > 0.16 )
		gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
	    else
		gl_FragColor = vec4(1.0, 0.0, 0.0, src_alpha);
    } else {
//         if (splatSetIndex==-1)
//             if ( ( r_squared > 0.2 ) && ( r_squared < 0.25 ) )
//                 gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
    }
    


    //    gl_FragDepthEXT = 1.0;
    

}
