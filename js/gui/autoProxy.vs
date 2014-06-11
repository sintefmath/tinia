#define USE_FRAG_DEPTH_EXT

attribute vec2 aVertexPosition;

varying highp vec2 texCoo;                      // Implicitly taken to be *output*?!
varying highp float depth;                      // Should be equal to gl_FragCoord.z in the FS, so should not be necessary to pass on. Doing it now for debug purposes
varying highp float sampled_depth;              // Actually sampled depth from the texture. Should also not be needed in the FS, probably.
varying highp float sampled_depth_dx; // debugging
varying highp float sampled_depth_dy; // debugging
varying highp vec2 depth_e;                     // For approximating the intra-splat depth, depth = sampled_depth + depth_e' * c

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

uniform mat4 PM;
uniform mat4 MV;
uniform mat4 depthPMinv;
uniform mat4 depthMVinv;

uniform sampler2D depthImg;
uniform float splatOverlap;

uniform int screenSpaceSized;
uniform float splats_x;
uniform float splats_y;
uniform int vp_width;
uniform int vp_height;
uniform int splatSetIndex;
uniform int mostRecentOffset;
uniform int splatSizeLimiting;
//uniform int testFlag;
//uniform int differentiationTestFlag;
uniform int fragDepthTest;
uniform int tcConst;




mat2 invrs(mat2 tmp)
{
    float det_inv = 1.0/(tmp[0][0]*tmp[1][1] - tmp[1][0]*tmp[0][1]);              // det_inv = 1/(ad-bc)
    return det_inv * mat2( tmp[1][1], -tmp[0][1], -tmp[1][0], tmp[0][0] );        // det_inv * [d, -b; -c, a]
}




