dojo.require( "renderlist.RenderList" );
dojo.provide( "renderlist.RenderListRenderer" );

dojo.declare( "renderlist.RenderListRenderer", null, {
    constructor:
        function(gl) {
            this.m_gl = gl;
            this.m_view_coord_sys = null;
            this.m_local_coord_sys = null;
            this.m_revision = 0;
            this.initGLState();
            this.m_renderer = function( store, view_coordsys ) {
            //console.debug( view_coordsys );
            };
        },

    render:
        function(store, view_coord_sys) {
            if( this.m_revision != store.revision() ) {
                console.debug( "renderList store has changed, creating new render code." );
                var renderer = "";
                var max_attrib = 0;
                renderer += "var gl = store.m_gl;\n";
                renderer += "gl.clearColor( 0, 0, 0, 1 );\n";
                renderer += "gl.viewport( 0, 0, gl.canvas.width, gl.canvas.height );\n";
                renderer += "gl.clear( gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT );\n";
                renderer += "var proj = view_coordsys.m_projection;\n";
                renderer += "var iproj = view_coordsys.m_projection_inverse;\n";
                renderer += "var view_to_world = view_coordsys.m_to_world;\n";
                renderer += "var view_from_world = view_coordsys.m_from_world;\n";
                renderer += "var local_to_world = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];\n";
                renderer += "var local_from_world = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];\n";
                renderer += "var a, mvp, nm;\n";
                var draw_order = store.drawOrder();
                for( var j in draw_order ) {
                    var id =  draw_order[j];
                    var action = store.action( id );
                    if( action === undefined ) {
                    }
                    else if( action.m_type == 'SetLocalCoordSys' ) {
                        renderer += "local_to_world = store.action(" + id + ").m_to_world;\n"
                        renderer += "local_from_world = store.action(" + id + ").m_from_world;\n";
                    }
                    else if( action.m_type == 'SetShader' ) {
                        renderer += "gl.useProgram( store.action(" + id + ").m_program );\n";
                    }
                    else if( action.m_type == 'Draw' ) {
                        var mode;
                        if( action.m_mode ==        "GL_POINTS" ) {         mode = "gl.POINTS"; }
                        else if( action.m_mode ==   "GL_LINES" ) {          mode = "gl.LINES"; }
                        else if( action.m_mode ==   "GL_LINE_STRIP" ) {     mode = "gl.LINE_STRIP"; }
                        else if( action.m_mode ==   "GL_LINE_LOOP" ) {      mode = "gl.LINE_LOOP"; }
                        else if( action.m_mode ==   "GL_LINE_LOOOP" ) {      mode = "gl.LINE_LOOP"; }   // fixme
                        else if( action.m_mode ==   "GL_TRIANGLES" ) {      mode = "gl.TRIANGLES"; }
                        else if( action.m_mode ==   "GL_TRIANGLE_STRIP" ) { mode = "gl.TRIANGLE_STRIP"; }
                        else if( action.m_mode ==   "GL_TRIANGLE_FAN" ) {   mode = "gl.TRIANGLE_FAN"; }
                        else if( action.m_mode ==   "GL_QUADS" ) {          mode = "gl.QUADS"; }
                        else if( action.m_mode ==   "GL_QUAD_STRIP" ) {     mode = "gl.QUAD_STRIP"; }
                        else {
                            console.debug( "Illegal primitve mode " + action.m_mode );
                        }
                        if( action.m_indices === null ) {
                            renderer += "gl.drawArrays( " + mode + ", " + action.m_first + ", " + action.m_count + " );\n";
                        }
                        else {
                            renderer += "gl.bindBuffer( gl.ELEMENT_ARRAY_BUFFER, store.m_buffers[ " + action.m_indices + " ].m_index_buffer );\n";
                            renderer += "gl.drawElements( " + mode +
                                    ", " + action.m_count +
                                    ", store.action("+id+").m_element_type" +
                                    ", " + action.m_offset + ");\n";
                            renderer += "gl.bindBuffer( gl.ELEMENT_ARRAY_BUFFER, store.m_buffers[ null ]);\n";
                        }
                    }
                    else if( action.m_type == 'SetInputs' ) {
                        for(var j=0; j<action.m_inputs.length; j++ ) {
                            var lmax_attrib = 0;
                            var i = action.m_inputs[j];
                            var type;
                            if( i.m_type == this.m_gl.FLOAT ) {
                                type = "gl.FLOAT";
                            }
                            lmax_attrib = Math.max( lmax_attrib, i.m_location );
                            renderer += "gl.bindBuffer( gl.ARRAY_BUFFER, store.m_buffers[ " + i.m_buffer + " ].m_vertex_buffer );\n";
                            renderer += "gl.vertexAttribPointer( " + i.m_location + ", "
                                    + i.m_components + ", " + type + ", gl.FALSE, "
                                    + i.m_stride +", " + i.m_offset + " );\n";
                            renderer += "gl.enableVertexAttribArray( " + i.m_location + " );\n";
                        }
                        renderer += "gl.bindBuffer( gl.ARRAY_BUFFER, null );\n";
                        for( var j=lmax_attrib+1; j<max_attrib+1; j++ ) {
                            renderer += "gl.disableVertexAttribArray( " + j + " );\n";
                        }
                        max_attrib = lmax_attrib;
                    }
                    else if( action.m_type == 'SetUniforms' ) {
                        for( var i=0; i<action.m_uniforms.length; i++ ) {
                            var u = action.m_uniforms[i];
                            if( u.m_semantic !== null ) {
                                if( u.m_semantic == 'MODELVIEW_PROJECTION_MATRIX' ) {
                                    renderer += "mvp = mat4.create( proj );\n";
                                    renderer += "mat4.multiply( mvp, view_from_world );\n";
                                    renderer += "mat4.multiply( mvp, local_to_world ); \n";
                                    renderer += "gl.uniformMatrix4fv( store.action(" + id + ").m_uniforms["+i+"].m_location, gl.FALSE, mvp );\n";
                                }
                                else if( u.m_semantic == 'NORMAL_MATRIX' ) {
                                    renderer += "nm = mat4.create( local_from_world );\n";
                                    renderer += "mat4.multiply( nm, view_to_world );\n";
                                    renderer += "nm = mat3.transpose( mat4.toMat3( nm ) );\n";
                                    renderer += "gl.uniformMatrix3fv( store.action(" + id + ").m_uniforms["+i+"].m_location, gl.FALSE, nm );\n";
                                }
                                else {
                                    renderer += "// unsupported uniform semantic " + u.m_semantic + "\n";
                                }
                            }
                            else {
                                if( u.m_type == 'int' ) {
                                    renderer += "gl.uniform1i( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float' ) {
                                    renderer += "gl.uniform1f( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float2' ) {
                                    renderer += "gl.uniform2fv( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float3' ) {
                                    renderer += "gl.uniform3fv( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float4' ) {
                                    renderer += "gl.uniform4fv( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float3x3' ) {
                                    renderer += "gl.uniformMatrix3fv( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else if( u.m_type == 'float4x4' ) {
                                    renderer += "gl.uniformMatrix4fv( store.action(" + id + ").m_uniforms["+i+"].m_location, store.action(" + id + ").m_uniforms["+i+"].m_value );\n";
                                }
                                else {
                                    renderer += "// unsupported uniform type " + u.m_type + "\n";
                                }
                            }

                        }
                    }
                    else {
                        renderer += "// type: " + action.m_type + "\n";
                    }
                }
                for( var j=0; j<max_attrib+1; j++ ) {
                    renderer += "gl.disableVertexAttribArray( " + j + " );\n";
                }
                renderer += "gl.useProgram( null );\n";
                console.debug( renderer );
                this.m_renderer = new Function( "store", "view_coordsys", renderer );
                this.m_revision = store.revision();
            }
            this.m_renderer( store, view_coord_sys );
            return;
        },

    initGLState:
        function() {
            this.m_gl.clearColor(0.5, 0.5, 0.5, 1.0);
            this.m_gl.enable(this.m_gl.DEPTH_TEST);
        }
});
