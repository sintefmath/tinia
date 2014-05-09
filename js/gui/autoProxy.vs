attribute vec2 aVertexPosition;

varying highp vec2 vTextureCoord; // Implicitly taken to be *output*?!

uniform mat4 PM;
uniform mat4 MV;
uniform mat4 depthPMinv;
uniform mat4 depthMVinv;
uniform sampler2D uSampler;
uniform float splatSize;

varying highp float depth;



void main(void)
{
    vec2 st = 0.5*(aVertexPosition.xy+1.0);
    st.y=1.0-st.y; 
    vTextureCoord = st;
    gl_PointSize = splatSize;

    // 8-bit version. Remember to fix in depth-reading code also, if changed.
//     depth = texture2D( uSampler, st ).r;

    // 16-bit version. Remember to fix in depth-reading code also, if changed.
    depth = ( texture2D( uSampler, st ).r +
              texture2D( uSampler, st ).g / 255.0 );
    
    // 24-bit version. Remember to fix in depth-reading code also, if changed.
//     depth = ( texture2D( uSampler, st ).r +
//               texture2D( uSampler, st ).g / 255.0 +
//               texture2D( uSampler, st ).b / (255.0*255.0) );

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
}
