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
            Model(Core& core) : mCore(core) { }

            void addResource(const rttr::type& resourceType, const std::string& mID = "");
            void addGroup(const std::string& groupID) { addResource(RTTI_OF(ResourceGroup), groupID); }
            void addResourceToGroup(const std::string& groupID, const rttr::type& resourceType, const std::string& mID = "");
            void addGroupToParent(const std::string& newGroupID, const std::string& parentGroupID);
            void removeResource(const std::string& mID);
            const std::map<std::string, std::unique_ptr<Resource>>& getResources() const { return mResources; }

        private:
        	std::map<std::string, std::unique_ptr<Resource>> mResources;
            std::vector<std::function<void()>> mUndoHistory;
            Core& mCore;
        };

    }

}
