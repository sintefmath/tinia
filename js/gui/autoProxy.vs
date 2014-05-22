attribute vec2 aVertexPosition;

varying highp vec2 vTextureCoord; // Implicitly taken to be *output*?!

uniform mat4 PM;
uniform mat4 MV;
uniform mat4 depthPMinv;
uniform mat4 depthMVinv;
uniform sampler2D uSampler;
uniform float splatSize;
uniform float splatOverlap;

uniform int variableSized;
uniform float splats_x;
uniform float splats_y;
uniform int vp_width;
uniform int vp_height;
uniform int splatSetIndex2;
uniform int mostRecentOffset;
uniform int splatSizeLimiting;

varying highp float depth;



void main(void)
{
    vec2 st = 0.5*(aVertexPosition.xy+1.0); // From [-1, 1] to [0, 1]
    st.y=1.0-st.y; 
    vTextureCoord = st;

    // With a 1024^2 canvas and 512^2 splats, there are no artifacts to be seen from using 16 bits for the depth. (But 8 is crap.)
    // Now using all 24 bits, since we do send them from the server, currently.
    depth = ( texture2D( uSampler, st ).r +
              texture2D( uSampler, st ).g / 255.0 +
              texture2D( uSampler, st ).b / (255.0*255.0) );
    
    if ( depth > 0.9999 ) {
        // The depth should be 1 for fragments not rendered. Discarding the whole splat.
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
    }
    
    if ( (splatSetIndex2==-1) && (mostRecentOffset>0) ) {
        // Moving the most recent proxy model forward.
        // The problem with this is that splats are large, and "coplanar with the viewport", so that the amount of shifting must be rather large to 
        // produce the desired effect on regions for which the gemetry is not "coplanar with the newport"...
        // (Would it be feasible to adjust all fragments intra-splat for more accurate depths? This could perhaps solve the problem.)
        // (Unfortunately, WebGL doesn't support gl_FragDepth.)
	depth = depth - 1.0/32767.0; // We probably have 24 bits of depth...
    }

    // We may think of the depth texture as a grid of screen space points together with depths, which we will subsample
    // in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
    float x_ndc = aVertexPosition.x;
    float y_ndc = aVertexPosition.y;
    float z_ndc = 2.0*depth - 1.0;

    // We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
    vec4 vert_eb = depthPMinv * vec4( x_ndc, y_ndc, z_ndc, 1.0 );
    vec4 vert_wb = depthMVinv * vert_eb;
    
    // Next, we apply the current transformation to get the proxy splat.
    vec4 pos = PM * MV * vert_wb;
    gl_Position = pos;

    if ( variableSized > 0 ) {
#if 0
        // We sample the depths for next splat in x-direction (*_dx) and y-direction (*_dy), and will use the screen space
        // difference between splats to determine appropriate (screen space) splat sizes.
        // This doesn't work as it should, but I can't see the bug here... We get fine results just reusing the same depth, though.
        // Hmm. Is it the "length(scr_coo_dx-scr_coo)" instead of "abs(scr_coo_dx.x-scr_coo.x)" etc. that is the cause?
        // Is the problem maybe that very large splats simply look very "wrong"?
	vec2 st_dx = 0.5*( vec2(aVertexPosition.x+2.0/splats_x, aVertexPosition.y) + 1.0 );
	vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+2.0/splats_y) + 1.0 );
	st_dx.y = 1.0-st_dx.y; 
	st_dy.y = 1.0-st_dy.y; 
        float depth_dx = ( texture2D( uSampler, st_dx ).r +
                           texture2D( uSampler, st_dx ).g / 255.0 +
                           texture2D( uSampler, st_dx ).b / (255.0*255.0) );
        float depth_dy = ( texture2D( uSampler, st_dy ).r +
                           texture2D( uSampler, st_dy ).g / 255.0 +
                           texture2D( uSampler, st_dy ).b / (255.0*255.0) );
        // Note that we must have a way to avoid sampling the texture outside of any rendered depths, or, at least not use such values.
        if (depth_dx>0.9999)
            depth_dx = depth;
        if (depth_dy>0.9999)
            depth_dy = depth;
#else
        float depth_dx = depth;
        float depth_dy = depth;
#endif
	
	// We may think of the depth texture as a grid of screen space points together with depths, which we will
	// subsample in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
	float x_ndc_dx = aVertexPosition.x + 2.0/splats_x;
	float y_ndc_dx = aVertexPosition.y;
	float z_ndc_dx = 2.0*depth_dx - 1.0;
	float x_ndc_dy = aVertexPosition.x;
	float y_ndc_dy = aVertexPosition.y + 2.0/splats_y;
	float z_ndc_dy = 2.0*depth_dy - 1.0;

	// We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
	vec4 vert_eb_dx = depthPMinv * vec4( x_ndc_dx, y_ndc_dx, z_ndc_dx, 1.0 );
	vec4 vert_eb_dy = depthPMinv * vec4( x_ndc_dy, y_ndc_dy, z_ndc_dy, 1.0 );

	vec4 vert_wb_dx = depthMVinv * vert_eb_dx;
	vec4 vert_wb_dy = depthMVinv * vert_eb_dy;
	
	// Next, we apply the current transformation to get the proxy splat.
	vec4 pos_dx = PM * MV * vert_wb_dx;
	vec4 pos_dy = PM * MV * vert_wb_dy;

	vec2 scr_coo    = pos.xy    * vec2(vp_width, vp_height) / 2.0 / pos.w;
	vec2 scr_coo_dx = pos_dx.xy * vec2(vp_width, vp_height) / 2.0 / pos_dx.w;
	vec2 scr_coo_dy = pos_dy.xy * vec2(vp_width, vp_height) / 2.0 / pos_dy.w;

        float ss = max( length(scr_coo_dx-scr_coo), length(scr_coo_dy-scr_coo) );
	//float ss = max( abs(scr_coo_dx.x-scr_coo.x), abs(scr_coo_dy.y-scr_coo.y) );

        // Putting an upper limit on the size, equal to an expansion of 3 (no splat larger than 3 times dist between splats)
        if ( splatSizeLimiting > 0 ) {
            ss = min( ss, 3.0*max(float(vp_width)/splats_x, float(vp_height)/splats_y) );
        }
        
        gl_PointSize = max( splatOverlap * ss, 1.0);
    } else {
        gl_PointSize = splatSize;
    }

}
