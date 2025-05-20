#include "model.h"

#include "nap/group.h"
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::Model)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {


        template <typename ClassType, typename MemberType>
            MemberType* getRawMemberPointer(ClassType& instance, MemberType ClassType::*memberPointer)
        {
            return &(instance.*memberPointer);
        }


        std::string Model::createResource(const rttr::type& resourceType, const std::string& aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(resourceType);
            auto resource = std::unique_ptr<Resource>(rtti_cast<Resource>(object));
            assert(resource != nullptr);
            auto typeName = resourceType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;

            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                resource->mID = mID;
                mResources.emplace_back(std::move(resource));
                mTree.mResources.emplace_back(mResources.back().get());
                return mResources.back()->mID;
            }
            return "";
        }



        std::string Model::createGroup(const rttr::type &groupType, const std::string &aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(groupType);
            auto group = rtti_cast<IGroup>(object);
            assert(group != nullptr);
            auto typeName = groupType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;
            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                group->mID = mID;
                auto groupPtr = std::unique_ptr<Resource>(group);
                mResources.emplace_back(std::move(groupPtr));
                mTree.mGroups.emplace_back(static_cast<ResourceGroup*>(group));
                return mID;
            }
            return "";
        }


        void Model::moveResourceToParent(const std::string &mID, const std::string &parentGroupID)
        {
            auto resource = rtti_cast<Resource>(findResource(mID));
            assert(resource != nullptr);
            bool found = eraseFromTree(*resource);
            assert(found);
            auto group = static_cast<ResourceGroup*>(findResource(parentGroupID));
            if (group != nullptr)
                group->mMembers.emplace_back(resource);
            else
                mTree.mResources.emplace_back(resource);
        }


        void Model::moveGroupToParent(const std::string &groupID, const std::string &parentGroupID)
        {
            auto group = rtti_cast<ResourceGroup>(findResource(groupID));
            assert(group != nullptr);
            auto found = eraseFromTree(*group);
            assert(found);
            auto parent = rtti_cast<ResourceGroup>(findResource(parentGroupID));
            if (parent != nullptr)
                parent->mChildren.emplace_back(group);
            else
                mTree.mGroups.emplace_back(group);
        }


        void Model::removeResource(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            assert(it != mResources.end());
            auto resource = it->get();
            bool found = eraseFromTree(*resource);
            assert(found);
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

            return nullptr;
        }


        ResourceGroup* Model::findParent(const std::string &mID)
        {
            auto it = std::find_if(mTree.mGroups.begin(), mTree.mGroups.end(), [&](auto& group){ return group->mID == mID; });
            if (it != mTree.mGroups.end())
                return nullptr;
            return (*it).get();
        }


        bool Model::eraseFromTree(std::vector<ResourcePtr<Resource>>& branch, Object &object)
        {;
            auto it = std::find(branch.begin(), branch.end(), &object);
            if (it != branch.end())
            {
                branch.erase(it);
                return true;
            }

            for (auto& element : branch)
            {
                auto group = rtti_cast<ResourceGroup>(element.get());
                if (group != nullptr)
                {
                    if (eraseFromTree(group->mMembers, object))
                        return true;
                    if (eraseFromTree(group->mChildren, object))
                        return true;
                }
            }

            return false;
        }


        bool Model::eraseFromTree(std::vector<ResourcePtr<ResourceGroup>> &branch, Object &resource)
        {
            auto it = std::find(branch.begin(), branch.end(), rtti_cast<Resource>(&resource));
            if (it != branch.end())
            {
                branch.erase(it);
                return true;
            }
            for (auto& element : branch)
            {
                if (eraseFromTree(element->mMembers, resource))
                    return true;
                if (eraseFromTree(element->mChildren, resource))
                    return true;
            }
            return false;
        }


        bool Model::eraseFromTree(Object &resource)
        {
            return eraseFromTree(mTree.mResources, resource) || eraseFromTree(mTree.mGroups, resource);
        }


        bool Model::findInTree(const std::string &mID, std::vector<ResourcePtr<Resource>> &branch)
        {
            auto it = std::find_if(branch.begin(), branch.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            if (it != branch.end())
                return true;
            return false;
        }

        
        bool Model::findParentInTree(const std::string &mID, std::vector<ResourcePtr<ResourceGroup>> &branch)
        {

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