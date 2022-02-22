#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIds;
layout(location = 6) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec4 varyingColor;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 localNormal = vec3(0.0f);

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(aBoneIds[i] == -1)
        continue;
        if(aBoneIds[i] >= MAX_BONES)
        {
            totalPosition = vec4(aPos, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * aWeights[i];
        localNormal = mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;

    }
    mat4 BoneTransform = mat4(0.0f);
//
//    BoneTransform  = finalBonesMatrices[aBoneIds[0]] * aWeights[0];
//    BoneTransform += finalBonesMatrices[aBoneIds[1]] * aWeights[1];
//    BoneTransform += finalBonesMatrices[aBoneIds[2]] * aWeights[2];
//    BoneTransform += finalBonesMatrices[aBoneIds[3]] * aWeights[3];

    mat3 normalMatrix = transpose(inverse(mat3(BoneTransform)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);

    vs_out.TBN = mat3(T, B, N);
    vs_out.FragPos = vec3(model * totalPosition);
    vs_out.Normal =	N;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    varyingColor = vec4(vs_out.FragPos, 1.0) * 0.5 + vec4(0.5, 0.5, 0.5, 0.5);
    mat4 viewModel = view * model;
    gl_Position = projection * view * model * totalPosition;

}