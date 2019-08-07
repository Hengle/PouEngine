#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

/*layout(location = 0) in vec3 inPos;
layout(location = 1) in float inRotation;
layout(location = 2) in vec2 inSize;
layout(location = 3) in vec2 inCenter;*/

layout(location = 0)  in mat4  inModel; //Use 4
//1
//2
//3

layout(location = 4) in vec4 inColor;
//layout(location = 5) in vec3 inRmt;
layout(location = 5) in vec2 inTexCoord;
layout(location = 6) in vec2 inTexExtent;
layout(location = 7) in uvec2 inAlbedoTexId;
/*layout(location = 9) in uvec2 inHeightTexId;
layout(location = 10) in uvec2 inNormalTexId;
layout(location = 11) in uvec2 inRmtTexId;*/

layout(location = 0) flat out vec4 fragColor;
//layout(location = 1) flat out vec3 fragRmt;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uvec2 fragAlbedoTexId;
/*layout(location = 4) flat out uvec2 fragHeightTexId;
layout(location = 5) flat out uvec2 fragNormalTexId;
layout(location = 6) flat out uvec2 fragRmtTexId;*/
layout(location = 3) out vec3 screenPos;


vec2 vertPos[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    gl_Position = viewUbo.view * (inModel * vec4(vertPos[gl_VertexIndex],0.0,1.0) - vec4(pc.camPosAndZoom.xyz,0.0));
    gl_Position = vec4(gl_Position.xyz/gl_Position.w, 1.0);

    //gl_Position.xyz = vec4(viewUbo.view * (vec4(vertPos[gl_VertexIndex],0.0,0.0) * inModel) - vec4(pc.camPosAndZoom.xyz,0.0)) ;

    screenPos = gl_Position.xyz;

    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);

    /*vec4 spriteViewCenter = viewUbo.view*vec4(inPos-pc.camPosAndZoom.xyz,1.0);
    spriteViewCenter.z = inPos.z;

    //This should be computed outside on CPU
    float c = cos(inRotation);
    float s = sin(inRotation);

    gl_Position = vec4(spriteViewCenter.xyz + vec3( mat2(c,s,-s,c) * (vertPos[gl_VertexIndex] * inSize.xy - inCenter), 0.0), 1.0);

    screenPos = gl_Position.xyz;

    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);*/

    fragTexCoord = inTexExtent * vertPos[gl_VertexIndex] + inTexCoord;
    fragColor    = inColor;
    fragAlbedoTexId    = inAlbedoTexId;
}


