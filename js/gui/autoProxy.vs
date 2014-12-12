// #define VS_DISCARD_DEBUG
// NB! Note that coloring the primitives instead of discarding them, may cause otherwise good primitives to be shadowed
// by bad ones, when the define above is enabled. To counter this, consider disabling all but one proxy model in the
// FS. (See that start of the FS main() function.)

// #define CULL_BACK_SIDES              // We use pos(i, j+1)-pos(i, j) x pos(i+1, j)-pos(i, j) to determine culling
#define CULL_SKEWED_SPLATS              // Whether or not to test on the texture coordinate transform at all
//#define BLOB_INSTEAD_OF_SKEWED_SPLAT    // If the intra-splat texture coordinate transform is skewed, we use uniform coloring of the splat,
                                        // otherwise, the splat is discarded.

attribute vec2 aVertexPosition;

varying highp vec2 texCoo;                      // Implicitly taken to be *output*?!
varying highp float sampled_depth;              // Actually sampled depth from the texture. Should also not be needed in the FS, probably.
varying highp vec2 depth_e;                     // For approximating the intra-splat depth, depth = sampled_depth + depth_e' * c

varying highp float frag_depth;                 // Fragment depth for the vertex, i.e., the center of the splat
varying highp vec2 frag_depth_e;

varying highp mat2 intraSplatTexCooTransform;
varying highp mat2 intraSplatTexCooTransform2;

uniform highp mat4 projUnproj; // PM * MV * depthMVinv * depthPMinv

uniform sampler2D depthImg;
// 141129: textureSize(sampler, lod) is not available in GLSL ES 1.0 (WebGL) so we use DEPTH_WIDTH and DEPTH_HEIGHT added in ProxyRenderer.js.

uniform float splatOverlap;                     // Makes little sense in having this much larger than one if we use screenSpaceSized splats. 
                                                // We need 2 for coverage at all times, for consider this case:
                                                //
                                                //         +---+   +---+
                                                //         | * |   | * |
                                                //         +---+---+---+
                                                //             | * |  
                                                //             +---+  ...       (* denotes splat centers)
                                                //
varying highp float actualSplatOverlap;         // Only used for debugging purposes in the FS

#define PI 3.1415926535

#ifdef DEBUG
uniform int screenSpaceSized;
varying highp float splat_i, splat_j;
varying highp vec4 debugCol;                    // For replacing discarded primitives with an identifying color.
#endif

uniform float splats_x;
uniform float splats_y;
uniform int vp_width;
uniform int vp_height;
uniform int splatSetIndex;
#ifdef USE_FRAG_DEPTH_EXT
const float mostRecentProxyModelOffset = 0.05; // 140825: Needed to increase from 0.007 to 0.05 for apc_job to render properly with default settings.
#else
const float mostRecentProxyModelOffset = 0.001;
#endif




// Is it really the case that WebGL doesn't have this?!
mat2 invrs(mat2 tmp)
{
    float det_inv = 1.0/(tmp[0][0]*tmp[1][1] - tmp[1][0]*tmp[0][1]);              // det_inv = 1/(ad-bc)
    return det_inv * mat2( tmp[1][1], -tmp[0][1], -tmp[1][0], tmp[0][0] );        // det_inv * [d, -b; -c, a]
}




void main(void)
{
#ifdef DEBUG
    splat_j = floor( (0.5*aVertexPosition.x+1.0)*splats_x ); // For debugging
    splat_i = floor( (0.5*aVertexPosition.y+1.0)*splats_y );
    debugCol = vec4(0.0, 0.0, 0.0, 0.0); // Using alpha=1 to signify "yes, escape with this fragment color" in the FS.
#endif
#ifdef VS_DISCARD_DEBUG
    // Setting these now, so that we can safely exit early from the VS.
    depth_e = vec2(0.0);
    frag_depth = 0.5;
    frag_depth_e = vec2(0.0);
    intraSplatTexCooTransform2 = mat2(1.0, 0.0, 0.0, 1.0);
    intraSplatTexCooTransform = intraSplatTexCooTransform2;
#endif
    
    vec2 st = 0.5*(aVertexPosition.xy+1.0); // From [-1, 1] to [0, 1]
    st.y = 1.0-st.y;
    texCoo = st;

    // With a 1024^2 canvas and 512^2 splats, there are no artifacts to be seen from using 16 bits for the depth.
    // (But 8 is clearly too coarse.)
    // Now using all 24 bits, since we do send them from the server, currently.
#ifdef MID_TEXEL_SAMPLING
    st = st + vec2(0.5/float(DEPTH_WIDTH), -0.5/float(DEPTH_HEIGHT)); // Must we add this to get sampling mid-texel?!
#endif
#ifdef MID_SPLAT_SAMPLING
    // Adding the amount of "a half delta", for delta=1, see below.
    st = st + vec2(0.5/splats_x, -0.5/splats_y);
#endif
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
        sampled_depth = clamp(sampled_depth - mostRecentProxyModelOffset, 0.0, 1.0);
    }