void main(void)
{
    vec2 st = 0.5*(aVertexPosition.xy+1.0); // From [-1, 1] to [0, 1]
    st.y = 1.0-st.y;
    texCoo = st;
    vertPos = aVertexPosition; // Declared as varying, but will be constant all over the POINT primitive.

    // With a 1024^2 canvas and 512^2 splats, there are no artifacts to be seen from using 16 bits for the depth.
    // (But 8 is clearly too coarse.)
    // Now using all 24 bits, since we do send them from the server, currently.
    st = st + vec2(0.5/float(vp_width), 0.5/float(vp_height)); // Must we add this to get sampling mid-texel?!
    sampled_depth = texture2D(depthImg, st).r + (texture2D(depthImg, st).g + texture2D(depthImg, st).b/255.0) / 255.0;

    if ( sampled_depth > 0.999 ) {
        // The depth should be 1 for fragments not rendered. Discarding the whole splat.
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
    }
    
#ifndef USE_FRAG_DEPTH_EXT
    if (splatSetIndex==-1) {
        // Moving the most recent proxy model forward.
        // The problem with this is that splats are large, and "coplanar with the viewport", so that the amount of
        // shifting must be rather large to produce the desired effect on regions for which the gemetry is not "coplanar
        // with the viewport"...
        // (Would it be feasible to adjust all fragments intra-splat for more accurate depths? This could perhaps solve the problem.)
        // (Unfortunately, WebGL doesn't support gl_FragDepth.)
        sampled_depth = clamp(sampled_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
    }
#endif

    // We may think of the depth texture as a grid of screen space points together with depths, which we will subsample
    // in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
    vec3 ndc = vec3( aVertexPosition.xy, 2.0*sampled_depth - 1.0 );
    ndc_to_fs = ndc;

    // We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
    vec4 vert_eb = depthPMinv * vec4( ndc, 1.0 );
    vert_eb_to_fs = vert_eb;
    vec4 vert_wb = depthMVinv * vert_eb;
    vert_wb_to_fs = vert_wb;

    // Next, we apply the current transformation to get the proxy splat. (This is the vertex in clip coordinates.)
    vec4 pos = PM * MV * vert_wb;
    gl_Position = pos;
    glpos = pos;

    float z_ndc = pos.z/pos.w;
    depth = 0.5*( gl_DepthRange.diff*z_ndc + gl_DepthRange.near + gl_DepthRange.far ); // z_window
    // This is not the depth of the point on the original geometry, but the new depth for the splat transformed into
    // place.  (This value is equal to gl_FragCoord.z in the fragment shader. Note that this is constant over the
    // primitive, unless we modify it in the fragment shader. Doing this requires the GL_EXT_frag_depth extension
    // currently. (May 2014))
    // 140611: Don't think this should be computed or used anywhere. Useful for debugging/testing. 

    vec2 splatSizeVec = vec2( float(vp_width)/float(splats_x), float(vp_height)/float(splats_y) );
    float splatSize = max( splatSizeVec.x, splatSizeVec.y );

    // Finally, setting the pointSize:
    if (screenSpaceSized>0) {
        // We sample the depths for next splat in x-direction (*_dx) and y-direction (*_dy), and will use the screen space
        // difference between splats to determine appropriate (screen space) splat sizes.
        // This doesn't work as it should, but I can't see the bug here... We get fine results just reusing the same depth, though.
        // Hmm. Is it the "length(scr_coo_dx-scr_coo)" instead of "abs(scr_coo_dx.x-scr_coo.x)" etc. that is the cause?
        // Is the problem maybe that very large splats simply look very "wrong"?

	// dette er egentlig numerisk derivasjon, og det er ikke noedvendig aa gaa saa langt ut. Om vi gaar kortere ut
	// blir det mindre sannsynlig av vi treffer noe med dybde ikke satt, dvs. dybde==1...!

        float delta = 0.1;
        // The value 1.0 means that we go to the "next" splat in each direction.
        // We should probably make this, i.e., the amount added to aVertexPosition below, independent of the number of splats.
	
        vec2 st_dx = 0.5*( vec2(aVertexPosition.x+delta*2.0/splats_x, aVertexPosition.y) + 1.0 );
        vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+delta*2.0/splats_y) + 1.0 );
        st_dx.y = 1.0-st_dx.y; 
        st_dy.y = 1.0-st_dy.y; 
        float depth_dx = texture2D(depthImg, st_dx).r + (texture2D(depthImg, st_dx).g + texture2D(depthImg, st_dx).b/255.0) / 255.0;
        float depth_dy = texture2D(depthImg, st_dy).r + (texture2D(depthImg, st_dy).g + texture2D(depthImg, st_dy).b/255.0) / 255.0;
#ifndef USE_FRAG_DEPTH_EXT
        if (splatSetIndex==-1) {
            depth_dx = clamp(depth_dx - 0.001*float(mostRecentOffset), 0.0, 1.0);
            depth_dy = clamp(depth_dy - 0.001*float(mostRecentOffset), 0.0, 1.0);
        }
#endif
        // Note that we must have a way to avoid sampling the texture outside of any rendered depths, or, at least not use such values.
        if (depth_dx>0.999)
            depth_dx = sampled_depth;
        if (depth_dy>0.999) 
            depth_dy = sampled_depth;
	
        // We may think of the depth texture as a grid of screen space points together with depths, which we will
        // subsample in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
        vec3 ndc_dx = vec3( aVertexPosition.x + delta*2.0/splats_x, aVertexPosition.y, 2.0*depth_dx - 1.0 );
        vec3 ndc_dy = vec3( aVertexPosition.x, aVertexPosition.y + delta*2.0/splats_y, 2.0*depth_dy - 1.0 );
	
        // We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
        vec4 vert_eb_dx = depthPMinv * vec4( ndc_dx, 1.0 );
        vec4 vert_eb_dy = depthPMinv * vec4( ndc_dy, 1.0 );
        vec4 vert_wb_dx = depthMVinv * vert_eb_dx;
        vec4 vert_wb_dy = depthMVinv * vert_eb_dy;
	
        // Next, we apply the current transformation to get the proxy splat.
        vec4 pos_dx = PM * MV * vert_wb_dx;
        vec4 pos_dy = PM * MV * vert_wb_dy;
	
        vec2 scr_coo    = pos.xy    * vec2(vp_width, vp_height) / 2.0 / pos.w;
        vec2 scr_coo_dx = pos_dx.xy * vec2(vp_width, vp_height) / 2.0 / pos_dx.w;
        vec2 scr_coo_dy = pos_dy.xy * vec2(vp_width, vp_height) / 2.0 / pos_dy.w;

        // This ("ss") is to replace splatSize, both are measured in pixels, and describe a real splat's size.a
        // Which of these two to use?!
        float ss = (1.0/delta)*max( length(scr_coo_dx-scr_coo), length(scr_coo_dy-scr_coo) );
        //float ss = max( abs(scr_coo_dx.x-scr_coo.x), abs(scr_coo_dy.y-scr_coo.y) );
	
        // Transferring the scale factors to the FS for intra-splat texture coordinate adjustment
        // (The factors would be 1 for un-skewed splats, not taking splatOverlap into consideration here.)
        ssFactor = (1.0/delta)*vec2( (scr_coo_dx.x-scr_coo.x)/splatSizeVec.x, (scr_coo_dy.y-scr_coo.y)/splatSizeVec.y  );
	
        // Putting an upper limit on the size, equal to an expansion of 3 (no splat larger than 3 times dist between splats)
        if (splatSizeLimiting>0) {
            ss = min( ss, 3.0*splatSize );
            ssFactor = min( ssFactor, vec2(3.0) );
        }
        
        gl_PointSize = max( splatOverlap * ss, 1.0); // Minimum splat size will be exactly 1 pixel wide splats.
    } else {
        ssFactor = vec2(1.0); // Corresponding to just plain splats exactly covering the viewport if no transformation is done
        gl_PointSize = splatOverlap * splatSize;
    }

    //----------------------------------------------------------------------------------------------------
    //
    // Setting up intraSplatTexCooTransform for the fragment shader
    //
    //----------------------------------------------------------------------------------------------------

    float c = float(tcConst)/100.0; // [0, 10], default = 100/100 = 1.0 (for testing and debugging)
    
    float delta = float(splats_x) / float(vp_width);
    // delta = 1.0;
    
    // A value of 1.0 will cause the "next splat" to be used for the subsequent computations. If the number of splats is
    // smaller than the depth buffer size, this means that we will skip depth values in the computations. This also
    // means that for the cube test, all splats on the edges will be similarly "skewed". This will not necessarily be
    // the case if we choose a smaller delta.
    //
    // To get the "next texture sample", we should have st_dx.x - st.x = 0.5 * delta*2.0/splats_x = 1/vp_width <=>
    // delta/splats_x = 1/vp_width <=> delta = splats_x/vp_width. In the cube case, it means that most splats on the
    // edges will turn out ok, if the number of splats is smaller than the number of depth samples. Note that some of
    // these splats will come out quite wrong anyway. Still, this should be the best value for delta.
    
    vec2 st_dx = 0.5*( vec2(aVertexPosition.x+delta*2.0/splats_x, aVertexPosition.y) + 1.0 );
    vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+delta*2.0/splats_y) + 1.0 );
    st_dx.y = 1.0-st_dx.y;
    st_dy.y = 1.0-st_dy.y; 
    st_dx = st_dx + vec2(0.5/float(vp_width), 0.5/float(vp_height)); // Must we add this to get sampling mid-texel?!
    st_dy = st_dy + vec2(0.5/float(vp_width), 0.5/float(vp_height)); // Must we add this to get sampling mid-texel?!
    float depth_dx = texture2D(depthImg, st_dx).r + (texture2D(depthImg, st_dx).g + texture2D(depthImg, st_dx).b/255.0) / 255.0;
    float depth_dy = texture2D(depthImg, st_dy).r + (texture2D(depthImg, st_dy).g + texture2D(depthImg, st_dy).b/255.0) / 255.0;
    if (depth_dx>0.999) {
        // Possible actions: Use the 'sampled_depth' value to get something more sensible. The primitive will be drawn,
        // but is it useful to draw an oddly textured splat? Another alternative is to discard the whole splat.
	//depth_dx = sampled_depth;
 	gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
 	return;
    }
    if (depth_dy>0.999) {
	//depth_dy = sampled_depth;
 	gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
 	return;
    }
