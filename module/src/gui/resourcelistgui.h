#pragma once

#include <Gui/Gui.h>
#include <model.h>
#include <nap/core.h>

namespace nap
{
    namespace edit
    {
    
        class ResourceListGui : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            ResourceListGui(Core& core);

            ResourcePtr<edit::Model> mModel; ///< Property: 'Model'

            // Inherited
            bool init(utility::ErrorState& errorState) override;

            void addResourcePopup();
            
        private:
            void draw() override;

            std::map<std::string, const rtti::TypeInfo*> mTypes;
            std::map<std::string, const rtti::TypeInfo*> mFilteredTypes;
            int mSelectedType = -1;
            char mSearchFilter[128];

            std::vector<ResourcePtr<Resource>> mTree;
            std::set<std::string> mExpandedIDs;

            std::string mSelectedID;
            Core& mCore;
        };
    
    }
}