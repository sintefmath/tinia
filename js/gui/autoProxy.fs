//#extension GL_EXT_frag_depth : enable

uniform sampler2D rgbImage;
uniform sampler2D depthImg;

varying highp vec2 vTextureCoord;
varying highp float depth;
varying highp vec2 vertPos;
varying highp vec4 glpos; // debugging
varying highp vec3 ndc_to_fs; // debugging
varying highp vec4 vert_wb_to_fs; // debugging
varying highp vec4 vert_eb_to_fs; // debugging
varying highp vec2 ssFactor;

uniform highp mat4 PM;
uniform highp mat4 MV;
uniform highp mat4 depthPMinv;
uniform highp mat4 depthMVinv;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatSize;
uniform highp float splatOverlap;
uniform highp int splatSetIndex;
uniform highp int mostRecentOffset;

#define PI 3.1415926535

// For debugging
uniform int debugSplatCol;
uniform int decayMode;
uniform int roundSplats;
uniform int transpBackground;
uniform int fragDepthTest;
uniform int ignoreIntraSplatTexCoo;
uniform int splatOutline;
uniform highp int adjustTCwithFactorFromVS;




void main(void)
{
    //    gl_FragDepthEXT = depth;
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
    
    // Decay factor < 1.0 only outside of normal-sized splat
    if (false) {
        highp vec2 c2 = splatOverlap*c;   // c in splatOverlap*[-0.5, 0.5]^2, [-0.5, 0.5]^2 is the 'non-overlap' part
        highp float r_squared2 = dot(c2, c2);
        decay = exp(-1.0*r_squared2);
        if ( (abs(c2.x)<=0.5) && (abs(c2.y)<=0.5) ) {
            // We are inside the non-overlap part
            decay = 1.0;
        }
    }

    // Adjusting for intra-splat texture coordinate
    // Hmm. But is this really correct? Shouldn't this be done between the "un-projection" and the new transformation?!
    // Note that it should be correct if the proxy model's view is the same as the current view!
    highp vec2 tc = vTextureCoord + vec2( c.x/splats_x*splatOverlap, c.y/splats_y*splatOverlap );
    //    highp vec2 tc = vTextureCoord + vec2( c.x/splats_x, c.y/splats_y );
    if (adjustTCwithFactorFromVS>0)
	tc = vTextureCoord + vec2( c.x/splats_x/ssFactor.x/splats_x*splatOverlap, c.y/splats_y/ssFactor.y/splats_y*splatOverlap );
    if (ignoreIntraSplatTexCoo>0) 
	tc = vTextureCoord;

    // Discarding fragments that would look up depth and color outside the rendered scene
    // Note that this will remove parts of primitives containing "background pixels" from the rgb texture, it will not cause these
    // to be replaced by other parts of the geometry, these are simply not available in the proxy model.
    // Note also that this will not remove parts of primitives that are rotated outside of the correct geometry.
    // That is impossible to do, we do not have the necessary information. What we can do, is to adjust the fragment depth, which we
    // do below.
    if (transpBackground>0) {
        highp float intra_splat_depth = texture2D(depthImg, tc).r + (texture2D(depthImg, tc).g + texture2D(depthImg, tc).b/255.0)/255.0;
        if ( intra_splat_depth > 0.9999 )
            discard;
        if (splatSetIndex==-1)
            clamp(intra_splat_depth = intra_splat_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
    }

    if (decayMode==0) {
        decay = 1.0;
    }

    gl_FragColor = vec4( decay * texture2D( rgbImage, tc ).xyz, src_alpha );

    if (debugSplatCol>0) {
        if (splatSetIndex==0)  gl_FragColor = vec4(decay, 0.0, 0.0, src_alpha);
        if (splatSetIndex==1)  gl_FragColor = vec4(0.0, decay, 0.0, src_alpha);
        if (splatSetIndex==2)  gl_FragColor = vec4(0.0, 0.0, decay, src_alpha);
        if (splatSetIndex==3)  gl_FragColor = vec4(decay, decay, 0.0, src_alpha); // 3) yellow?
        if (splatSetIndex==4)  gl_FragColor = vec4(0.0, decay, decay, src_alpha); // 4) cyan?
        if (splatSetIndex==5)  gl_FragColor = vec4(decay, 0.0, decay, src_alpha); // 5) magenta?
        if (splatSetIndex==6)  gl_FragColor = vec4(0.5*decay, 0.0, 0.0, src_alpha);
        if (splatSetIndex==7)  gl_FragColor = vec4(0.0, 0.5*decay, 0.0, src_alpha);
        if (splatSetIndex==8)  gl_FragColor = vec4(0.0, 0.0, 0.5*decay, src_alpha);
        if (splatSetIndex==9)  gl_FragColor = vec4(0.5*decay, 0.5*decay, 0.0, src_alpha);
        if (splatSetIndex==-1)
	    if ( r_squared > 0.16 ) gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha); else gl_FragColor = vec4(1.0, 0.0, 0.0, src_alpha);
    } else {
        // To visualize the "most recent proxy model" even when not in debug-colour-mode:
	//         if (splatSetIndex==-1)
	//             if ( ( r_squared > 0.2 ) && ( r_squared < 0.25 ) )
	//                 gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
    }
    if (splatOutline>0) {
	highp float tmp = 0.5-0.002*float(splats_x);
	if ( (c.x<-tmp) || (c.x>tmp) || (c.y<-tmp) || (c.y>tmp) )
	    gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
    }
    

    // if (fragDepthTest>0) {
    //     gl_FragDepthEXT = 0.5;
    // }

    //    gl_FragColor = vec4(depth, vTextureCoord, src_alpha);
    
    if (fragDepthTest>0) {

        // gl_FragColor = vec4(0.0, 0.0, abs(gl_FragCoord.z-depth)*10.0, src_alpha);
	// Verifies that we understand in the vs how depth is computed


        // Re-computing the window coordinates with the idea to see how z_window is computed, and then modify this, or
        // use it to discard fragments. Or both, really. We want to discard fragments outside the silhouette, if
        // possible, and to adjust them inside. The former may not be possible?

        // We have gl_Position = (x, y, z, w)_clip, and the division by w_c is done after the VS stage.
        // We have (x, y, z)_ndc = (x_c/w_c, y_c/w_c, z_c/w_c) and then window coordinates:
        // (x, y, z)_window = (0.5*p_x*x_ndc + o_x, 0.5*p_y*y_ndc + o_y, 0.5*(f-n)*z_ndc + 0.5*(n+f)).

        
        
        highp float new_depth = ( texture2D( depthImg, vTextureCoord ).r +
                                  texture2D( depthImg, vTextureCoord ).g / 255.0 +
                                  texture2D( depthImg, vTextureCoord ).b / (255.0*255.0) );
        // Should be < 1.0, should have been discarded in the VS otherwise.
        if (splatSetIndex==-1) {
            new_depth = clamp(new_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
        }
        
        highp float x_ndc = vertPos.x;
        highp float y_ndc = vertPos.y;
        highp float z_ndc = 2.0*new_depth - 1.0;
        // gl_FragColor = vec4(ndc_to_fs-vec3(x_ndc, y_ndc, z_ndc), src_alpha); // Checked

        highp vec4 vert_eb = depthPMinv * vec4( x_ndc, y_ndc, z_ndc, 1.0 );
        // gl_FragColor = vec4((vert_eb-vert_eb_to_fs).xyz, src_alpha); // ok

        highp vec4 vert_wb = depthMVinv * vert_eb;
        // gl_FragColor = vec4((vert_wb-vert_wb_to_fs).xyz, src_alpha);
	// Mostly ok, but some odd primitives. Truncation/conversion artefacts?

        highp vec4 pos = PM * MV * vert_wb;
        // gl_FragColor = vec4((pos-glpos).xyz, src_alpha); // Mostly ok, but some odd primitives. Truncation/conversion artefacts?


        // Now with intra-splat position:

        tc = vTextureCoord + vec2( c.x/splats_x*splatOverlap, c.y/splats_y*splatOverlap );

        highp float intra_splat_depth = ( texture2D( depthImg, tc ).r +
                                          texture2D( depthImg, tc ).g / 255.0 +
                                          texture2D( depthImg, tc ).b / (255.0*255.0) );
        if (splatSetIndex==-1) {
	    intra_splat_depth = clamp(intra_splat_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
        }
        // gl_FragColor = vec4(abs(intra_splat_depth-new_depth), 0.0, 0.0, src_alpha); // tja?
        




        // Adjusting for intra-splat texture coordinate
        // highp vec2 tc = vTextureCoord + vec2( c.x/splats_x*splatOverlap, c.y/splats_y*splatOverlap );

        // Since the relation between vertex and texture coordinate is: s = 0.5*(x+1), and s_i = s+c.x/splats_x*overlap, we get
        // s_i = 0.5*(x_i + 1) <=> x_i = 2*s_i - 1 = 2s + 2c.x/splats_x*overlap - 1.
        // Then, x_i - x = 2s + 2c.x/splats_x*overlap - 1 - (2s-1) = 2c.x/splats_x*overlap. (*)
        
        highp float x_ndc_is = vertPos.x + 2.0/splats_x*splatOverlap * c.x; // c.x=0.5 & overlap=1 => x + 1/splats. Seems reasonable.
        highp float y_ndc_is = vertPos.y + 2.0/splats_y*splatOverlap * c.y;
        highp float z_ndc_is = 2.0*intra_splat_depth - 1.0;

        if (false) {
            highp float d = length( vec3(x_ndc_is, y_ndc_is, z_ndc_is) - vec3(x_ndc, y_ndc, z_ndc) );
            // Should be zero in the center of splats, but what about the rim? How do we normalize this length?  If we
            // let y_ndc_i = y_ndc and z_ndc_i = z_ndc, we see that the distance should be 1.0/splats_x*splatOverlap,
            // cf. the comment (*) above.
            d = d / ( 1.0/splats_x*splatOverlap );
            gl_FragColor = vec4(d, 0.0, 0.0, src_alpha); // Seems ok
        }

        highp vec4 vert_eb_is = depthPMinv * vec4( x_ndc_is, y_ndc_is, z_ndc_is, 1.0 );
        highp vec4 vert_wb_is = depthMVinv * vert_eb_is;
        highp vec4 pos_is = PM * MV * vert_wb_is;
//         gl_FragColor = vec4(10.0*clamp(  (pos_is-pos).x, 0.0, 1.0),
//                             10.0*clamp( -(pos_is-pos).y, 0.0, 1.0),
//                             0.0, src_alpha);
        // Ok, seems plausible. Freshly generated proxy model displays red to the right, internally, in each splat,
        // green on the lower side. (Flipped y-axis.) When rotated, this turns around, which it should.

	// Next: Use pos_is to reconstruct new and more correct intra-splat texture coordinate adjustments.


       





	// Note that we cannot change the fragment's window coordinates, even though we can set the depth (through the
	// extension GL_EXT_frag_depth.)


    }
    
    
}




// Hmm. Ville det vaert lurere aa gjoere alle "de-projeksjonene" når dybde-buffer ble mottatt fra server? Ville da
// slippe aa gjoere det for hvert renderkall. Ville ogsaa slippe aa lagre teksturene. Maatte i steden lagre hele
// splat-settet.
