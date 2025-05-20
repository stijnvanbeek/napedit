#pragma once

#include <nap/resource.h>
#include <imgui.h>

namespace nap
{
    namespace edit
    {

        class TypeMenu
        {
        public:
            void init(const std::map<std::string, const rtti::TypeInfo*>& types, rtti::TypeInfo* derivedFrom = nullptr);
            bool show();
            const rtti::TypeInfo* getSelectedType() const { return mSelectedType; }
            const std::string& getSelectedTypeID() const { return mSelectedTypeID; }

        private:
            std::map<std::string, const rtti::TypeInfo*> mTypes;
            std::map<std::string, const rtti::TypeInfo*> mFilteredTypes;
            char mSearchFilter[128];
            const rtti::TypeInfo* mSelectedType = nullptr;
            int mSelectedTypeIndex = -1;
            std::string mSelectedTypeID;
        };

    } // edit

}
