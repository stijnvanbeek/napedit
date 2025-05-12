#include "model.h"

#include "nap/group.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::Model)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        void Model::createResource(const rttr::type& resourceType, const std::string& aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(resourceType);
            auto resource = rtti_cast<Resource>(object);
            assert(resource != nullptr);
            auto typeName = resourceType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;

            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                resource->mID = mID;
                mResources.emplace_back(std::unique_ptr<Resource>(resource));
                mTree.mMembers.emplace_back(resource);
            }
        }



        void Model::createGroup(const rttr::type &groupType, const std::string &aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(groupType);
            auto group = rtti_cast<ResourceGroup>(object);
            assert(group != nullptr);
            auto typeName = groupType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;
            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                group->mID = mID;
                mResources.emplace_back(std::unique_ptr<ResourceGroup>(group));
                mTree.mChildren.emplace_back(group);
            }
        }


        void Model::moveResourceToParent(const std::string &mID, const std::string &parentGroupID)
        {
            auto resource = rtti_cast<Resource>(findResource(mID));
            assert(resource != nullptr);
            auto oldGroup = eraseFromTree(mTree, *resource);
            assert(oldGroup != nullptr);
            auto group = rtti_cast<ResourceGroup>(findResource(parentGroupID));
            if (group != nullptr)
                group->mMembers.emplace_back(resource);
            else
                mTree.mMembers.emplace_back(resource);
        }


        void Model::moveGroupToParent(const std::string &groupID, const std::string &parentGroupID)
        {
            auto group = rtti_cast<ResourceGroup>(findResource(groupID));
            assert(group != nullptr);
            auto oldParent = eraseGroupFromTree(mTree, *group);
            assert(oldParent != nullptr);
            auto parent = rtti_cast<ResourceGroup>(findResource(parentGroupID));
            if (parent != nullptr)
                parent->mChildren.emplace_back(group);
            else
                mTree.mChildren.emplace_back(group);
        }


        void Model::removeResource(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            assert(it != mResources.end());
            auto resource = it->get();
            auto oldGroup = eraseFromTree(mTree, *resource);
            assert(oldGroup != nullptr);
            mResources.erase(it);
        }


        void Model::renameResource(const std::string &mID, const std::string &aNewName)
        {
            auto resource = findResource(mID);
            assert(resource != nullptr);
            auto newName = getUniqueID(aNewName);
            if (!newName.empty())
                resource->mID = newName;
        }


        Resource* Model::findResource(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            if (it != mResources.end())
                return it->get();
            else
                return nullptr;
        }


        ResourceGroup* Model::eraseFromTree(ResourceGroup& branch, Resource &resource)
        {
            auto it = std::find(branch.mMembers.begin(), branch.mMembers.end(), &resource);
            if (it != branch.mMembers.end())
            {
                branch.mMembers.erase(it);
                return &branch;
            }

            for (auto& group : branch.mChildren)
            {
                auto parent = eraseFromTree(*group, resource);
                if (parent != nullptr)
                    return parent;
            }

            return nullptr;
        }


        ResourceGroup* Model::eraseGroupFromTree(ResourceGroup &branch, ResourceGroup &group)
        {
            auto it = std::find(branch.mChildren.begin(), branch.mChildren.end(), &group);
            if (it != branch.mChildren.end())
            {
                branch.mChildren.erase(it);
                return &branch;
            }

            for (auto& child : branch.mChildren)
            {
                auto parent = eraseGroupFromTree(*child, group);
                if (parent != nullptr)
                    return parent;
            }

            return nullptr;
        }


        std::string Model::getUniqueID(const std::string &aBaseID)
        {
            auto baseID = aBaseID;
            baseID = utility::replaceAllInstances(utility::trim(baseID), " ", "_");
            int idCounter = 2;
            auto mID = baseID;
            while (findResource(mID) != nullptr)
            {
                mID = baseID + std::to_string(idCounter);
                idCounter++;
            }
            return mID;
        }

    }

}