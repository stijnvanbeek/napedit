#pragma once

#include <nap/resource.h>
#include <nap/resourceptr.h>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace nap
{

    struct TestStruct
    {
        int mInt = 0;
        float mFloat = 0.f;
        std::string mString = "";
    };

    enum class TestEnum
    {
        EEN, TWEE, DRIE
    };


    class TestResource : public Resource
    {
        RTTI_ENABLE(Resource)

    public:
        TestResource() = default;

        TestStruct mStruct;
        TestEnum mEnum = TestEnum::EEN;
        std::vector<int> mVector = { 1, 2, 3 };
        ResourcePtr<TestResource> mPointer;
        ResourcePtr<TestResource> mEmbeddedPointer;
        std::vector<ResourcePtr<TestResource>> mPointerVector;
        std::vector<ResourcePtr<TestResource>> mEmbeddedPointerVector;
        std::vector<TestStruct> mObjectVector;
        std::array<int, 4> mArray;
        glm::vec2 mVec2;
        glm::vec3 mVec3;

    private:
    };
    
}
