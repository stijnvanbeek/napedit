#pragma once

#include <nap/resource.h>
#include <imgui.h>
#include <vector>
#include <memory>

namespace nap
{
    namespace edit
    {

        class ResourceMenu
        {
        public:
            void init(const std::vector<std::unique_ptr<Resource>>& resources, rtti::TypeInfo* derivedFrom, std::set<rtti::Object*> exclude = { });
            bool show();
            Resource* getSelectedResource() const { return mSelectedResource; }
            const std::string& getSelectedResourceID() const { return mSelectedResourceID; }

        private:
            std::vector<Resource*> mFilteredResources;
            std::vector<Resource*> mResources;
            char mSearchFilter[128];
            Resource* mSelectedResource = nullptr;
            int mSelectedResourceIndex = -1;
            std::string mSelectedResourceID;
            bool mFirstShow = true;
        };

    } // edit

}