#endif

    // We may think of the depth texture as a grid of screen space points together with depths, which we will subsample
    // in order to get a sparser set of 'splats'.  First, we obtain ndc coordinates.
    // We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.
    // Next, we apply the current transformation to get the proxy splat. (This is the vertex in clip coordinates.)
    vec4 pos = projUnproj * vec4( aVertexPosition.xy, 2.0*sampled_depth - 1.0, 1.0 );
    gl_Position = pos;

    float z_ndc = pos.z/pos.w;
    //depth = 0.5*( gl_DepthRange.diff*z_ndc + gl_DepthRange.near + gl_DepthRange.far ); // z_window
    frag_depth = 0.5*( gl_DepthRange.diff*z_ndc + gl_DepthRange.near + gl_DepthRange.far ); // z_window
    // This is not the depth of the point on the original geometry, but the new depth for the splat transformed into
    // place.  (This value is equal to gl_FragCoord.z in the fragment shader. Note that this is constant over the
    // primitive, unless we modify it in the fragment shader. Doing this requires the GL_EXT_frag_depth extension
    // currently. (May 2014))
    // 140611: Don't think this should be computed or used anywhere. Useful for debugging/testing. 

    vec2 splatSizeVec = vec2( float(vp_width)/float(splats_x), float(vp_height)/float(splats_y) );
    float splatSize = max( splatSizeVec.x, splatSizeVec.y );

    //----------------------------------------------------------------------------------------------------
    //
    // Setting up intraSplatTexCooTransform for the fragment shader
    //
    //----------------------------------------------------------------------------------------------------

#ifdef SMALL_DELTA_SAMPLING
    // To get the "next texture sample", we should have st_dx.x - st.x = 0.5 * delta*2.0/splats_x = 1/DEPTH_WIDTH <=>
    // delta/splats_x = 1/DEPTH_WIDTH <=> delta = splats_x/DEPTH_WIDTH. In the cube case, it means that most splats on the
    // edges will turn out ok, if the number of splats is smaller than the number of depth samples. Note that some of
    // these splats will come out quite wrong anyway. Still, this should be the best value for delta.

    // (141128: For the cube corners, this should mean that one of the two faces turn out just fine, and the other bogus,
    // in most cases, i.e., cases where all three samples (pos, pos+dx, pos+dy) are on the same side of the corner. Right?)

    // 141128: If we use such neighbouring fragments, should we rather put them in the middle of the splat instead of the 
    //         corner?! If that is what we currently do, then... Think maybe "yes" to this...
    //         (MID_SPLAT_SAMPLING) Hmm... Doesn't seem like an improvement.

    // 141128: And if we want "small delta", i.e., neighbouring texels, why not go not to the first neighbour, but instead
    //         to the next, to make sure we don't accidentally sample the same spot twice, for slightly skewed splats?!
    //         (LARGER_DELTA_SAMPLING) Doesn't seem like an improvement. But indicates that depth buffer resolution can be 
    //         reduced.

#ifdef LARGER_DELTA_SAMPLING
    float delta = 2.0 * float(splats_x) / float(DEPTH_WIDTH);
#else
    float delta = float(splats_x) / float(DEPTH_WIDTH);
#endif

#else
    // A value of 1.0 will cause the "next splat" to be used for the subsequent computations. If the number of splats is
    // smaller than the depth buffer size, this means that we will skip depth values in the computations. This also
    // means that for the cube test, all splats on the edges will be similarly "skewed". This will not necessarily be
    // the case if we choose a smaller delta.

    // 141128: Should we use delta=0.5 instead? And some offset so that all three samples (pos, pos+dx, pos+dy) are inside
    //         the splat?

    float delta = 1.0;
