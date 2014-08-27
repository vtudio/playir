/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : alphacolour.fx
 * Description : Used to draw fonts.
 *
 * Created     : 08/09/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

precision mediump float;

// Globals
uniform highp mat4 u_projectionMatrix;
uniform highp mat4 u_viewMatrix;
uniform highp mat4 u_modelMatrix;
uniform vec4 u_modelColour;

// PS Input
varying vec2 ps_texCoord;


#ifdef VERTEX_SHADER

// VS Input
attribute highp vec3 vs_position;
attribute vec2 vs_texCoord;

void main()
{
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4( vs_position, 1.0 );
    ps_texCoord = vs_texCoord;
}

#endif


#ifdef PIXEL_SHADER

uniform sampler2D s_diffuseTexture;

void main()
{
    gl_FragColor.rgb = u_modelColour.rgb;
    gl_FragColor.a = u_modelColour.a * texture2D( s_diffuseTexture, ps_texCoord ).a;
}

#endif
