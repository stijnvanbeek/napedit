#include "filteredmenu.h"


namespace nap
{

    namespace edit
    {

        void FilteredMenu::init(const std::vector<std::string> &&items)
        {
            mItems = std::move(items);
            mFilteredItems.clear();

            mSelectedItem.clear();
            mSelectedItemIndex = 0;
            mSearchFilter[0] = '\0';

            mFirstShow = true;
        }


        struct FilterInputCallbackData
        {
            std::vector<std::string>* mItems = nullptr;
            std::vector<std::string>* mFilteredItems = nullptr;
            char* mSearchFilter;
        };


        int FilterInputCallBack(ImGuiInputTextCallbackData* callbackData)
        {
            FilterInputCallbackData* data = (FilterInputCallbackData*)callbackData->UserData;
            data->mFilteredItems->clear();
            for (auto& item : *data->mItems)
                if (utility::contains(item, data->mSearchFilter, false))
                    data->mFilteredItems->emplace_back(item);
            return 0;
        }


        bool FilteredMenu::show()
        {
            bool result = false;

            // Filter the available items using input text
            FilterInputCallbackData data = { &mItems, &mFilteredItems, mSearchFilter };

            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter), ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_EnterReturnsTrue, FilterInputCallBack, &data))
            {
                if (mFilteredItems.size() == 1)
                {
                    mSelectedItemIndex = 0;
                    mSelectedItem = *mFilteredItems.begin();
                    ImGui::CloseCurrentPopup();
                    result = true;
                }
            }
            if (mFirstShow)
            {
                ImGui::SetKeyboardFocusHere();
                mFirstShow = false;
            }

            // List with all available items filtered
            auto& showedMap = mFilteredItems;
            if (std::string(mSearchFilter).empty())
                showedMap = mItems;

            if (ImGui::ListBoxHeader("##FilteredItemsListBox", showedMap.size(), 20))
            {
                auto item = showedMap.begin();
                for (int i = 0; i < showedMap.size(); ++i)
                {
                    if (ImGui::Selectable(item->c_str()))
                    {
                        mSelectedItemIndex = i;
                        mSelectedItem = *item;
                        ImGui::CloseCurrentPopup();
                        result = true;
                    }
                    item++;
                }

                if (showedMap.empty())
                    ImGui::Selectable("Nothing found", false, ImGuiSelectableFlags_Disabled);

                ImGui::ListBoxFooter();
            }

            return result;
        }

    }

}