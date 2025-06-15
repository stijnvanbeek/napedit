#include "model.h"

#include "nap/group.h"
#include <utility/stringutils.h>

#include "entity.h"

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


        bool Model::init(utility::ErrorState &errorState)
        {
            auto groupBase = RTTI_OF(IGroup);
            auto allGroups = groupBase.get_derived_classes();
            for (auto &group: allGroups)
                if (mCore.getResourceManager()->getFactory().canCreate(group))
                    mGroupTypes[group.get_name().to_string()] = &group;

            auto resourceBase = RTTI_OF(Resource);
            auto allResources = resourceBase.get_derived_classes();
            for (auto &resource: allResources)
                if (mCore.getResourceManager()->getFactory().canCreate(resource))
                    if (mGroupTypes.find(resource.get_name().to_string()) == mGroupTypes.end())
                        mResourceTypes[resource.get_name().to_string()] = &resource;

            return true;
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
            // Create the group
            auto object = mCore.getResourceManager()->getFactory().create(groupType);
            auto group = rtti_cast<IGroup>(object);
            assert(group != nullptr);
            auto groupPtr = std::unique_ptr<Resource>(group);

            // Generate the ID
            auto typeName = groupType.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;

            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                // Add to mResources and to mTree
                groupPtr->mID = mID;
                mResources.emplace_back(std::move(groupPtr));
                mTree.mGroups.emplace_back(static_cast<ResourceGroup*>(group));
                return mID;
            }

            return "";
        }


        std::string Model::createEntity(const std::string &aID)
        {
            std::string mID = "Entity";
            if (!aID.empty())
                mID = aID;
            mID = getUniqueID(mID);
            if (mID.empty())
                return "";

            auto entity = std::make_unique<Entity>();
            entity->mID = mID;
            mResources.emplace_back(std::move(entity));
            mTree.mEntities.emplace_back(dynamic_cast<Entity*>(mResources.back().get()));
            return mID;
        }


        std::string Model::createComponent(const rttr::type &componentType, const std::string &entityID,
            const std::string &componentID)
        {
            auto entity = findResource<Entity>(entityID);
            assert(entity != nullptr);
            auto object = mCore.getResourceManager()->getFactory().create(componentType);
            auto component = rtti_cast<Component>(object);
            assert(component != nullptr);
            auto componentPtr = std::unique_ptr<Component>(component);

            auto typeName = componentType.get_name().to_string();
            auto mID = typeName;
            if (!componentID.empty())
                mID = componentID;
            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                componentPtr->mID = mID;
                entity->mComponents.emplace_back(rtti_cast<Component>(componentPtr.get()));
                mResources.emplace_back(std::move(componentPtr));
                return mID;
            }
            return "";
        }


        Resource* Model::createObject(const rttr::type &type, const std::string &aID)
        {
            auto object = mCore.getResourceManager()->getFactory().create(type);
            auto resource = std::unique_ptr<Resource>(rtti_cast<Resource>(object));
            assert(resource != nullptr);
            auto typeName = type.get_name().to_string();
            auto mID = typeName;
            if (!aID.empty())
                mID = aID;

            mID = getUniqueID(mID);
            if (!mID.empty())
            {
                resource->mID = mID;
                mResources.emplace_back(std::move(resource));
                return mResources.back().get();
            }
            return nullptr;
        }


        void Model::removeEmbeddedObject(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            assert(it != mResources.end());

            // Make sure it's not in the tree
            auto resource = it->get();
            bool found = eraseFromTree(*resource);
            assert(!found);
            // Remove from the owned resources list
            mResources.erase(it);
        }


        void Model::moveResourceToGroup(const std::string &mID, const std::string &groupID)
        {
            auto resource = findResource(mID);
            assert(resource != nullptr);
            bool found = eraseFromTree(*resource);
            assert(found);
            auto group = findGroup(groupID);
            if (group != nullptr)
                group->mMembers.emplace_back(resource);
            else
                mTree.mResources.emplace_back(resource);
        }


        void Model::moveGroupToParent(const std::string &groupID, const std::string &parentGroupID)
        {
            auto group = findGroup(groupID);
            assert(group != nullptr);
            auto found = eraseFromTree(*group);
            assert(found);
            auto parent = findGroup(parentGroupID);
            if (parent != nullptr)
            {
                parent->mChildren.emplace_back(group);
            }
            else
                mTree.mGroups.emplace_back(group);
        }


        void Model::moveEntityToParent(const std::string &entityID, const std::string &parentID)
        {
            auto entity = findResource<Entity>(entityID);
            assert(entity != nullptr);
            auto found = eraseFromTree(*entity);
            assert(found);
            auto parent = findResource<Entity>(parentID);
            if (parent != nullptr)
            {
                parent->mChildren.emplace_back(entity);
            }
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


        ResourceGroup* Model::findGroup(const std::string &mID)
        {
            auto resource = findResource(mID);
            if (resource == nullptr)
                return nullptr;
            auto group = rtti_cast<IGroup>(resource);
            if (group != nullptr)
                return static_cast<ResourceGroup*>(group);
            return nullptr;
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


        bool Model::eraseFromTree(std::vector<ResourcePtr<Entity>> &branch, Object &resource)
        {
            auto it = std::find(branch.begin(), branch.end(), rtti_cast<Resource>(&resource));
            if (it != branch.end())
            {
                branch.erase(it);
                return true;
            }
            for (auto& element : branch)
            {
                if (eraseFromTree(element->mChildren, resource))
                    return true;
                auto it = std::find(element->mComponents.begin(), element->mComponents.end(), rtti_cast<Resource>(&resource));
                if (it != element->mComponents.end())
                {
                    element->mComponents.erase(it);
                    return true;
                }
            }
            return false;
        }


        bool Model::eraseFromTree(Object &resource)
        {
            return eraseFromTree(mTree.mResources, resource) || eraseFromTree(mTree.mGroups, resource) || eraseFromTree(mTree.mEntities, resource);
        }


        bool Model::findInTree(const std::string &mID, std::vector<ResourcePtr<Resource>> &branch)
        {
            auto it = std::find_if(branch.begin(), branch.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            if (it != branch.end())
                return true;
            return false;
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