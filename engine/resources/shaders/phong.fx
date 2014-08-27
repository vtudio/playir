/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : phong.fx
 * Description : Basic phong lighting.
 *
 * Created     : 08/09/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

precision mediump float;

// -------
// Globals
// -------
uniform highp mat4 u_projectionMatrix;
uniform highp mat4 u_viewMatrix;
uniform highp mat4 u_modelMatrix;
uniform vec4 u_modelColour;

//#ifdef LIGHTING_ENABLED
uniform mat4 u_modelNormalMatrix;
uniform vec3 u_lightPosition;
uniform vec4 u_lightDiffuse;
//#endif


// ------------------
// VS Output/PS Input
// ------------------
varying vec2 ps_texCoord;

//#ifdef LIGHTING_ENABLED
varying highp vec3 ps_worldPosition;
varying vec3 ps_worldNormal;
//#endif


// -----------------
#ifdef VERTEX_SHADER
// -----------------
// VS Input
attribute highp vec3 vs_position;
attribute vec2 vs_texCoord;

//#ifdef LIGHTING_ENABLED
attribute vec3 vs_normal;
//#endif

void main()
{
    mat4 modelViewMatrix = u_viewMatrix * u_modelMatrix;

    gl_Position = u_projectionMatrix * modelViewMatrix * vec4( vs_position, 1.0 );
    ps_texCoord = vs_texCoord;

//#ifdef LIGHTING_ENABLED
    ps_worldPosition = vec3( modelViewMatrix * vec4( vs_position, 1.0 ) );

#ifdef QT
    ps_worldNormal = vs_normal;
    ps_worldNormal = vec3( 1, 1, 1 );
#else
    // Works for uniform scaled models
    // normal.w must be 0.0 to kill off translation
    vec3 fakeNormal = vs_normal;
    fakeNormal = vec3( 1, 1, 1 );
    vec3 transformedNormal = vec3( modelViewMatrix * vec4( fakeNormal, 0.0 ) );
    ps_worldNormal = normalize( transformedNormal );
#endif
    // Normal matrix is required for non-uniform scaled models
//    vec3 transformedNormal = ( u_modelNormalMatrix * vec4( vs_normal, 1.0 ) ).xyz;
//    ps_worldNormal = normalize( transformedNormal );
//#endif
}

#endif



// ----------------
#ifdef PIXEL_SHADER
// ----------------

uniform sampler2D s_diffuseTexture;

void main()
{
    vec4 textureColour = texture2D( s_diffuseTexture, ps_texCoord ).rgba;

//#ifdef LIGHTING_ENABLED

    vec3 u_lightPosition = vec3( -100.0, 100.0, 100.0 );
    vec4 u_lightDiffuse = vec4( 1.0, 1.0, 1.0, 1.0 );

    vec3 L = normalize( u_lightPosition - ps_worldPosition );
    vec4 Idiff = u_lightDiffuse * max( dot( ps_worldNormal, L ), 0.0 );
    Idiff = clamp( Idiff, 0.5, 1.0 );
    Idiff.a = 1.0;

    gl_FragColor = Idiff * u_modelColour * textureColour;

//#else
//
//    gl_FragColor = u_modelColour * textureColour;
//
//#endif
}

#endif