#endif
    
    vec2 st_dx = 0.5*( vec2(aVertexPosition.x+delta*2.0/splats_x, aVertexPosition.y) + 1.0 );
    vec2 st_dy = 0.5*( vec2(aVertexPosition.x, aVertexPosition.y+delta*2.0/splats_y) + 1.0 );

    st_dx.y = 1.0-st_dx.y;
    st_dy.y = 1.0-st_dy.y; 
#ifdef MID_TEXEL_SAMPLING
    st_dx = st_dx + vec2(0.5/float(DEPTH_WIDTH), -0.5/float(DEPTH_HEIGHT)); // Must we add this to get sampling mid-texel?!
    st_dy = st_dy + vec2(0.5/float(DEPTH_WIDTH), -0.5/float(DEPTH_HEIGHT)); // Must we add this to get sampling mid-texel?!
#endif
#ifdef MID_SPLAT_SAMPLING
    st_dx = st_dx + vec2(0.5/splats_x, 0.0);
    st_dy = st_dy - vec2(0.0, 0.5/splats_y);
#endif
    float depth_dx = texture2D(depthImg, st_dx).r + (texture2D(depthImg, st_dx).g + texture2D(depthImg, st_dx).b/255.0) / 255.0;
    float depth_dy = texture2D(depthImg, st_dy).r + (texture2D(depthImg, st_dy).g + texture2D(depthImg, st_dy).b/255.0) / 255.0;
    if (depth_dx>0.999) {
        // Possible actions: Use the 'sampled_depth' value to get something more sensible. The primitive will be drawn,
        // but is it useful to draw an oddly textured splat? Another alternative is to discard the whole splat.
#ifdef VS_DISCARD_DEBUG
	debugCol = vec4(0.5, 1.0, 0.5, 1.0); // Not discarding, colouring the whole primitive light green instead
#else
 	gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
#endif
 	return;
    }
    if (depth_dy>0.999) {
#ifdef VS_DISCARD_DEBUG
        debugCol = vec4(0.5, 0.5, 1.0, 1.0); // Not discarding, colouring the whole fragment light blue instead
#else
 	gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
#endif
 	return;
    }
#ifndef USE_FRAG_DEPTH_EXT
    if (splatSetIndex==-1) {
        depth_dx = clamp(depth_dx - mostRecentProxyModelOffset, 0.0, 1.0);
        depth_dy = clamp(depth_dy - mostRecentProxyModelOffset, 0.0, 1.0);
    }
#endif
    
    vec4 pos_dx = projUnproj * vec4( aVertexPosition + vec2(delta*2.0/splats_x, 0.0), 2.0*depth_dx - 1.0, 1.0 );
    vec4 pos_dy = projUnproj * vec4( aVertexPosition + vec2(0.0, delta*2.0/splats_y), 2.0*depth_dy - 1.0, 1.0 );
    
    // For interpolating the fragment's z-value in the FS
    float frag_depth_dx = pos_dx.z/pos_dx.w;
    frag_depth_dx = 0.5*( gl_DepthRange.diff*frag_depth_dx + gl_DepthRange.near + gl_DepthRange.far );
    float frag_depth_dy = pos_dy.z/pos_dy.w;
    frag_depth_dy = 0.5*( gl_DepthRange.diff*frag_depth_dy + gl_DepthRange.near + gl_DepthRange.far );
    frag_depth_e = (1.0/delta)*vec2(frag_depth_dx-frag_depth, frag_depth_dy-frag_depth);
    
    // Difference of screen coordinates, in pixels:
    vec2 scr_dx = float(vp_width )/(2.0*delta) * ( pos_dx.xy/pos_dx.w - pos.xy/pos.w ); // @@@ vp-size or DEPTH-size?!
    vec2 scr_dy = float(vp_height)/(2.0*delta) * ( pos_dy.xy/pos_dy.w - pos.xy/pos.w ); // @@@ vp-size or DEPTH-size?!
    // Difference of texture coordinates for adjacent splats, measured in texels:
    vec2 st_e1 = vec2( 1.0/splats_x, 0.0 );
    vec2 st_e2 = vec2( 0.0, -1.0/splats_y );

    depth_e = (1.0/delta)*vec2(depth_dx-sampled_depth, depth_dy-sampled_depth);

    // This will discard all splats not facing forward
