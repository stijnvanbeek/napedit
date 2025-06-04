#pragma once

#include <imgui.h>

namespace nap
{
    namespace edit
    {
    
        class FilteredMenu
        {

        public:
            FilteredMenu() = default;
            void init(const std::vector<std::string>&& items);
            bool show();
            const std::string& getSelectedItem() const { return mSelectedItem; }

        private:
            std::vector<std::string> mItems;
            std::vector<std::string> mFilteredItems;
            char mSearchFilter[128];
            int mSelectedItemIndex = -1;
            std::string mSelectedItem;
            bool mFirstShow = true;
        };
    
    }
}