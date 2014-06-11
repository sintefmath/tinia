#extension GL_EXT_frag_depth : enable

#define USE_FRAG_DEPTH_EXT

uniform sampler2D rgbImage;
uniform sampler2D depthImg;

varying highp vec2 texCoo;
varying highp float depth;
varying highp float sampled_depth; // debugging (Splat-centered depth)
varying highp float sampled_depth_dx; // debugging (Splat-centered depth for the (1,0)-splat)
varying highp float sampled_depth_dy; // debugging (Splat-centered depth for the (1,0)-splat)
varying highp vec2 depth_e; // For approximating the intra-splat depth, depth = sampled_depth + depth_e' * c

varying highp float frag_depth;
varying highp vec2 frag_depth_e;

varying highp vec2 vertPos;
varying highp vec4 glpos; // debugging
varying highp vec3 ndc_to_fs; // debugging
varying highp vec4 vert_wb_to_fs; // debugging
varying highp vec4 vert_eb_to_fs; // debugging
varying highp vec2 ssFactor;
varying highp mat2 intraSplatTexCooTransform;
varying highp mat2 intraSplatTexCooTransform2;

uniform highp mat4 PM;
uniform highp mat4 MV;
uniform highp mat4 depthPMinv;
uniform highp mat4 depthMVinv;

uniform highp float splats_x;
uniform highp float splats_y;
uniform highp float splatOverlap;
uniform highp int splatSetIndex;
uniform highp int mostRecentOffset;
uniform highp int screenSpaceSized;
uniform highp int vp_width;
uniform highp int vp_height;

#define PI 3.1415926535

// For debugging
uniform int debugSplatCol;
uniform int decayMode;
uniform int roundSplats;
uniform int transpBackground;
uniform highp int fragDepthTest;
uniform int ignoreIntraSplatTexCoo;
uniform int splatOutline;
// uniform highp int adjustTCwithFactorFromVS;




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

    // Adjusting for intra-splat texture coordinate.
    highp vec2 tc = texCoo;
    tc = tc + vec2(0.5/float(vp_width), 0.5/float(vp_height)); // Must we add this to get sampling mid-texel?!
    if (!(ignoreIntraSplatTexCoo>0)) {
	tc = tc + intraSplatTexCooTransform * vec2(c.x, -c.y); // Flip needed because texture is flipped, while gl_PointCoord is not?!;
	// Or should the flip here and a corresponding in the intraSplatTexCooTransform-generation be removed simultaneously?!
	//tc = tc + (1.0/32.0)*vec2(c.x, c.y); // testing
    }

    // Discarding fragments that would look up depth and color outside the rendered scene
    // Note that this will remove parts of primitives containing "background pixels" from the rgb texture, it will not cause these
    // to be replaced by other parts of the geometry, these are simply not available in the proxy model.
    // Note also that this will not remove parts of primitives that are rotated outside of the correct geometry.
    // That is impossible to do, we do not have the necessary information. What we can do, is to adjust the fragment depth, which we
    // do below.
    highp float intra_splat_depth = texture2D(depthImg, tc).r + (texture2D(depthImg, tc).g + texture2D(depthImg, tc).b/255.0)/255.0;
    if ( (transpBackground>0) && (intra_splat_depth>0.999) ) {
        discard;
    }

    // If this is "the most recent proxy model", it should be brought some amount toward the front. Note that this is
    // not a very good solution for billboards, since it requires a very large offset to work properly. Possible
    // solutions to this: 1) Use non-gl_Point-primitives that have "transformed fragments", or 2) set depths using the
    // gl_FragDepth extension.
