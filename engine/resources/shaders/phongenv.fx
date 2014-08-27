/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : phongenv.fx
 * Description : Uses multitexturing for env mapping.
 *
 * Created     : 10/10/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

precision mediump float;

// -------
// Globals
// -------
uniform highp mat4 u_modelViewProjectionMatrix;
uniform highp mat4 u_modelViewMatrix;
uniform vec4 u_modelColour;

uniform mat4 u_modelNormalMatrix;
uniform vec3 u_lightPosition;
uniform vec4 u_lightDiffuse;

uniform vec3 u_cameraPosition;


// ------------------
// VS Output/PS Input
// ------------------
varying vec2 ps_texCoord;

varying highp vec3 ps_worldPosition;
varying vec3 ps_worldNormal;

varying vec3 ps_reflectionVector;


// -----------------
#ifdef VERTEX_SHADER
// -----------------
// VS Input
attribute highp vec3 vs_position;
attribute vec2 vs_texCoord;

attribute vec3 vs_normal;

mat3 GetLinearPart(mat4 m)
{
    mat3 result;

    result[0][0] = m[0][0];
    result[0][1] = m[0][1];
    result[0][2] = m[0][2];

    result[1][0] = m[1][0];
    result[1][1] = m[1][1];
    result[1][2] = m[1][2];

    result[2][0] = m[2][0];
    result[2][1] = m[2][1];
    result[2][2] = m[2][2];

    return result;
}

void main()
{
    gl_Position = u_modelViewProjectionMatrix * vec4( vs_position, 1.0 );
    ps_texCoord = vs_texCoord;

    ps_worldPosition = vec3( u_modelViewMatrix * vec4( vs_position, 1.0 ) );

#ifdef QT
    ps_worldNormal = vec3( 1, 1, 1 );
#else

    // Works for uniform scaled models
    // normal.w must be 0.0 to kill off translation
    vec3 transformedNormal = vec3( u_modelViewMatrix * vec4( vs_normal, 0.0 ) );
    ps_worldNormal = normalize( transformedNormal );

    // Normal matrix is required for non-uniform scaled models
    //vec3 transformedNormal = ( u_modelNormalMatrix * vec4( vs_normal, 1.0 ) ).xyz;
    //ps_worldNormal = normalize( transformedNormal );

#endif

	mat3 modelView3x3 = GetLinearPart( u_modelViewMatrix );

    // find world space normal.
    vec3 N = normalize( modelView3x3 * ps_worldNormal );

    // find world space eye vector.
    vec3 E = normalize( ps_worldPosition - u_cameraPosition );

    ps_reflectionVector = reflect( E, N );
}

#endif



// ----------------
#ifdef PIXEL_SHADER
// ----------------

uniform sampler2D s_diffuseTexture;
uniform sampler2D s_envTexture;

void main()
{
    // Environment map
    vec4 envColour;
    vec3 vR = normalize( ps_reflectionVector );

    // Select the front or back env map according to the sign of vR.z.
    if( vR.z > 0.0 )
    {
        // calculate the forward paraboloid map texture coordinates
        vec2 frontUV;
        frontUV = ( vR.xy / (2.0 * ( 1.0 + vR.z ) ) ) + 0.5;
        envColour = texture2D( s_envTexture, frontUV );
    }
    else
    {
        // calculate the backward paraboloid map texture coordinates
        vec2 backUV;
        backUV = ( vR.xy / ( 2.0 * ( 1.0 - vR.z ) ) ) + 0.5;
        envColour = texture2D( s_envTexture, backUV );
    }

    vec4 diffColour = texture2D( s_diffuseTexture, ps_texCoord ).rgba;
    vec4 textureColour = diffColour + envColour * 0.2;
    textureColour = clamp( textureColour, 0.0, 1.0 );

    vec3 L = normalize( u_lightPosition - ps_worldPosition );
    vec4 Idiff = u_lightDiffuse * max( dot( ps_worldNormal, L ), 0.0 );
    Idiff = clamp( Idiff, 0.5, 1.0 );
    Idiff.a = 1.0;

    gl_FragColor = Idiff * u_modelColour * textureColour;
}

#endif
