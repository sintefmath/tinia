uniform sampler2D rgbImage;

varying highp vec2 vTextureCoord;
varying highp float depth;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatOverlap;
uniform int splatSetIndex;



// For debugging
uniform int debugSplatCol;
uniform int decayMode;
uniform int pieSplats;
uniform int roundSplats;


//#define DEBUG_SHOW_CIRCULAR_COMPLEMENT
//#define DEBUG_SHOW_NON_OVERLAP_SQUARE
//#define DEBUG_SHOW_SPLAT_CENTER
#define DEBUG_SPLAT_SET_COLOUR

#define PI 3.1415926535



void main(void)
{
    highp float src_alpha = 1.0;
    
    // Hmm. Even when this is disabled, we still don't get all splats rendered. Why is this so? Shouldn't it be
    // necessary with the discard here?!  Ah. The explanation is that the splats are really rendered, but with the
    // background color, so they are not visible!
    if ( depth > 0.999 ) {
        // The depth should be 1 for fragments not rendered. It may be a problem that depth input is 'varying'.
        discard;
        // gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); // white
        // return;
    }

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
            if ( (pieSplats>0) && (! ((al>=0.0) && (al<0.5*PI)) ) )
                discard;
            gl_FragColor = vec4(decay, 0.0, 0.0, src_alpha);
        }
        if (splatSetIndex==1) {
            if ( (pieSplats>0) && (! ((al>=0.5*PI) && (al<PI)) ) )
                discard;
            gl_FragColor = vec4(0.0, decay, 0.0, src_alpha);
        }
        if (splatSetIndex==2) {
            if ( (pieSplats>0) && (! ((al>=PI) && (al<1.5*PI)) ) )
                discard;
            gl_FragColor = vec4(0.0, 0.0, decay, src_alpha);
        }
        if (splatSetIndex==3) {
            if ( (pieSplats>0) && (! ((al>=1.5*PI) && (al<2.0*PI)) ) )
                discard;
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
    }
#endif
}