#ifndef USE_FRAG_DEPTH_EXT
    if (splatSetIndex==-1) {
        depth_dx = clamp(depth_dx - 0.001*float(mostRecentOffset), 0.0, 1.0);
        depth_dy = clamp(depth_dy - 0.001*float(mostRecentOffset), 0.0, 1.0);
    }
#endif
    
    vec3 ndc_dx     = vec3( aVertexPosition.x + delta*2.0/splats_x, aVertexPosition.y, 2.0*depth_dx - 1.0 );
    vec3 ndc_dy     = vec3( aVertexPosition.x, aVertexPosition.y + delta*2.0/splats_y, 2.0*depth_dy - 1.0 );
    vec4 vert_eb_dx = depthPMinv * vec4( ndc_dx, 1.0 ); // eye before
    vec4 vert_eb_dy = depthPMinv * vec4( ndc_dy, 1.0 );
    vec4 vert_wb_dx = depthMVinv * vert_eb_dx; // world before
    vec4 vert_wb_dy = depthMVinv * vert_eb_dy;
    vec4 pos_dx     = PM * MV * vert_wb_dx;
    vec4 pos_dy     = PM * MV * vert_wb_dy;
    
    vec2 scr_coo    = pos.xy    * vec2(vp_width, vp_height) / 2.0 / pos.w;    // Splat_00
    vec2 scr_coo_dx = pos_dx.xy * vec2(vp_width, vp_height) / 2.0 / pos_dx.w; // Splat_10
    vec2 scr_coo_dy = pos_dy.xy * vec2(vp_width, vp_height) / 2.0 / pos_dy.w; // Splat_01
    
    // Difference of screen coordinates, in pixels
    vec2 scr_dx = (1.0/delta)*(scr_coo_dx-scr_coo);
    vec2 scr_dy = (1.0/delta)*(scr_coo_dy-scr_coo);
    
    // Difference of texture coordinates for adjacent splats, measured in texels.
    vec2 st_e1 = (1.0/delta)*(st_dx - st);
    vec2 st_e2 = (1.0/delta)*(st_dy - st);
    
    // For interpolating the fragment's z-value in the FS
    frag_depth = depth;
    float frag_depth_dx = pos_dx.z/pos_dx.w; // = ndc_dx.z?!
    frag_depth_dx = 0.5*( gl_DepthRange.diff*frag_depth_dx + gl_DepthRange.near + gl_DepthRange.far );
    float frag_depth_dy = pos_dy.z/pos_dy.w; // = ndc_dy.z?!
    frag_depth_dy = 0.5*( gl_DepthRange.diff*frag_depth_dy + gl_DepthRange.near + gl_DepthRange.far );
    frag_depth_e = c * (1.0/delta)*vec2(frag_depth_dx-frag_depth, frag_depth_dy-frag_depth);

    



    // Simplifying...
    
    mat4 A = PM * MV * depthMVinv * depthPMinv;
    vec4 Au = A * vec4( aVertexPosition.xy, 2.0*sampled_depth - 1.0, 1.0 );
    vec4 Adu = A * vec4( aVertexPosition.xy + vec2(delta*2.0/splats_x, 0.0), 2.0*depth_dx - 1.0, 1.0 );
    vec4 Adv = A * vec4( aVertexPosition.xy + vec2(0.0, delta*2.0/splats_y), 2.0*depth_dy - 1.0, 1.0 );
    scr_dx = float(vp_width )/(2.0*delta) * ( Adu.xy/Adu.w - Au.xy/Au.w );
    scr_dy = float(vp_height)/(2.0*delta) * ( Adv.xy/Adv.w - Au.xy/Au.w );
    st_e1 = vec2( 1.0/splats_x, 0.0 );
    st_e2 = vec2( 0.0, -1.0/splats_y );

    depth_e = (1.0/delta)*vec2(depth_dx-sampled_depth, depth_dy-sampled_depth);

    sampled_depth_dx = depth_dx;
    sampled_depth_dy = depth_dy;

    // Discarding splats that are not front-facing
    // Modifying the test to remove some more of the border-line cases. How can we do this more precisely?
    vec3 dx = normalize( vec3(scr_dx, 0.0) );
    vec3 dy = normalize( vec3(scr_dy, 0.0) );
    if ( cross(dx, dy).z < 0.1 ) {
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
    }



    // The vectors 'st_e1' and 'st_e2' span the region in the textures with lower left corner 'st' (splat_00), to
    // which the screen space region with lower left corner 'scr_coo' and spanning vectors 'scr_coo_dx' and 'scr_coo_dy'
    // should be mapped.
    
    intraSplatTexCooTransform =
	mat2( st_e1, st_e2 ) *                                 // This term maps the screen region "between the splats", given
	                                                       // with coordinates in [0, 1]^2, to the corresponding texture region.

	invrs( mat2(scr_dx, scr_dy) ) *                        // These terms map gl_PointCoord-0.5 to the (scr_dx, scr_dy)-spanned
	mat2( splatSizeVec.x, 0.0, 0.0, splatSizeVec.y ) *     // screen region, producing a coordinate in [0, 1]^2 for points inside
	splatOverlap;                                          // this region.

    intraSplatTexCooTransform2 =
	invrs( mat2(scr_dx, scr_dy) ) *                        // These terms map gl_PointCoord-0.5 to the (scr_dx, scr_dy)-spanned
	mat2( splatSizeVec.x, 0.0, 0.0, splatSizeVec.y ) *     // screen region, producing a coordinate in [0, 1]^2 for points inside
	splatOverlap;                                          // this region.
}
