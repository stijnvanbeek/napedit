#pragma once

#include <model.h>
#include <Gui/Action.h>

namespace nap
{
    namespace edit
    {
    
        class ActionController : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            ActionController() = default;

            ResourcePtr<gui::Action> mNewAction;
            ResourcePtr<gui::Action> mOpenAction;
            ResourcePtr<gui::Action> mSaveAction;
            ResourcePtr<gui::Action> mSaveAsAction;

            ResourcePtr<Selector> mSelector;

            bool init(utility::ErrorState& errorState) override;
            
        private:
            Slot<gui::Action&> mNewActionSlot = { this, &ActionController::onNewAction };
            void onNewAction(gui::Action&);
            Slot<gui::Action&> mOpenActionSlot = { this, &ActionController::onOpenAction };
            void onOpenAction(gui::Action&);
            Slot<gui::Action&> mSaveActionSlot = { this, &ActionController::onSaveAction };
            void onSaveAction(gui::Action&);
            Slot<gui::Action&> mSaveAsActionSlot = { this, &ActionController::onSaveAsAction };
            void onSaveAsAction(gui::Action&);

            std::string mPath;
            Model* mModel = nullptr;
        };
    
    }
}