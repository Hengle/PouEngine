#version 450
#extension GL_ARB_separate_shader_objects : enable

//1.0/pi ~ 0.31830988618

layout(early_fragment_tests) in;

//layout (constant_id = 0) const float const_aoIntensity = 1;
//layout (constant_id = 1) const float const_gioIntensity = 1;

layout(set = 0, binding = 0) uniform ViewUBO {
    ///mat4 view;
    ///mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
    float proj;
} viewUbo;

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

layout(set = 1, binding = 0) uniform sampler samp;
layout(set = 1, binding = 1) uniform texture2DArray textures[128];
layout(set = 1, binding = 2) uniform texture2DArray renderedTextures[32];

layout (set = 2, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 2, binding = 1) uniform sampler2D samplerPosition;
layout (set = 2, binding = 2) uniform sampler2D samplerNormal;
layout (set = 2, binding = 3) uniform sampler2D samplerRme;
layout (set = 2, binding = 4) uniform sampler2D samplerBentNormals;


/*layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;*/

layout(location = 0) flat in vec4  lightPos;
layout(location = 1) flat in vec3  lightColor;
layout(location = 2) flat in float lightRadiusInv;
layout(location = 3) flat in uvec2  lightShadowMap;
layout(location = 4) flat in vec2   lightShadowShift;

layout(location = 0) out vec4 outColor;


vec2 hashed[16] = vec2[](
    vec2(.92, .09),
    vec2(-.51,-.10),
    vec2(-.70,.22),
    vec2(.97, -.03),
    vec2(.42, .75),
    vec2(-.64,-1.0),
    vec2(-.56,.86),
    vec2(.45,-.15),
    vec2(.21,.67),
    vec2(-.46,-.77),
    vec2(-.65, .57),
    vec2(.88, -.6),
    vec2(.53,.12),
    vec2(-.71,-.08),
    vec2(-.05,.82),
    vec2(.41,-.14)
);

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a         = roughness*roughness;
    float a2        = a*a;
    float NdotH     = max(dot(N, H), 0.0);
    float NdotH2    = NdotH*NdotH;
    float nom       = a2;
    float denom     = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415926535 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) * 0.125;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

