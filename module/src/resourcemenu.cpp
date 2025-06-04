#include "resourcemenu.h"

namespace nap
{
    namespace edit
    {

        void ResourceMenu::init(const std::vector<std::unique_ptr<Resource>>& resources, rtti::TypeInfo* derivedFrom)
        {
            mResources.clear();
            for (const auto& resource : resources)
            {
                auto resourceType = resource->get_type().get_raw_type();
                if (derivedFrom == nullptr || resourceType.is_derived_from(*derivedFrom) || resourceType == *derivedFrom)
                    mResources.emplace_back(resource.get());
            }

            mSelectedResource = nullptr;
            mSelectedResourceIndex = -1;
            mSelectedResourceID.clear();
            mFirstShow = true;
            std::memset(mSearchFilter, 0, sizeof(mSearchFilter));
        }


        struct ResourceInputCallbackData
        {
            std::vector<Resource*>* mFilteredResources;
            std::vector<Resource*>* mResources;
            char* mSearchFilter;
        };


        int ResourceInputCallBack(ImGuiInputTextCallbackData* callbackData)
        {
            ResourceInputCallbackData* data = (ResourceInputCallbackData*)callbackData->UserData;
            data->mFilteredResources->clear();
            for (auto& resource : *data->mResources)
                if (utility::contains(resource->mID, data->mSearchFilter, false))
                    data->mFilteredResources->emplace_back(resource);
            return 0;
        }

        
        bool ResourceMenu::show()
        {
            bool result = false;

            // Search filter input
            ResourceInputCallbackData data = { &mResources, &mFilteredResources, mSearchFilter };
            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter), ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_EnterReturnsTrue, ResourceInputCallBack, &data))
            {
                if (mFilteredResources.size() == 1)
                {
                    mSelectedResourceIndex = 0;
                    mSelectedResource = mFilteredResources[0];
                    mSelectedResourceID = mSelectedResource->mID;
                    ImGui::CloseCurrentPopup();
                    result = true;
                }
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
            ImGui::PushAllowKeyboardFocus(true);
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
            ImGui::PopAllowKeyboardFocus();


            return result;
        }


    } // edit
}
