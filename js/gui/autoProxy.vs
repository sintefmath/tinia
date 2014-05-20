attribute vec2 aVertexPosition;

varying highp vec2 vTextureCoord; // Implicitly taken to be *output*?!

uniform mat4 PM;
uniform mat4 MV;
uniform mat4 depthPMinv;
uniform mat4 depthMVinv;
uniform sampler2D uSampler;
uniform float splatSize;

uniform int variableSized;

varying highp float depth;



void main(void)
{
    vec2 st = 0.5*(aVertexPosition.xy+1.0); // From [-1, 1] to [0, 1]
    st.y=1.0-st.y; 
    vTextureCoord = st;

    depth = ( texture2D( uSampler, st ).r +
              texture2D( uSampler, st ).g / 255.0 +
              texture2D( uSampler, st ).b / (255.0*255.0) );
    
    if ( depth > 0.9999 ) {
        // The depth should be 1 for fragments not rendered. It may be a problem that depth input is 'varying'.
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
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
    gl_Position = PM * MV * vert_wb;
    vec4 pos = PM * MV * vert_wb;

    if ( variableSized > 0 ) {

	vec2 st_dx = 0.5*( vec2(aVertexPosition.x+2.0/16.0, aVertexPosition.y) + 1.0 );
	vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+2.0/16.0) + 1.0 );
	st_dx.y = 1.0-st_dx.y; 
	st_dy.y = 1.0-st_dy.y; 

	// 16-bit version. Remember to fix in depth-reading code also, if changed.
// 	float depth_dx = ( texture2D( uSampler, st_dx ).r +
// 			   texture2D( uSampler, st_dx ).g / 255.0 );
// 	float depth_dy = ( texture2D( uSampler, st_dy ).r +
// 			   texture2D( uSampler, st_dy ).g / 255.0 );
        // 24-bit version. Remember to fix in depth-reading code also, if changed.
        float depth_dx = ( texture2D( uSampler, st_dx ).r +
                           texture2D( uSampler, st_dx ).g / 255.0 +
                           texture2D( uSampler, st_dx ).b / (255.0*255.0) );
        float depth_dy = ( texture2D( uSampler, st_dy ).r +
                           texture2D( uSampler, st_dy ).g / 255.0 +
                           texture2D( uSampler, st_dy ).b / (255.0*255.0) );
	
	depth_dx = depth;
	depth_dy = depth;
	
	// We may think of the depth texture as a grid of screen space points together with depths, which we will
	// subsample in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
	float x_ndc_dx = aVertexPosition.x + 2.0/16.0;
	float y_ndc_dx = aVertexPosition.y;
	float z_ndc_dx = 2.0*depth_dx - 1.0;
	float x_ndc_dy = aVertexPosition.x;
	float y_ndc_dy = aVertexPosition.y + 2.0/16.0;
	float z_ndc_dy = 2.0*depth_dy - 1.0;

	// We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
	vec4 vert_eb_dx = depthPMinv * vec4( x_ndc_dx, y_ndc_dx, z_ndc_dx, 1.0 );
	vec4 vert_eb_dy = depthPMinv * vec4( x_ndc_dy, y_ndc_dy, z_ndc_dy, 1.0 );

	vec4 vert_wb_dx = depthMVinv * vert_eb_dx;
	vec4 vert_wb_dy = depthMVinv * vert_eb_dy;
	
	// Next, we apply the current transformation to get the proxy splat.
	vec4 pos_dx = PM * MV * vert_wb_dx;
	vec4 pos_dy = PM * MV * vert_wb_dy;

	vec2 scr_coo    = pos.xy    * vec2(512, 512) / 2.0 / pos.w;
	vec2 scr_coo_dx = pos_dx.xy * vec2(512, 512) / 2.0 / pos_dx.w;
	vec2 scr_coo_dy = pos_dy.xy * vec2(512, 512) / 2.0 / pos_dy.w;

	float ss = 1.0*max( length(scr_coo_dx-scr_coo), length(scr_coo_dy-scr_coo) );

        gl_PointSize = max( ss, 5.0);
    } else {
        gl_PointSize = splatSize;
    }

}
