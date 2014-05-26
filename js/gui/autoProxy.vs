attribute vec2 aVertexPosition;

varying highp vec2 texCoo; // Implicitly taken to be *output*?!
varying highp float depth;
varying highp vec2 vertPos;
varying highp vec4 glpos; // debugging
varying highp vec3 ndc_to_fs; // debugging
varying highp vec4 vert_wb_to_fs; // debugging
varying highp vec4 vert_eb_to_fs; // debugging
varying highp vec2 ssFactor;

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




void main(void)
{
    vec2 st = 0.5*(aVertexPosition.xy+1.0); // From [-1, 1] to [0, 1]
    st.y=1.0-st.y; 
    texCoo = vec2(st);
    vertPos = aVertexPosition; // Declared as varying, but will be constant all over the POINT primitive.

    // With a 1024^2 canvas and 512^2 splats, there are no artifacts to be seen from using 16 bits for the depth.
    // (But 8 is clearly too coarse.)
    // Now using all 24 bits, since we do send them from the server, currently.
    float sampled_depth = texture2D(depthImg, st).r + (texture2D(depthImg, st).g + texture2D(depthImg, st).b/255.0) / 255.0;

    if ( sampled_depth > 0.9999 ) {
        // The depth should be 1 for fragments not rendered. Discarding the whole splat.
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
    }
    
    if (splatSetIndex==-1) {
        // Moving the most recent proxy model forward.
        // The problem with this is that splats are large, and "coplanar with the viewport", so that the amount of
        // shifting must be rather large to produce the desired effect on regions for which the gemetry is not "coplanar
        // with the viewport"...
        // (Would it be feasible to adjust all fragments intra-splat for more accurate depths? This could perhaps solve the problem.)
        // (Unfortunately, WebGL doesn't support gl_FragDepth.)
	sampled_depth = clamp(sampled_depth - 0.001*float(mostRecentOffset), 0.0, 1.0);
    }

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
	
        vec2 st_dx = 0.5*( vec2(aVertexPosition.x+delta*2.0/splats_x, aVertexPosition.y) + 1.0 );
        vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+delta*2.0/splats_y) + 1.0 );
        st_dx.y = 1.0-st_dx.y; 
        st_dy.y = 1.0-st_dy.y; 
        float depth_dx = texture2D(depthImg, st_dx).r + (texture2D(depthImg, st_dx).g + texture2D(depthImg, st_dx).b/255.0) / 255.0;
        float depth_dy = texture2D(depthImg, st_dy).r + (texture2D(depthImg, st_dy).g + texture2D(depthImg, st_dy).b/255.0) / 255.0;
        // Note that we must have a way to avoid sampling the texture outside of any rendered depths, or, at least not use such values.
        if (depth_dx>0.9999) depth_dx = sampled_depth;
        if (depth_dy>0.9999) depth_dy = sampled_depth;
	
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
            ssFactor = min( ssFactor, vec2(3.0) ); // Hope this is per-element
        }
        
        gl_PointSize = max( splatOverlap * ss, 1.0); // Minimum splat size will be exactly 1 pixel wide splats.
    } else {
        ssFactor = vec2(1.0); // Corresponding to just plain splats exactly covering the viewport if no transformation is done
        gl_PointSize = splatOverlap * splatSize;
    }

}
