#pragma once

#include <model.h>
#include <controller.h>
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

            ResourcePtr<gui::Action> mQuitAction;

            ResourcePtr<gui::Action> mUndoAction;
            ResourcePtr<gui::Action> mRedoAction;

            ResourcePtr<Selector> mSelector;
            ResourcePtr<Controller> mController;

            bool init(utility::ErrorState& errorState) override;

            bool isQuitting() const { return mQuitting; }

        private:
            Slot<gui::Action&> mNewActionSlot = { this, &ActionController::onNewAction };
            void onNewAction(gui::Action&);
            Slot<gui::Action&> mOpenActionSlot = { this, &ActionController::onOpenAction };
            void onOpenAction(gui::Action&);
            Slot<gui::Action&> mSaveActionSlot = { this, &ActionController::onSaveAction };
            void onSaveAction(gui::Action&);
            Slot<gui::Action&> mSaveAsActionSlot = { this, &ActionController::onSaveAsAction };
            void onSaveAsAction(gui::Action&);

            Slot<gui::Action&> mQuitActionSlot = { this, &ActionController::onQuitAction };
            void onQuitAction(gui::Action&);

            Slot<gui::Action&> mUndoActionSlot = { this, &ActionController::onUndoAction };
            void onUndoAction(gui::Action&);
            Slot<gui::Action&> mRedoActionSlot = { this, &ActionController::onRedoAction };
            void onRedoAction(gui::Action&);

            std::string mPath;
            ResourcePtr<Model> mModel;
            bool mQuitting = false;
        };
    
    }
}