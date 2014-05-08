uniform sampler2D rgbImage;

varying highp vec2 vTextureCoord;
varying highp float depth;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatOverlap;



void main(void)
{
    // gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
    
    // Hmm. Even when this is disabled, we still don't get all splats rendered. Why is this so? Shouldn't it be necessary with the discard here?!
    // Ah. The explanation is that the splats are really rendered, but with the background color, so they are not visible!
    if ( depth > 0.999 ) {
        // The depth should be 1 for fragments not rendered. It may be a problem that depth
        // input is 'varying'.
        discard;
        // gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); // white
        // return;
    }
    
    // Just to see if this ever happens...
//     if ( depth < 0.01 ) {
//         gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); // yellow
//         return;
//     }

    highp vec2 c = gl_PointCoord-vec2(0.5);   // c in [-0.5, 0.5]^2
    highp float r_squared = dot(c, c);        // r_squared in [0, 0.5], radius squared for the largest inscribed circle is 0.25
    					      // radius squared for the smallest circle containing the 'square splat' is 0.5
    
    // For circular splats:
//     if ( r_squared > 0.25 ) {
//         discard;
//     }
    
    highp float decay = 1.0;
    
    // Decay factor = 1.0 in splat center, tending toward 0 of circular rim
//     decay = exp(-20.0*r_squared); // exp(-20.0*0.25) = 0.0067...
    
    // Decay factor < 1.0 only outside of normal-sized splat
//     highp vec2 c2 = splatOverlap*c;   // c in splatOverlap*[-0.5, 0.5]^2, [-0.5, 0.5]^2 is the 'non-overlap' part
//     highp float r_squared2 = dot(c2, c2);
//     decay = exp(-1.0*r_squared2);
//     if ( (abs(c2.x)<=0.5) && (abs(c2.y)<=0.5) ) {
//         // We are inside the non-overlap part
//         decay = 1.0;
//     }
    
    highp vec2 tc = vTextureCoord + vec2( c.x/splats_x*splatOverlap, c.y/splats_y*splatOverlap ); // Adjusting for intra-splat texture coordinate
    
    gl_FragColor = decay * texture2D( rgbImage, tc );
    
    // To help visualizing the splats during testing/debugging, outside of circular splats padded with white to squares
//     if ( r_squared > 0.25 ) {
//         gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
//     }
    
    // To help visualizing the splats during testing/debugging. Rendering a frame around the 'non-overlap-part' of the splat.
    if ( ( abs(abs(c.x*splatOverlap)-0.5) < 0.05 ) && (abs(c.y*splatOverlap)<=0.5) ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
    if ( ( abs(abs(c.y*splatOverlap)-0.5) < 0.05 ) && (abs(c.x*splatOverlap)<=0.5) ) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
}
