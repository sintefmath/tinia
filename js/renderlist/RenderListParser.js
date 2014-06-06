/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

dojo.require( "renderlist.RenderList" );
dojo.provide( "renderlist.RenderListParser" );

dojo.declare( "renderlist.RenderListParser", null, {
    constructor:
        function( renderlist ) {
        },

    parse:
        function( store, xml ) {
                     if( !xml ) {
                         return;
                     }

            var from_revision, to_revision;
                     console.debug( xml );

            dojo.query( "renderList", xml ).forEach( function(node, index, arr ) {
                from_revision = dojo.attr( node, 'from' );
                to_revision = dojo.attr( node, 'to' );
            } );
            if( (from_revision === null) || (to_revision === null ) ) {
                console.debug( "missing revision numbers" );
                return;
            }

            if( from_revision != store.revision() ) {
                console.debug( "update not compatible with store's revision number" );
            }

            var that = this;
            var keep = {};
            dojo.query( "updateItems > buffers > update",  xml )
                .forEach( function(node, index, arr ) { that.buffer(store, keep, node); } );
            dojo.query( "updateItems > shaders > update",  xml )
                .forEach( function(node, index, arr ) { that.shader(store, keep, node); } );
            dojo.query( "updateItems > actions > setShader", xml )
                .forEach( function(node, index, arr ) { that.setShader(store, keep, node); } );
            dojo.query( "updateItems > actions > setInputs", xml )
                .forEach( function(node, index, arr ) { that.setInputs(store, keep, node); } );
            dojo.query( "updateItems > actions > setUniforms", xml )
                .forEach( function(node, index, arr ) { that.setUniforms(store, keep, node); } );
            dojo.query( "updateItems > actions > setLocalCoordSys", xml )
                .forEach( function(node, index, arr ) { that.setLocalCoordSys(store, keep, node); } );
            dojo.query( "updateItems > actions > draw", xml )
                .forEach( function(node, index, arr ) { that.draw(store, keep, node); } );

            // check for presence of prune items (it can be empty)
            if( dojo.query( "pruneItems", xml ).length > 0 ) {
                dojo.query( "pruneItems > keep", xml ).forEach( function( node, index, arr ) {
                    var id = dojo.attr( node, 'id' );
                    if( id !== null ) {
                        keep[ id ] = 1;
                    }
                } );
                store.pruneItems( keep );
            }

            // check for presence of new draw order (it can be empty)
            if( dojo.query( "drawOrder", xml ).length > 0 ) {
                var actions = new Array();
                dojo.query( "drawOrder > invoke", xml ).forEach( function( node, index, arr ) {
                    var action = dojo.attr( node, 'action' );
                    if( action !== null ) {
                        actions[ actions.length ] = action;
                    }
                });
                store.updateDrawOrder( actions );
                console.debug( "moo3" );
            }

            store.setRevision( to_revision );
        },

        _JSONparseNodeList:
            function( nodelist ) {
                var data;
                var res;
                for( var i=0; i<nodelist.length; i++) {
                    data = (i==0?"":data) + nodelist[i].data;
                }
                try {
                    res = JSON.parse( data );
                }
                catch( e ) {
                    console.log( e + "\nwhile parsing\n" + data );
                }
                return res;
            },

        float2:
            function( node ) {
                if( node.firstChild ) {
                    var val = this._JSONparseNodeList( node.childNodes );
                    if( !(val instanceof Array ) ) {
                        return null;
                    }
                    else if( val.length != 2 ) {
                        return null;
                    }
                    else {
                        return val;
                    }
                }
                else {
                    return null;
                }
            },

        float3:
            function( node ) {
                if( node.firstChild ) {
                    var val = this._JSONparseNodeList( node.childNodes );
                    if( !(val instanceof Array ) ) {
                        return null;
                    }
                    else if( val.length != 3 ) {
                        return null;
                    }
                    else {
                        return val;
                    }
                }
                else {
                    return null;
                }
            },

        float4:
            function( node ) {
                if( node.firstChild ) {
                    var val = this._JSONparseNodeList( node.childNodes );
                    if( !(val instanceof Array ) ) {
                        return null;
                    }
                    else if( val.length != 4 ) {
                        return null;
                    }
                    else {
                        return val;
                    }
                }
                else {
                    return null;
                }
            },

    float3x3:
        function( node ) {
            if( node.firstChild ) {
                var val = this._JSONparseNodeList( node.childNodes );
                if( !(val instanceof Array ) ) {
                    return null;
                }
                else if( val.length != 9 ) {
                    return null;
                }
                else {
                    return val;
                }
            }
            else {
                return null;
            }
        },

    float4x4:
        function( node ) {
            if( node.firstChild ) {
                var val = this._JSONparseNodeList( node.childNodes );
                if( !(val instanceof Array ) ) {
                    return null;
                }
                else if( val.length != 16 ) {
                    return null;
                }
                else {
                    return val;
                }
            }
            else {
                return null;
            }
        },

    setShader:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            var shader = dojo.attr( node, 'shader' );
            if( (id!==null) && (shader!==null) ) {
                store.updateSetShader( id, shader );
            }
            else {
                console.debug( "Malformed <setShader>" );
                console.debug( node );
            }
        },

    setInputs:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            var shader = dojo.attr( node, 'shader' );
            if( (id!==null) && (shader!==null) ) {
                var inputs = new Array();
                dojo.query( "input", node ).forEach( function(node, index, nodeList ) {
                    var symbol = dojo.attr( node, 'symbol' );
                    var buffer = dojo.attr( node, 'buffer' );
                    var components = dojo.attr( node, 'components' );
                    var offset = dojo.attr( node, 'offset' );
                    var stride = dojo.attr( node, 'stride' );
                    if( (symbol!==null) && (buffer!==null) && (components!==null) && (offset!==null) && (stride!==null) ) {
                        inputs[ inputs.length ] = {
                            m_symbol      : symbol,
                            m_buffer      : buffer,
                            m_components  : components,
                            m_offset      : offset,
                            m_stride      : stride
                        };
                    }
                } );
                store.updateSetInputs( id, shader, inputs );
            }
            else {
                console.debug( "Malformed <setInputs>" );
                console.debug( node );
            }
        },

    setUniforms:
        function(store, keep, node) {
            var success = true;
            var id = dojo.attr( node, 'id' );
            var shader = dojo.attr( node, 'shader' );
            if( (id===null) || (shader===null) ) {
                success = false;
            }
            else {
                var that = this;
                var uniforms = new Array();
                dojo.query( "uniform", node ).forEach( function(node, index, nodeList ) {
                    var symbol = dojo.attr( node, 'symbol' );
                    if( symbol === null ) {
                        success = false;
                    }
                    else {
                        var uniform = {
                            m_symbol    : symbol,
                            m_semantic  : dojo.attr( node, 'semantic' ),
                            m_type      : null,
                            m_value     : null
                        };
                        uniforms[ uniforms.length ] = uniform;
                        if( uniform.m_semantic !== null ) {
                            // semantic doesn't have value, do nothing.
                        }
                        else if( node.firstChild === null || node.firstChild.firstChild === null ) {
                            success = false;
                        }
                        else if( node.firstChild.nodeName == 'int' ) {
                            uniform.m_type = 'int';
                            uniform.m_value = node.firstChild.firstChild.data;
                        }
                        else if( node.firstChild.nodeName == 'float' ) {
                            uniform.m_type = 'float';
                            uniform.m_value = node.firstChild.firstChild.data;
                        }
                        else if( node.firstChild.nodeName == 'float2' ) {
                            uniform.m_type = 'float2';
                            uniform.m_value = that.float2( node.firstChild );
                        }
                        else if( node.firstChild.nodeName == 'float3' ) {
                            uniform.m_type = 'float3';
                            uniform.m_value = that.float3( node.firstChild );
                        }
                        else if( node.firstChild.nodeName == 'float4' ) {
                            uniform.m_type = 'float4';
                            uniform.m_value = that.float4( node.firstChild );
                        }
                        else if( node.firstChild.nodeName == 'float3x3' ) {
                            uniform.m_type = 'float3x3';
                            uniform.m_value = that.float3x3( node.firstChild );
                        }
                        else if( node.firstChild.nodeName == 'float4x4' ) {
                            uniform.m_type = 'float4X4';
                            uniform.m_value = that.float4x4( node.firstChild );
                        }
                        else {
                            success = false;
                        }
                    }
                });
                if( success ) {
                    store.updateSetUniforms( id, shader, uniforms );
                }
            }

            if(!success) {
                console.debug( "Malformed <setUniforms>" );
                console.debug( node );
            }
        },


    setLocalCoordSys:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            if( id !== null ) {
                var that = this;
                var to_world = null;
                var from_world = null;
                dojo.query( "> ", node ).forEach( function(node, index, nodeList ) {
                    var value;
                    if( node.nodeName == 'toWorld' ) {
                        value = that.float4x4( node );
                        if( value !== null ) {
                            to_world = value;
                        }
                    }
                    else if( node.nodeName == 'fromWorld' ) {
                        value = that.float4x4( node );
                        if( value !== null ) {
                            from_world = value;
                        }
                    }
                });
                if( (to_world!==null) && (from_world!==null) ) {
                    store.updateSetLocalCoordSys( id, from_world, to_world );
                }
            }
        },

    draw:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            var mode = dojo.attr( node, 'mode' );
            var first = dojo.attr( node, 'first' );
            var count = dojo.attr( node, 'count' );
            var indices = dojo.attr( node, 'indices' );
            if( (id!==null) && (mode!==null) && (first!==null) && (count!==null) ) {
                store.updateDraw( id, mode, first, count, indices );
            }
            else {
                console.debug( "Malformed <draw>" );
                console.debug( node );
            }
        },

    shader:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            if( id === null ) {
                console.debug( "Node has no id" );
                return;
            }
            var vs = null;
            var n = dojo.query( "vertex", node );
            if( n.length > 0 ) {
                if( n[0].firstChild ) {
                    vs = n[0].firstChild.data;
                }
            }
            var tc = null;
            n = dojo.query( "tessControl", node );
            if( n.length > 0 ) {
                if( n[0].firstChild ) {
                    tc = n[0].firstChild.data;
                }
            }
            var te = null;
            n = dojo.query( "tessEvaluation", node );
            if( n.length > 0 ) {
                if( n[0].firstChild ) {
                    te = n[0].firstChild.data;
                }
            }
            var gs = null;
            n = dojo.query( "geometry", node );
            if( n.length > 0 ) {
                if( n[0].firstChild ) {
                    gs = n[0].firstChild.data;
                }
            }
            var fs = null;
            n = dojo.query( "fragment", node );
            if( n.length > 0 ) {
                if( n[0].firstChild ) {
                    fs = n[0].firstChild.data;
                }
            }
            store.updateShader( id, vs, tc, te, gs, fs );
            keep[ id ] = 1;
        },

    buffer:
        function(store, keep, node) {
            var id = dojo.attr( node, 'id' );
            if( id === null ) {
                console.debug( "Node has no id" );
                return;
            }
            if( !node.firstChild ) {
                console.debug( "Node has no body" );
                return;
            }
            var body = this._JSONparseNodeList( node.childNodes );
            if( !(body instanceof Array) ) {
                console.debug( "body doesn't evaluate to Array" );
                return;
            }
            var count = dojo.attr( node, 'count' );
            if( count === null ) {
                console.debug( "Node has no count attribute" );
            }
            else if( count != body.length ) {
                console.debug( "Mismatch between count ("+count+") attribute and elements in body (" + body.length + ")" );
            }
            var type = dojo.attr( node, 'type' );
            if( type === null ) {
                console.debug( "Node has no type attribute " );
                return;
            }
            store.updateBuffer( id, type, body );
            keep[ id ] = 1;
         }


});