/*float sampleShadow(vec2 screenPos, float fragZ)
{
    vec2 shadowPos = screenPos - min(lightShadowShift, vec2(0.0));
    vec2 shadowSizeFactor = 1.0/(2.0/viewUbo.screenSizeFactor+abs(lightShadowShift));

    float shadowMap = texture(sampler2DArray(textures[lightShadowMap.x], samp),
                               vec3(shadowPos*shadowSizeFactor,lightShadowMap.y)).x;


    //return 1.0 - min(1.0,max(0.0, (depth-shadowDepth)*"<<0.1*PBRTextureAsset::DEPTH_BUFFER_NORMALISER_INV<<"));

    float z = viewUbo.depthOffsetAndFactor.x + fragZ * viewUbo.depthOffsetAndFactor.y;
    return 1.0 - min(1.0, max(0.0, (shadowMap - z) / (20.0*viewUbo.depthOffsetAndFactor.y)));
}

vec4 ComputeLighting(vec4 fragAlbedo, vec3 fragPos, vec4 fragNormal, vec4 fragBentNormal, vec3 fragRme)
{
    vec4 lighting = vec4(0.0);

    //if(fragAlbedo.a < .1)
      //  return lighting;

    float attenuation = 0.0;
    vec3 lightDirection = vec3(0.0);

    vec3 surfaceReflection0 = vec3(0.04);
    surfaceReflection0 = mix(surfaceReflection0, fragAlbedo.rgb, fragRme.g);

    vec3 viewDirection = normalize(pc.camPosAndZoom.xyz - fragPos);

    if(lightPos.w == 0.0)
    {
        lightDirection = -normalize(lightPos.xyz);
        attenuation = 1.0;

        if(lightShadowMap.x != 0)
        {
            vec3 v = vec3((fragPos.z/lightDirection.z)*lightDirection.xy,0.0);
            vec2 projPos;

            projPos      = gl_FragCoord.xy;
            projPos.y   -= fragPos.z * viewUbo.view[2][1];
            projPos     -= (viewUbo.view * vec4(v,0.0)).xy;

            //int h = int(gl_FragCoord.x)%4+4*(int(gl_FragCoord.y)%4);
            int h = int(fragPos.x)%4 + 4*(int(fragPos.y)%4);

            float shadowing = 1.0/6.0 * (sampleShadow(projPos, fragPos.z) * 2.0
                               + sampleShadow(projPos + hashed[h] * 6.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+1)%16] * 6.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+2)%16] * 6.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+3)%16] * 6.0, fragPos.z));

            if(shadowing > 0.2 && shadowing < 0.8)
            {
                shadowing = shadowing * 0.5 + 1.0/8.0 * (
                               + sampleShadow(projPos + hashed[(h+4)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+5)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+6)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos + hashed[(h+7)%16] * 4.0, fragPos.z));
            }

            attenuation *= shadowing * shadowing;
        }
    }
    else
    {
        lightDirection = lightPos.xyz - fragPos.xyz;
        float dist = max(length(lightDirection)*0.01,1.0);
        float dr = dist*lightRadiusInv;
        float sqrtnom = 1.0 - dr*dr*dr*dr;
        if(sqrtnom >= 0.0)
            attenuation = clamp(sqrtnom*sqrtnom/(dist*dist+1.0),0.0,1.0);

        lightDirection      = normalize(lightDirection);
    }

    if(attenuation > 0.0)
    {

        //Compute attenuation from shadows here

        float gio = abs(fragBentNormal.z);
        fragBentNormal.xyz = vec3(fragBentNormal.xy, sqrt(1.0 - fragBentNormal.x*fragBentNormal.x - fragBentNormal.y*fragBentNormal.y));

        vec3 halfwayVector  = normalize(viewDirection + lightDirection);
        float NdotL         = max(dot(fragNormal.xyz, lightDirection), 0.0);
        float BNdotL        = clamp(dot(fragBentNormal.xyz, lightDirection), 0.0, 1.0);
        //float BNdotL        = min(dot(fragBentNormal.xyz, lightDirection), 0.0);

        vec3 radiance   = attenuation*lightColor.rgb;
        float NDF   = DistributionGGX(fragNormal.xyz, halfwayVector, fragRme.r);
        float G     = GeometrySmith(fragNormal.xyz, viewDirection, lightDirection, fragRme.r);
        vec3 F      = FresnelSchlick(max(dot(halfwayVector, viewDirection), 0.0), surfaceReflection0);
        vec3 kS     = F;
        vec3 kD     = vec3(1.0) - kS;
        kD         *= 1.0 - fragRme.g;


        float ao  = fragBentNormal.a;
        ao = pow(ao, const_aoIntensity);

        gio =  1.0 - max((1.0 - BNdotL) * gio, (gio-0.5));
        gio = pow(gio, const_gioIntensity);

        float occlusion = max(min(ao, gio),fragNormal.w);  //We dont want to occlude truly transparent fragments


        vec3 nominator      = NDF * G * F;
        float denominator   = 4.0 * max(dot(fragNormal.xyz, viewDirection), 0.0) * NdotL;
        vec3 specular       = nominator / max(denominator, 0.01);
        lighting.rgb       += (kD * fragAlbedo.rgb * 0.31830988618 + specular)  * NdotL * radiance * occlusion;

        //Translucency
        float t         = fragRme.b;
        lighting.rgb   -= (kD * fragAlbedo.rgb*0.31830988618) * radiance * min(dot(fragNormal.xyz, lightDirection), 0.0)*t* occlusion;

    }

    return lighting;
}*/

float discretize(float v, uint steps)
{
    return floor(v * steps)/steps;
}

float linstep(float low, float high, float v){
    return clamp((v-low)/(high-low), 0.0, 1.0);
}

