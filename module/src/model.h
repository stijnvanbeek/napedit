#pragma once

#include <nap/resource.h>
#include <nap/core.h>

#include "entity.h"
#include "nap/group.h"

namespace nap
{

    namespace edit
    {

        class NAPAPI Model : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            struct Tree
            {
                std::vector<ResourcePtr<Resource>> mResources;
                std::vector<ResourcePtr<ResourceGroup>> mGroups;
                std::vector<ResourcePtr<Entity>> mEntities;
            };

            Model(Core& core) : mCore(core) { }

            bool init(utility::ErrorState &errorState) override;

            std::string createResource(const rttr::type& resourceType, const std::string& mID = "");
            std::string createGroup(const rttr::type& groupType, const std::string& groupID = "");
            std::string createEntity(const std::string& id = "");
            std::string createComponent(const rttr::type& componentType, const std::string& entityID, const std::string& componentID = "");
            Resource* createObject(const rttr::type& type, const std::string& mID = ""); // Creates object without adding it to the tree
            void removeEmbeddedObject(const std::string& mID = ""); // Removes without deleting from the tree
            void moveResourceToGroup(const std::string& mID, const std::string& groupID);
            void moveGroupToParent(const std::string& groupID, const std::string& parentGroupID);
            void moveEntityToParent(const std::string& entityID, const std::string& parentID);
            void removeResource(const std::string& mID);
            void renameResource(const std::string& mID, const std::string& newName);
            const std::vector<std::unique_ptr<Resource>>& getResources() const { return mResources; }

            template <typename T> T* findResource(const std::string& mID);
            Resource* findResource(const std::string& mID);
            ResourceGroup* findGroup(const std::string& mID);
            Tree& getTree() { return mTree; }

            const std::map<std::string, const rtti::TypeInfo*>& getResourceTypes() const { return mResourceTypes; }
            const std::map<std::string, const rtti::TypeInfo*>& getGroupTypes() const { return mGroupTypes; }

        private:
            bool eraseFromTree(std::vector<ResourcePtr<Resource>>& branch, Object& resource);
            bool eraseFromTree(std::vector<ResourcePtr<ResourceGroup>>& branch, Object& resource);
            bool eraseFromTree(std::vector<ResourcePtr<Entity>>& branch, Object& resource);
            bool eraseFromTree(Object& resource);

            bool findInTree(const std::string& mID, std::vector<ResourcePtr<Resource>>& branch);

            std::string getUniqueID(const std::string& baseID);

        	std::vector<std::unique_ptr<Resource>> mResources;
            Tree mTree;

            std::map<std::string, const rtti::TypeInfo*> mResourceTypes;
            std::map<std::string, const rtti::TypeInfo*> mGroupTypes;

            Core& mCore;
        };


        template<typename T>
        T * Model::findResource(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            if (it != mResources.end())
            {
                auto result = rtti_cast<T>(it->get());
                if (result != nullptr)
                    return result;
                else
                    return nullptr;
            }
            return nullptr;
        }

    }

}
