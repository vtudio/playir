/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : basic_vc.fx
 * Description : Basic unlit shader with vertex colours.
 *
 * Created     : 21/01/12
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#define VERTEX_COLOUR

precision mediump float;

// Globals
uniform highp mat4 u_projectionMatrix;
uniform highp mat4 u_viewMatrix;
uniform highp mat4 u_modelMatrix;
uniform vec4 u_modelColour;

// PS Input
varying vec2 ps_texCoord;

#ifdef VERTEX_COLOUR
varying vec4 ps_colour;
#endif


#ifdef VERTEX_SHADER

// VS Input
attribute highp vec3 vs_position;
attribute vec2 vs_texCoord;

#ifdef VERTEX_COLOUR
attribute vec4 vs_colour;
#endif

void main()
{
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4( vs_position, 1.0 );
    ps_texCoord = vs_texCoord;

#ifdef VERTEX_COLOUR
    ps_colour = vs_colour;
#endif
}

#endif


#ifdef PIXEL_SHADER

uniform sampler2D s_diffuseTexture;

void main()
{
    vec4 colour = u_modelColour;

#ifdef VERTEX_COLOUR
    colour *= ps_colour;
#endif

    gl_FragColor = colour * texture2D( s_diffuseTexture, ps_texCoord ).rgba;
}

#endif