float chebyshevUpperBound(vec2 screenPos, float fragZ)
{
    vec2 shadowPos = screenPos /* - min(lightShadowShift, vec2(0.0))*/;

    shadowPos.xy *= pc.camPosAndZoom.w;

    vec2 shadowSizeFactor = 1.0/(2.0/viewUbo.screenSizeFactor+abs(lightShadowShift));

    /*vec2 moments = vec2(texture(sampler2DArray(renderedTextures[lightShadowMap.x], samp),
                               vec3((shadowPos)*shadowSizeFactor+vec2(.5,.5),lightShadowMap.y)).x,
                        texture(sampler2DArray(renderedTextures[lightSquaredShadowMap.x], samp),
                               vec3((shadowPos)*shadowSizeFactor+vec2(.5,.5),lightSquaredShadowMap.y)).x);*/

    vec2 moments = texture(sampler2DArray(renderedTextures[lightShadowMap.x], samp),
                               vec3((shadowPos)*shadowSizeFactor+vec2(.5,.5),lightShadowMap.y)).xy;

   // moments.x = 1.0-moments.x;
    float z = 1.0-(viewUbo.depthOffsetAndFactor.x + fragZ * viewUbo.depthOffsetAndFactor.y);

    /*float p = smoothstep(z-0.02, z, moments.x);
    float variance = max(moments.y - moments.x*moments.x, -0.001);
    float d = z - moments.x;
    float p_max = linstep(0.2, 1.0, variance / (variance + d*d));
    return clamp(max(p, p_max), 0.0, 1.0);*/

    float p = smoothstep(z-0.02, z, moments.x);
    float variance = max(moments.y - moments.x*moments.x, -0.001);
    float d = z - moments.x;
    float p_max = linstep(0.2, 1.0, variance / (variance + d*d));

    return clamp(max(p, p_max), 0.0, 1.0);


    /*// Surface is fully lit. as the current fragment is before the light occluder
    if (z <= moments.x)
        return 1.0 ;


    // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
    // How likely this pixel is to be lit (p_max)
    float variance = moments.y  - (moments.x*moments.x);
    variance = max(variance,0.00002);

    float d = z -moments.x;
    float p_max = variance / (variance + d*d);

    return p_max;*/
}

///float sampleShadow(vec2 screenPos, float fragZ)
//{
//    vec2 shadowPos = screenPos /* - min(lightShadowShift, vec2(0.0))*/;
//    vec2 shadowSizeFactor = 1.0/(2.0/viewUbo.screenSizeFactor+abs(lightShadowShift));
//
//    float shadowMap = texture(sampler2DArray(renderedTextures[lightShadowMap.x], samp),
//                               vec3((shadowPos)*shadowSizeFactor+vec2(.5,.5),lightShadowMap.y)).x;
//
//
//    //return 1.0 - min(1.0,max(0.0, (depth-shadowDepth)*"<<0.1*PBRTextureAsset::DEPTH_BUFFER_NORMALISER_INV<<"));
//
//    float z = viewUbo.depthOffsetAndFactor.x + fragZ * viewUbo.depthOffsetAndFactor.y;
//    return 1.0 - min(1.0, max(0.0, (shadowMap - z) / (20.0*viewUbo.depthOffsetAndFactor.y)));
//}