#ifndef USE_FRAG_DEPTH_EXT
    if (splatSetIndex==-1) {
        clamp(intra_splat_depth = intra_splat_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
    }
#endif

    // If the assumed-locally-planar geometry and the measured intra-splat depth deviates too much, the intra-splat
    // texture lookups will simply be wrong, so we might as well discard these fragments.
    highp float planar_depth = sampled_depth + dot( depth_e, intraSplatTexCooTransform2*vec2(c.x, -c.y) );
    // To visualize the difference between the assumed-locally-planar geometry and the measured intra-splat depth:
    // gl_FragColor = vec4( 1000.0*abs(planar_depth-intra_splat_depth)*vec3(1.0), src_alpha ); return;
    if ( abs(planar_depth-intra_splat_depth) > 0.5/1000.0 ) { // Values chosen by using the visualization above
        discard;
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
	highp float tmp = 0.5-0.0015*float(splats_x)/splatOverlap;
	if ( (c.x<-tmp) || (c.x>tmp) || (c.y<-tmp) || (c.y>tmp) )
	    gl_FragColor = vec4(1.0, 1.0, 1.0, src_alpha);
    }
    
   

    if (false) { // (fragDepthTest>0) {

        //----------------------------------------------------------------------------------------------------
        // 
        // Reconstructing vertex from intra-splat coordinates to obtain proper depth.
        //
        //----------------------------------------------------------------------------------------------------

        // gl_FragColor = vec4(0.0, 0.0, abs(gl_FragCoord.z-depth)*10.0, src_alpha);
	// Verifies that we understand in the vs how depth is computed

        // Re-computing the window coordinates with the idea to see how z_window is computed, and then modify this, or
        // use it to discard fragments. Or both, really. We want to discard fragments outside the silhouette, if
        // possible, and to adjust them inside. The former may not be possible?

        // We have gl_Position = (x, y, z, w)_clip, and the division by w_c is done after the VS stage.
        // We have (x, y, z)_ndc = (x_c/w_c, y_c/w_c, z_c/w_c) and then window coordinates:
        // (x, y, z)_window = (0.5*p_x*x_ndc + o_x, 0.5*p_y*y_ndc + o_y, 0.5*(f-n)*z_ndc + 0.5*(n+f)).


        
        // This is the depth for the vertex in the VS, i.e., not intra-splat depth.
        highp float new_depth = texture2D(depthImg, texCoo).r + (texture2D(depthImg, texCoo).g + texture2D(depthImg, texCoo).b/255.0)/255.0;
#ifndef USE_FRAG_DEPTH_EXT
        if (splatSetIndex==-1) // If this is "the most recent proxy model", it should be brought some amount toward the front
            new_depth = clamp(new_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
#endif
        
        //        highp float
        new_depth = intra_splat_depth;
        // gl_FragColor = vec4( 10.0*abs(new_depth-1.0) , 0.0, 0.0, src_alpha ); return; // ok
        // This will not always catch those regions of splats that are outside the hull of the scene. Finding these regions is the purpose of the following.

        // Er dette helt doedfoedt?!

        // Note that it should not be necessary to recompute window coordinates, these should be available in gl_FragCoord!

        highp vec3 ndc = vec3(vertPos.xy + (gl_PointCoord-0.5)*2.0/vec2(splats_x, splats_y)*splatOverlap, 2.0*new_depth-1.0);
        // gl_FragColor = vec4(10.0*abs(ndc_to_fs-ndc), src_alpha); return; // Checked (Should be black in the center of the splats, at least)
        
        highp vec4 vert_eb = depthPMinv * vec4(ndc, 1.0);
        // gl_FragColor = vec4(( 10.0*abs(vert_eb-vert_eb_to_fs) ).xyz, src_alpha); return; // Checked
        
        highp vec4 vert_wb = depthMVinv * vert_eb;
        // gl_FragColor = vec4(( 10.0*abs(vert_wb-vert_wb_to_fs) ).xyz, src_alpha); return; // Checked
        
        highp vec4 pos = PM * MV * vert_wb;
        // gl_FragColor = vec4(( 10.0*abs(pos-glpos) ).xyz, src_alpha); return; // Ok
        // gl_FragColor = vec4( 10.0*length((pos-glpos).xyz), 0.0, 0.0, src_alpha); return; // Ok
        
        // highp vec2 scr_coo   = pos.xy   * vec2(vp_width, vp_height) / 2.0 / pos.w;
        // highp vec2 glscr_coo = glpos.xy * vec2(vp_width, vp_height) / 2.0 / glpos.w;
        // gl_FragColor = vec4( 0.1*length((scr_coo-glscr_coo).xy), 0.0, 0.0, src_alpha); return; // Ok, not a black spot in the middle due to no division with .w?

        // Best we can do without the proper texture-coo transformation
        if (false) {
            highp vec2 tc2 = texCoo;
            tc2 = tc2 + vec2(0.5/float(vp_width), 0.5/float(vp_height)); // Must we add this to get sampling mid-texel?!
            tc2 = tc2 + c/vec2(splats_x, splats_y)*splatOverlap;
            // intra_splat_depth = texture2D(depthImg, tc).r + (texture2D(depthImg, tc).g + texture2D(depthImg, tc).b/255.0)/255.0;
            gl_FragColor = vec4( decay * texture2D( rgbImage, tc2 ).xyz, src_alpha );
            return;
        }
        
        


        // Since the relation between vertex and texture coordinate is: s = 0.5*(x+1), and s_i = s+c.x/splats_x*overlap, we get
        // s_i = 0.5*(x_i + 1) <=> x_i = 2*s_i - 1 = 2s + 2c.x/splats_x*overlap - 1.
        // Then, x_i - x = 2s + 2c.x/splats_x*overlap - 1 - (2s-1) = 2c.x/splats_x*overlap. (*)
        
        highp vec3 ndc_is = vec3( vertPos.x + 2.0/splats_x*splatOverlap * c.x, // c.x=0.5 & overlap=1 => x + 1/splats. Seems reasonable.
                                  vertPos.y + 2.0/splats_y*splatOverlap * c.y,
                                  2.0*intra_splat_depth - 1.0 );
        
        if (false) {
            highp float d = length(ndc_is-ndc);
            // Should be zero in the center of splats, but what about the rim? How do we normalize this length?  If we
            // let y_ndc_i = y_ndc and z_ndc_i = z_ndc, we see that the distance should be 1.0/splats_x*splatOverlap,
            // cf. the comment (*) above.
            d = d / ( 1.0/splats_x*splatOverlap );
            gl_FragColor = vec4(d, 0.0, 0.0, src_alpha); // Seems ok
        }

        highp vec4 vert_eb_is = depthPMinv * vec4(ndc_is, 1.0);
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




    if (fragDepthTest>0) {

        //----------------------------------------------------------------------------------------------------
        // 
        // Reconstructing vertex from intra-splat coordinates to obtain proper depth.
        //
        //----------------------------------------------------------------------------------------------------

	// Verifies that we understand in the vs how depth is computed
        // gl_FragColor = vec4(0.0, 0.0, abs(gl_FragCoord.z-depth)*100000.0, src_alpha);

        // We have a more or less ok depth value in planar_depth, this should be the appropriate depth for the
        // "depth-adjusted" billboard. We try to turn this into a depth fragment value.

        // To obtain the fragment's z-value, we need the ndc for the point. An alternative is to bilinearly interpolate,
        // with a 2x2 matrix from the VS, like we already do for 'planar_depth'. Trying this first. (What will be the
        // relation between planar_depth and the new planar_frag_depth? Are they the same? A linear relation given by
        // the near and far params?)

        highp float planar_frag_depth = frag_depth + dot( frag_depth_e, intraSplatTexCooTransform2*vec2(c.x, -c.y) );
        
        // Testing. Should give black in center. Then the interpolated z-frag is correct there. Should then be red
        // farther away, and green closer.
        // gl_FragColor = vec4(500.0*clamp(planar_frag_depth-depth, 0.0, 1.0), 500.0*clamp(depth-planar_frag_depth, 0.0, 1.0), 0.0, src_alpha); return;
        
        
#ifdef USE_FRAG_DEPTH_EXT
        if (splatSetIndex==-1) {
            planar_frag_depth = clamp(planar_frag_depth - 0.001, 0.0, 1.0);
        }
#endif
        
        gl_FragDepthEXT = planar_frag_depth;
        
    }
    
    
}




// Hmm. Ville det vaert lurere aa gjoere alle "de-projeksjonene" når dybde-buffer ble mottatt fra server? Ville da
// slippe aa gjoere det for hvert renderkall. Ville ogsaa slippe aa lagre teksturene. Maatte i steden lagre hele
// splat-settet.