#ifdef CULL_BACK_SIDES
    // This does not work as originally intended. For unwanted splats (or, rather, splats with unwanted parts, typically
    // extending beyond the silhouette of the object) the direction computed by 'dx x dy' is more often that not from the
    // "good part" of the splat, hence, it cannot be used to discard the whole splat or detect that it has "bad parts"!
    vec3 dx = normalize( pos_dx.xyz/pos_dx.w - pos.xyz/pos.w ); // possible to reuse computations from above?
    vec3 dy = normalize( pos_dy.xyz/pos_dy.w - pos.xyz/pos.w );
    if ( cross(dx, dy).z < 0.0 ) {
  #ifdef VS_DISCARD_DEBUG
        debugCol = vec4(1.0, 1.0, 1.0, 1.0); // Not discarding, colouring the whole primitive white instead
  #else
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
  #endif
        return;
    }
#endif

    // Can we discard splats that were "recorded" with a direction very much different from the one we view it with?
    // (This does not take into account perspective. How can we fix this? It does not work very well without this. Or, maybe
    // there is something else wrong?!)
#if 0
    // 140617: NB! If enabled, remember to uncomment the declaration of MV and depthMVinv at the top, and also to set these in the ProxyRenderer.js!
    mat4 tmp = MV * depthMVinv;
    vec3 dir = vec3( -tmp[2][0], -tmp[2][1], -tmp[2][2] );
    dir = normalize(dir);
    if ( dir.z > 0.0 ) { // What is a good theshold here? Started with -0.6, even that is sometimes not strict enough. 
                         // But it removes too much. Trying 0...
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
        return;
    }
#endif
    
    //----------------------------------------------------------------------------------------------------
    //
    // Setting the size of splats
    //
    //----------------------------------------------------------------------------------------------------