vec4 ComputeLighting(vec4 fragAlbedo, vec3 fragPos, vec3 fragNormal, vec3 fragRme, vec4 fragBentNormals)
{
    vec4 lighting = vec4(0.0);

    //if(fragAlbedo.a < .1)
      //  return lighting;

    float attenuation = 0.0;
    vec3 lightDirection = vec3(0.0);

    if(lightPos.w == 0.0)
    {
        lightDirection = -normalize(lightPos.xyz);
        attenuation = 1.0;


        if(lightShadowMap.x != 0)
        {
           /* vec3 v = vec3((fragPos.z/lightDirection.z)*lightDirection.xy,0.0);
            vec2 projPos;

            projPos      = gl_FragCoord.xy;
            //projPos.y   -= fragPos.z * viewUbo.view[2][1];
            projPos     -= (viewUbo.view * vec4(v,0.0)).xy;*/


            vec2 lightXYonZ = lightPos.xy / lightPos.z;
            vec4 projWorldPos  = vec4(fragPos.xy - fragPos.z*lightXYonZ,0.0,1.0);

            vec4 projPos = /** viewUbo.view* **/(projWorldPos /*- vec4(pc.camPosAndZoom.xyz,0.0)*/);
            //projPos.z = worldPos.z;

            projPos.xy -= lightShadowShift * 0.5;
            /*vec2 shadowSizeFactor = 2.0/(2.0/viewUbo.screenSizeFactor+abs(lightShadowShift));

            projPos.xyz = projPos.xyz * vec3(shadowSizeFactor, viewUbo.depthOffsetAndFactor.y)
                                + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);*/


            //int h = int(gl_FragCoord.x)%4+4*(int(gl_FragCoord.y)%4);
            ///Old PCF
            /**int h = int(fragPos.x)%7 + 3*(int(fragPos.y)%5);

            float shadowing = 1.0/6.0 * (sampleShadow(projPos.xy, fragPos.z) * 2.0
                               + sampleShadow(projPos.xy + hashed[h] * 8.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+1)%16] * 8.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+2)%16] * 8.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+3)%16] * 8.0, fragPos.z));

            if(shadowing > 0.2 && shadowing < 0.8)
            {
                shadowing = shadowing * 0.5 + 1.0/8.0 * (
                               + sampleShadow(projPos.xy + hashed[(h+4)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+5)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+6)%16] * 4.0, fragPos.z)
                               + sampleShadow(projPos.xy + hashed[(h+7)%16] * 4.0, fragPos.z));
            }

            attenuation *= shadowing * shadowing;**/

            attenuation = chebyshevUpperBound(projPos.xy, fragPos.z);
        }


    } else {
        lightDirection = lightPos.xyz - fragPos.xyz;
        float dist = max(length(lightDirection)*0.01,1.0);
        float dr = dist*lightRadiusInv;
        float sqrtnom = 1.0 - dr*dr*dr*dr;
        if(sqrtnom >= 0.0)
            attenuation = clamp(sqrtnom*sqrtnom/(dist*dist+1.0),0.0,1.0);

        lightDirection = normalize(lightDirection);
    }

    //attenuation = discretize(attenuation, 5);

    if(attenuation > 0)
    {
        vec3 radiance = attenuation*lightColor.rgb;
        vec3 viewDirection = normalize(/*pc.camPosAndZoom.xyz +*/ vec3(0.0,0.0,1000.0) - fragPos);
        vec3 surfaceReflection0 = vec3(0.04);
        surfaceReflection0 = mix(surfaceReflection0, fragAlbedo.rgb, fragRme.g);

        vec3 halfwayVector  = normalize(viewDirection + lightDirection);
        float NdotL         = max(dot(fragNormal.xyz, lightDirection), 0.0);
        float NDF   = DistributionGGX(fragNormal.xyz, halfwayVector, fragRme.r);
        float G     = GeometrySmith(fragNormal.xyz, viewDirection, lightDirection, fragRme.r);

        /*NdotL = discretize(NdotL,3);
        NDF = discretize(NDF,3);
        G = discretize(G,3);*/

        vec3 F      = FresnelSchlick(max(dot(halfwayVector,  viewDirection), 0.0), surfaceReflection0);
        vec3 kS     = F;
        vec3 kD     = vec3(1.0) - kS;
        kD         *= 1.0 - fragRme.g;
        vec3 nominator      = NDF * G * F;
        float denominator   = 4.0 * max(dot(fragNormal.xyz, viewDirection), 0.0) * NdotL;

        //denominator = discretize(denominator,3);

        //NdotL = discretize(NdotL, 3);


        //if(lightShadowMap.x == 0)
        {
            float AOBNDotL = max(dot(fragBentNormals.xyz, lightDirection), 0.0);
            float ao = fragBentNormals.w;
            ao = clamp(ao,0.9,1.0);
            ao = (ao-.9)*10.0;
            AOBNDotL = mix(AOBNDotL, NdotL, ao);
            NdotL = min(NdotL, AOBNDotL);
        }

        vec3 specular       = nominator / max(denominator, 0.01);
        lighting.rgb       += (kD * fragAlbedo.rgb * 0.31830988618 + specular)  * NdotL * radiance /* * occlusion*/;

        /*vec3 radiance = attenuation*lightColor.rgb;
        lighting.rgb += fragAlbedo.rgb * radiance;*/
    }

    return lighting;
}

void main()
{
    vec4 fragAlbedo = texture(samplerAlbedo, gl_FragCoord.xy*viewUbo.screenSizeFactor.xy*0.5);
    vec3 fragPos    = texture(samplerPosition, gl_FragCoord.xy*viewUbo.screenSizeFactor.xy*0.5).xyz;
    vec3 fragNormal = texture(samplerNormal, gl_FragCoord.xy*viewUbo.screenSizeFactor.xy*0.5).xyz;
    vec3 fragRme    = texture(samplerRme, gl_FragCoord.xy*viewUbo.screenSizeFactor.xy*0.5).xyz;
    vec4 fragBentNormals = texture(samplerBentNormals, gl_FragCoord.xy*viewUbo.screenSizeFactor.xy*0.5);
	/*fragAlbedo.rgb = pow(fragAlbedo.rgb, vec3(2.2));
    outColor = ComputeLighting(fragAlbedo, fragPos, fragNormal, fragBentNormal, fragRme);*/

    fragBentNormals.rgb = fragBentNormals.rgb*2 - vec3(1);

	fragAlbedo.rgb = pow(fragAlbedo.rgb, vec3(2.2));
    outColor = ComputeLighting(fragAlbedo, fragPos, fragNormal, fragRme, fragBentNormals);

}

