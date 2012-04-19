dojo.provide("renderlist.RenderList");


function checkGL(gl, who) {
    var err = gl.getError();
    if( err == gl.NO_ERROR ) {
        return;
    }
    var msg = "unknown error";
    switch(err) {
        case gl.OUT_OF_MEMORY:
            msg = "GL_OUT_OF_MEMORY";
            break;
        case gl.INVALID_VALUE:
            msg = "GL_INVALID_VALUE";
            break;
        case gl.INVALID_ENUM:
            msg = "GL_INVALID_ENUM";
            break;
        case gl.INVALID_OPERATION:
            msg = "GL_INVALID_OPERATION";
            break;
        case gl.INVALID_FRAMEBUFFER_OPERATION:
            msg = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
    }
    window.console.log( who + ': ' + msg );
}


/**
 * The RenderListStore class holds the data associated with a
 * renderlist
 */
dojo.declare("renderlist.RenderListStore", null, {
    constructor: function(gl) {
        this.m_gl = gl;
        this.m_buffers = {};
        this.m_shaders = {};
        this.m_actions = {};
        this.m_revision = 0;
        this.m_draw_order = [];
    },

    revision:
        function() { return this.m_revision; },

    setRevision:
        function( revision ) { this.m_revision = revision; },

    drawOrder:
        function() { return this.m_draw_order; },

    action:
        function( id ) { return this.m_actions[ id ]; },

    pruneItems:
        function( keep ) {
            console.debug( keep );
        },

    updateDrawOrder:
        function( actions ) {
            this.m_draw_order = actions;
        },

    /// Set new buffer contents.
    ///
    /// When a buffer first is bound to a target, the buffer is locked to that
    /// target for its lifetime, i.e., vertex and index data cannot be
    /// interchanged as in desktop GL. When declaring a buffer, we don't know
    /// how it is going to be used, so we just keep a copy of data without
    /// uploading it to GL. However, the actions setInputs and draw specify
    /// the use of the buffers, so we detain uploading it to GL until those
    /// actions are created. Librenderlist taints actions that references a
    /// buffer when the buffer is updated, so we are assured that the actions
    /// are always updated when a buffer is modified.
    updateBuffer:
        function( id, type, data ) {
            this.deleteBuffer( id );
            item = {
                m_type              : type,
                m_data              : data,
                m_vertex_buffer     : null,
                m_vertex_type       : null,
                m_vertex_typesize   : null,
                m_index_buffer      : null,
                m_index_type        : null,
                m_index_typesize    : null
            };
            this.m_buffers[ id ] = item;
        },

    deleteBuffer:
        function( id ) {
            var item = this.m_buffers[ id ];
            if( item !== undefined ) {
                if( item.m_vertex_buffer !== null ) {
                    this.m_gl.deleteBuffer( item.m_vertex_buffer );
                }
                if( item.m_index_buffer !== null ) {
                    this.m_gl.deleteBuffer( item.m_index_buffer );
                }
            }
            delete this.m_buffers[ id ];
        },


    /// Set up a buffer for being used as vertex data.
    initVertexBuffer:
        function( id ) {
           var item = this.m_buffers[ id ];
           if( item === null ) {
                console.debug( "buffer " + id + " doesn't exist." );
                return null;
           }
           if( item.m_vertex_buffer === null ) {
               var data = null;
               if( item.m_type == 'float' ) {
                   item.m_vertex_type     = this.m_gl.FLOAT;
                   item.m_vertex_typesize = 4;
                   data = new Float32Array( item.m_data );
               }
               else {
                   console.debug( "buffer " + id + " has unsupported vertex type " + item.m_type );
                   return null;
               }
               item.m_vertex_buffer = this.m_gl.createBuffer();
               this.m_gl.bindBuffer( this.m_gl.ARRAY_BUFFER, item.m_vertex_buffer );
               this.m_gl.bufferData( this.m_gl.ARRAY_BUFFER, data, this.m_gl.STATIC_DRAW );
               checkGL( this.m_gl, "initVertexBuffer" );
           }
           return item;
        },

    initIndexBuffer:
        function( id ) {
            var item = this.m_buffers[ id ];
            if( item === null ) {
                console.debug( "buffer " + id + " doesn't exist." );
                return false;
            }
            if( item.m_index_buffer === null ) {
                if( item.m_data.length == 0 ) {
                    console.debug( "Empty buffer!" )
                    return false;
                }

                var data = null;
                if( item.m_type == 'int' ) {
                    item.m_index_type     = this.m_gl.UNSIGNED_SHORT;
                    item.m_index_typesize = 2;
                    data = new Uint16Array( item.m_data );
                }
                else {
                    console.debug( "buffer " + id + " has unsupported index type " + item.m_type );
                    return false;
                }
                item.m_index_buffer = this.m_gl.createBuffer();
                this.m_gl.bindBuffer( this.m_gl.ELEMENT_ARRAY_BUFFER, item.m_index_buffer );
                this.m_gl.bufferData( this.m_gl.ELEMENT_ARRAY_BUFFER, data, this.m_gl.STATIC_DRAW );
                checkGL( this.m_gl, "initIndexBuffer" );
            }
            return true;
        },

    updateShader:
        function( id, vs, tc, te, gs, fs ) {
            this.deleteShader( id );
            var item = {
                m_program   : this.m_gl.createProgram()
            };
            this.m_shaders[ id ] = item;

            var s;
            if( vs ) {
                s = this.compileShader( vs, this.m_gl.VERTEX_SHADER );
                this.m_gl.attachShader( item.m_program, s );
            }
            if( tc ) {
                s = this.compileShader( tc, this.m_gl.TESS_CONTROL_SHADER );
                this.m_gl.attachShader( item.m_program, s );
            }
            if( te ) {
                s = this.compileShader( te, this.m_gl.TESS_EVALUATION_SHADER );
                this.m_gl.attachShader( item.m_program, s );
            }
            if( gs ) {
                s = this.compileShader( gs, this.m_gl.GEOMETRY_SHADER );
                this.m_gl.attachShader( item.m_program, s );
            }
            if( fs ) {
                s = this.compileShader( fs, this.m_gl.FRAGMENT_SHADER );
                this.m_gl.attachShader( item.m_program, s );
            }
            this.m_gl.linkProgram( item.m_program );
            if ( !this.m_gl.getProgramParameter( item.m_program, this.m_gl.LINK_STATUS) ) {
                console.debug( "Failed to log shader program" );
                console.debug( this.m_gl.getProgramParameter( item.m_program, this.m_gl.LINK_STATUS) );
                this.m_gl.deleteProgram( item.m_program );
                item.m_program = null;
            }
            checkGL( this.m_gl, "linkShader" );
        },

    deleteShader:
        function( id ) {
            var item = this.m_shaders[ id ];
            if( item !== undefined ) {
                this.m_gl.deleteProgram( item.m_program );
                delete this.m_shaders[id];
            }
        },

    compileShader: function(src, type) {
        var shader = this.m_gl.createShader(type);
        if( shader != null ) {
            this.m_gl.shaderSource( shader, src );
            this.m_gl.compileShader( shader );
            if( !this.m_gl.getShaderParameter( shader, this.m_gl.COMPILE_STATUS ) ) {
                var infolog = this.m_gl.getShaderInfoLog( shader );
                window.console.log( "Failed to compile shader: " + infolog );
                this.m_gl.deleteShader( shader );
                shader = null;
            }
        }
        checkGL(this.m_gl, "compileShader");

        return shader;
    },

    linkShader: function(vertex, tess_ctrl, tess_eval, geometry, fragment) {
        var program = this.m_gl.createProgram();
        if( program != null ) {
            this.m_gl.attachShader(program, vertex);
            if (tess_ctrl) {
                this.m_gl.attachShader(program, tess_ctrl);
            }
            if (tess_eval) {
                this.m_gl.attachShader(program, tess_eval);
            }
            if (geometry) {
                this.m_gl.attachShader(program, geometry);
            }
            this.m_gl.attachShader(program, fragment);

            this.m_gl.linkProgram(program);
            if (!this.m_gl.getProgramParameter( program, this.m_gl.LINK_STATUS)) {
                window.console.log("Failed to link shader program:\n" +
                    this.m_gl.getProgramParameter( program, this.m_gl.LINK_STATUS));
                this.m_gl.deleteProgram( program );
                program = null;
            }
        }
        checkGL(this.m_gl, "linkShader");

        return program;
    },
    updateDraw:
                 function( id, mode, first, count, indices )
                 {
                     if( indices === null ) {
                         this.m_actions[ id ] = {
                             m_type         : 'Draw',
                             m_mode         : mode,
                             m_first        : first,
                             m_count        : count,
                             m_indices      : null
                         };
                     }
                     else if( this.initIndexBuffer( indices ) ) {
                         this.m_actions[ id ] = {
                             m_type         : 'Draw',
                             m_mode         : mode,
                             m_first        : first,
                             m_count        : count,
                             m_indices      : indices,
                             m_element_type : this.m_buffers[ indices ].m_index_type,
                             m_offset       : this.m_buffers[ indices ].m_index_typesize * first
                         };
                     //    console.log( "indexed drawing not implemented yet." );
                     }
                     else {
                         console.log( "Error processing updateDraw." );
                     }

                 },

    updateSetLocalCoordSys:
                 function( id, from_world, to_world ) {
                     this.m_actions[ id ] = {
                         m_type         : 'SetLocalCoordSys',
                         m_to_world     : to_world,
                         m_from_world   : from_world
                     };
                 },

    updateSetShader:
                 function( id, shader ) {
                     var sh = this.m_shaders[ shader ];
                     if( sh !== undefined ) {
                         this.m_actions[ id ] = {
                             m_type     : 'SetShader',
                             m_program  : sh.m_program
                         };
                     }
                 },
    updateSetUniforms:
                 function( id, shader, uniforms ) {
                     var loc, u;
                     var sh = this.m_shaders[ shader ];
                     if( sh !== undefined ) {
                         var action = {
                             m_type : 'SetUniforms',
                             m_uniforms : new Array()
                         };
                         this.m_actions[ id ] = action;
                         for( var j=0; j<uniforms.length; j++ ) {
                             loc = this.m_gl.getUniformLocation( sh.m_program, uniforms[j].m_symbol );
                             if( loc !== null ) {
                                 action.m_uniforms[ action.m_uniforms.length ] = {
                                     m_type     : uniforms[j].m_type,
                                     m_semantic : uniforms[j].m_semantic,
                                     m_value    : uniforms[j].m_value,
                                     m_location : loc
                                 };
                             }
                         }
                     }
                 },

    updateSetInputs:
                 function( id, shader, inputs ) {
                     var loc, i, b;
                     var sh = this.m_shaders[ shader ];
                     if( sh !== undefined ) {
                         var action = {
                             m_type     : 'SetInputs',
                             m_inputs   : new Array()
                         };
                         this.m_actions[ id ] = action;
                         for( var j=0; j<inputs.length; j++ ) {
                             i = inputs[j];
                             b = this.initVertexBuffer( i.m_buffer );
                             if( b !== null ) {
                                 loc = this.m_gl.getAttribLocation( sh.m_program, i.m_symbol );
                                 if( loc !== null ) {
                                     action.m_inputs[ action.m_inputs.length ] = {
                                         m_buffer       : i.m_buffer,
                                         m_components   : i.m_components,
                                         m_type         : b.m_vertex_type,
                                         m_offset       : b.m_vertex_typesize*i.m_offset,
                                         m_stride       : b.m_vertex_typesize*i.m_stride,
                                         m_location     : loc
                                     };
                                 }
                             }
                         }
                     }
                 }


});
