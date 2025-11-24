#pragma once

#include <imgui.h>

namespace nap
{
    namespace edit
    {
        /**
         * Displays a popup menu with a search filter.
         */
        class FilteredMenu
        {

        public:
            FilteredMenu() = default;

            /**
             * Initialize the menu
             * @param items List of items to display in the menu.
             */
            void init(const std::vector<std::string>&& items);

            /**
             * Show the menu
             * @return True when an item is selected, false if not
             */
            bool show();

            /**
             * Get the selected item during the last show() call.
             * @return The selected item, or an empty string if no item is selected.
             */
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