uniform sampler2D rgbImage;
uniform sampler2D uSampler;

varying highp vec2 vTextureCoord;
varying highp float depth;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatOverlap;
uniform int splatSetIndex;



// For debugging
uniform int debugSplatCol;
uniform int decayMode;
uniform int roundSplats;
uniform int transpBackground;

//#define DEBUG_SHOW_CIRCULAR_COMPLEMENT
//#define DEBUG_SHOW_NON_OVERLAP_SQUARE
//#define DEBUG_SHOW_SPLAT_CENTER
#define DEBUG_SPLAT_SET_COLOUR

#define PI 3.1415926535



void main(void)
{
    highp float src_alpha = 1.0;
    
    highp vec2 c = gl_PointCoord-vec2(0.5);   // c in [-0.5, 0.5]^2
    highp float r_squared = dot(c, c);        // r_squared in [0, 0.5], radius squared for the largest inscribed circle is 0.25
    					      // radius squared for the smallest circle containing the 'square splat' is 0.5
    
#ifndef DEBUG_SHOW_CIRCULAR_COMPLEMENT
    if ( (roundSplats>0) && (r_squared>0.25) ) {
        discard;
    }
#endif

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

#ifdef DEBUG_SHOW_CIRCULAR_COMPLEMENT
    // To help visualizing the splats during testing/debugging, outside of circular splats padded with white to squares
    if ( (roundSplats>0) && (r_squared>0.25) ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
#endif
    
#ifdef DEBUG_SHOW_NON_OVERLAP_SQUARE
    // To help visualizing the splats during testing/debugging. Rendering a frame around the 'non-overlap-part' of the splat.
    if ( ( abs(abs(c.x*splatOverlap)-0.5) < 0.05 ) && (abs(c.y*splatOverlap)<=0.5) ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
    if ( ( abs(abs(c.y*splatOverlap)-0.5) < 0.05 ) && (abs(c.x*splatOverlap)<=0.5) ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
#endif

#ifdef DEBUG_SHOW_SPLAT_CENTER
    // To help visualizing the splats during testing/debugging. Rendering a white dot in the center of the splat
    if ( dot(c, c) <= 0.003 ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
#endif

#ifdef DEBUG_SPLAT_SET_COLOUR
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
        if (splatSetIndex==-1)
            if ( ( r_squared > 0.16 ) && ( r_squared < 0.25 ) )
                gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
    }
#endif
}
