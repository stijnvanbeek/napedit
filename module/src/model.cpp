#include "model.h"

#include "nap/group.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::Model)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        void Model::addResource(const rttr::type& resourceType, const std::string& aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(resourceType);
            auto resource = rtti_cast<Resource>(object);
            assert(resource != nullptr);
            auto typeName = resourceType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;
            int idCounter = 2;
            while (mResources.find(mID) != mResources.end())
            {
                mID = typeName + std::to_string(idCounter);
                idCounter++;
            }
            resource->mID = mID;
            mResources[resource->mID] = std::unique_ptr<Resource>(resource);
        }


        void Model::addResourceToGroup(const std::string &groupID, const rttr::type &resourceType,
            const std::string &mID)
        {
            addResource(resourceType, mID);
            auto resource = rtti_cast<Resource>(mResources[mID].get());
            auto group = rtti_cast<ResourceGroup>(mResources[groupID].get());
            assert(resource != nullptr);
            assert(group != nullptr);
            group->mMembers.emplace_back(resource);
        }


        void Model::addGroupToParent(const std::string &newGroupID, const std::string &parentGroupID)
        {
            addResource(RTTI_OF(ResourceGroup), newGroupID);
            auto newGroup = rtti_cast<ResourceGroup>(mResources[newGroupID].get());
            auto destGroup = rtti_cast<ResourceGroup>(mResources[parentGroupID].get());
            assert(destGroup != nullptr);
            assert(newGroup != nullptr);
            destGroup->mChildren.emplace_back(newGroup);
        }


        void Model::removeResource(const std::string &mID)
        {
            mResources.erase(mID);
        }
    }

}