#ifdef DEBUG
        if (screenSpaceSized>0) {
#else
    if (true) {
#endif

        // We have already sampled the depths for the adjacent splats in the x-direction (*_dx) and y-direction (*_dy),
        // and will use the screen space difference between these adjacent splats to determine an appropriate (screen
        // space) splat size.
        //
        // It makes sense to limit the splat size, for if splats get very large, it means that a very small portion of
        // the textures (both depth and RGB) will be expanded a lot on screen, which will likely not look particularly
        // nice. Instead of clamping the size, we simply discard these potentially large splats, because a splat with
        // very blurred texture does not look any better being cropped...
        //
        // And to take this even further, it may also make sense to take the overlap factor into consideration. As the
        // overlap factor gets larger, there is less necessity for keeping these overly large splats, and they also tend
        // to evade our other measures for removing "outside silhouette" fragments.
        //
        // We combine discarding of very large splats with a reduction of the size of "just large" splats.
        
        // 141027: Abandoning special treatment of the "most recent proxy model".

        actualSplatOverlap = max( max(abs(scr_dx.x), abs(scr_dx.y)), max(abs(scr_dy.x), abs(scr_dy.y)) ) / splatSize * 2.5; // splatOverlap;
        //actualSplatOverlap = max( length(scr_dx), length(scr_dy) )/1.41 / splatSize * 1.0; // splatOverlap;
        // It is probably not important to have this as large as 2.0. Using 1.0 seems to leave unnecessary gaps. Maybe 1.5 is an ok compromise.
        // On the other hand, the "non-mostRecentModel" is only to be shown a brief period of time, so it may actually
        // be better to have a larger value to avoid noticeable gaps, rather than having a perfect texturing. Trying 2.5.

        if ( actualSplatOverlap > 100.0 ) {
            // Such huge splats we simply get rid of. Cons for keeping large splats: Bad texture resolution inside
            // the splats, and tricky to crop them along the scene silhouettes. Also, danger of those "bad and
            // large" splats shadowing better and smaller ones. Not easy to handle the last one. (Maybe with a
            // multi-pass algorithm.) Pros: Better coverage
            // 141021: Increasing this threshold from 10 to 100, to avoid "large blank regions" when running only with "most recent model".
#ifdef VS_DISCARD_DEBUG
            debugCol = vec4(1.0, 0.0, 0.0, 1.0); // Not discarding, colouring the fragment red instead
#else
            gl_Position = vec4(0.0, 0.0, -1000.0, 0.0);
#endif
            return;
        }
        // We restrict the size of these splats to avoid "silhouette overshooting". We need at least 2.0 to get nice
        // silhouettes when MV*MV_depth_inv == id, which is only relevant for the "most recent proxy", so we treat this specially below.
        // 140617: Changing this from 1.5 (used together with 1.5 above) to 2.5 (with 2.5 above.) Hopefully, this would give better
        //         coverage during interaction, while still keeping the "silhouette overshooting artifacts" small enough to be
        //         overwritten by the "most recent model".
        // 141021: Also increasing this, to better match the new threshold above. (From 3.0 to 5.0)
        actualSplatOverlap = clamp(actualSplatOverlap, 0.0, 5.0);
    } else {
        actualSplatOverlap = splatOverlap;
    }
    gl_PointSize = actualSplatOverlap * splatSize;

    //----------------------------------------------------------------------------------------------------
    //
    // Finalizing the texture transformations to be used in the FS
    //
    //----------------------------------------------------------------------------------------------------

    // The vectors 'st_e1' and 'st_e2' span the region in the textures with lower left corner 'st' (splat_00), to which
    // the screen space region with lower left corner 'pos.xy * vec2(vp_width, vp_height) / 2.0 / pos.w' and spanning
    // vectors 'scr_dx' and 'scr_dy' should be mapped.
    // @@@ vp-size or DEPTH-size?! (Don't quite get this comment...)
    
    intraSplatTexCooTransform2 =
	invrs( mat2(scr_dx, scr_dy) ) *                        // These terms map gl_PointCoord-0.5 to the (scr_dx, scr_dy)-spanned
	mat2( splatSizeVec.x, 0.0, 0.0, splatSizeVec.y ) *     // screen region, producing a coordinate in [0, 1]^2 for points inside
	actualSplatOverlap;                                    // this region.

    intraSplatTexCooTransform =
	mat2( st_e1, st_e2 ) *                                 // This term maps the screen region "between the splats", given
	                                                       // with coordinates in [0, 1]^2, to the corresponding texture region.
	intraSplatTexCooTransform2;

    // Discarding the primitive if the transformation stretches too much. We test on the length of the spanning vectors,
    // as well as the angle between them. (Could we do this more elegantly? We want to test for "affinity" in some
    // sense...)

    // Without "screen-spaced" splats: length seems to be overlap/#splats
    vec2 stretch_target = actualSplatOverlap / vec2(splats_x, splats_y);
    vec2 basis_lengths = vec2(length(intraSplatTexCooTransform[0]), length(intraSplatTexCooTransform[1]));
    vec2 stretch_factor = basis_lengths / stretch_target;

    // Angle between the two vectors
    float angle = acos( dot(intraSplatTexCooTransform[0], intraSplatTexCooTransform[1])/basis_lengths.x/basis_lengths.y );
    float distortion_factor = angle/(0.5*PI);
    
#ifdef CULL_SKEWED_SPLATS
    if ( (abs(stretch_factor.x-1.0)>0.7) ||
         (abs(stretch_factor.y-1.0)>0.7) ||
         (abs(distortion_factor-1.0)>0.7) ) {
  #ifdef VS_DISCARD_DEBUG
        // Not discarding, colouring the fragment dark red|green|blue instead
        if (abs(stretch_factor.x-1.0)>0.7) {
            debugCol = vec4(0.4, 0.0, 0.0, 1.0);
        }
        if (abs(stretch_factor.y-1.0)>0.7) {
            debugCol = vec4(0.0, 0.4, 0.0, 1.0);
        }
        if (abs(distortion_factor-1.0)>0.7) {
            debugCol = vec4(0.0, 0.0, 0.4, 1.0);
        }
  #else
    #ifdef BLOB_INSTEAD_OF_SKEWED_SPLAT
        intraSplatTexCooTransform = mat2(0.0); // This will cause the splat to get uniform coloring
    #else
        gl_Position = vec4(0.0, 0.0, -1000.0, 0.0); // This amounts to a "discard" operation on the primitive
    #endif
  #endif
        return;
    }
#endif

}
