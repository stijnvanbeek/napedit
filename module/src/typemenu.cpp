#include "typemenu.h"

namespace nap
{

    namespace edit
    {

        void TypeMenu::init(const std::map<std::string, const rtti::TypeInfo *> &types, rtti::TypeInfo *derivedFrom)
        {
            mTypes.clear();
            if (derivedFrom == nullptr)
                mTypes = types;
            else {
                for (auto it = types.begin(); it != types.end(); ++it)
                    if (it->second->is_derived_from(*derivedFrom))
                        mTypes[it->first] = it->second;
            }
            mSelectedType = nullptr;
            mSelectedTypeID.clear();
            mSelectedTypeID = -1;
            mFilteredTypes.clear();
        }


        bool TypeMenu::show()
        {
            bool result = false;

            // Filter the available resource types
            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter)))
            {
                mFilteredTypes.clear();
                for (auto& pair : mTypes)
                    if (utility::contains(pair.first, mSearchFilter))
                        mFilteredTypes[pair.first] = pair.second;
            }

            // List with all available resource types filtered
            auto& showedMap = mFilteredTypes;
            if (showedMap.empty())
                showedMap = mTypes;

            if (ImGui::ListBoxHeader("##TypesListBox", showedMap.size(), 20))
            {
                auto resourceTypePair = showedMap.begin();
                for (int i = 0; i < showedMap.size(); ++i)
                {
                    if (ImGui::Selectable(resourceTypePair->first.c_str(), i == mSelectedTypeIndex))
                    {
                        mSelectedTypeIndex = i;
                        mSelectedType = resourceTypePair->second;
                        mSelectedTypeID = resourceTypePair->first;
                        ImGui::CloseCurrentPopup();
                        result = true;
                    }
                    resourceTypePair++;
                }
                ImGui::ListBoxFooter();
            }

            return result;
        }

    }

}