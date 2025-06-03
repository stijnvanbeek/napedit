#include "resourcemenu.h"

namespace nap
{
    namespace edit
    {

        void ResourceMenu::init(const std::vector<std::unique_ptr<Resource>>& resources, rtti::TypeInfo* derivedFrom, std::set<rtti::Object*> exclude)
        {
            mResources.clear();
            for (const auto& resource : resources)
            {
                auto it = exclude.find(resource.get());
                if (it == exclude.end())
                {
                    auto resourceType = resource->get_type().get_raw_type();
                    if (derivedFrom == nullptr || resourceType.is_derived_from(*derivedFrom) || resourceType == *derivedFrom)
                        mResources.emplace_back(resource.get());
                }
            }

            mSelectedResource = nullptr;
            mSelectedResourceIndex = -1;
            mSelectedResourceID.clear();
            mFirstShow = true;
            std::memset(mSearchFilter, 0, sizeof(mSearchFilter));
        }

        
        bool ResourceMenu::show()
        {
            bool result = false;

            // Search filter input
            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter)))
            {
                mFilteredResources.clear();
                for (const auto& resource : mResources)
                    if (utility::contains(resource->mID, mSearchFilter, false))
                        mFilteredResources.emplace_back(resource);
            }
            if (mFirstShow)
            {
                ImGui::SetKeyboardFocusHere();
                mFirstShow = false;
            }

            // List with all available resource types filtered
            auto& showedList = mFilteredResources;
            if (std::string(mSearchFilter).empty())
                showedList = mResources;

            // Display filtered resources
            if (ImGui::ListBoxHeader("##ResourceList", showedList.size()))
            {
                for (auto i = 0; i < showedList.size(); ++i)
                {
                    bool isSelected = (mSelectedResourceIndex == i);
                    if (ImGui::Selectable(showedList[i]->mID.c_str(), isSelected))
                    {
                        mSelectedResourceIndex = i;
                        mSelectedResource = showedList[i];
                        mSelectedResourceID = mSelectedResource->mID;
                        ImGui::CloseCurrentPopup();
                        result = true;
                    }

                    if (showedList.empty())
                        ImGui::Selectable("Nothing found", false, ImGuiSelectableFlags_Disabled);

                    // if (isSelected)
                    //     ImGui::SetItemDefaultFocus();
                }
                ImGui::ListBoxFooter();
            }

            return result;
        }


    } // edit
}
