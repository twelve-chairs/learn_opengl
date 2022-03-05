#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Bone.h"
#include <functional>
#include "ModelAnimation.h"


struct AssimpNodeData
{
    glm::mat4 transformation{};
    std::string name;
    int childrenCount{};
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    float m_Duration{};
    float m_TicksPerSecond{};
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    Animation() = default;

    Animation(const std::string &animationPath, ModelAnimation *model){
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        if (scene->mAnimations) {
            auto animation = scene->mAnimations[0];
            m_Duration = static_cast<float>(animation->mDuration);
            m_TicksPerSecond = static_cast<float>(animation->mTicksPerSecond);
            aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
            globalTransformation = globalTransformation.Inverse();
            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            ReadMissingBones(animation, *model);
        }
    }

    ~Animation()= default;

    Bone *FindBone(const std::string &name){
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                                 [&](const Bone& Bone)
                                 {
                                     return Bone.GetBoneName() == name;
                                 }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }


    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
    inline float GetDuration() const { return m_Duration;}
    inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    inline const std::map<std::string, BoneInfo> &GetBoneIDMap() const{
        return m_BoneInfoMap;
    }

private:
    void ReadMissingBones(const aiAnimation* animation, ModelAnimation& model){
        unsigned int size = animation->mNumChannels;

        auto &boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from ModelAnimation class
        int &boneCount = model.GetBoneCount(); //getting the m_BoneCounter from ModelAnimation class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++){
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end()){
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.emplace_back(Bone(channel->mNodeName.data,
                                   boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src){
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++){
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
};
