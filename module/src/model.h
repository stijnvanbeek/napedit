#pragma once

#include <nap/resource.h>
#include <nap/core.h>

#include "nap/group.h"

namespace nap
{

    namespace edit
    {

        class NAPAPI Model : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            Model(Core& core) : mCore(core) { mTree.mID = "Resources"; }

            void createResource(const rttr::type& resourceType, const std::string& mID = "");
            void createGroup(const rttr::type& groupType, const std::string& groupID = "");
            void moveResourceToParent(const std::string& mID, const std::string& parentGroupID);
            void moveGroupToParent(const std::string& groupID, const std::string& parentGroupID);
            void removeResource(const std::string& mID);
            void renameResource(const std::string& mID, const std::string& newName);
            const std::vector<std::unique_ptr<Resource>>& getResources() const { return mResources; }
            Resource* findResource(const std::string& mID);
            ResourceGroup& getTree() { return mTree; }

        private:
            ResourceGroup* eraseFromTree(ResourceGroup& branch, Resource& resource);
            ResourceGroup* eraseGroupFromTree(ResourceGroup& branch, ResourceGroup& group);
            std::string getUniqueID(const std::string& baseID);

        	std::vector<std::unique_ptr<Resource>> mResources;
            ResourceGroup mTree;
            Core& mCore;
        };

    }

}
