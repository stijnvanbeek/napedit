#include "actioncontroller.h"

#include <nap/logger.h>
#include <NativeFileDialog.h>

RTTI_BEGIN_CLASS(nap::edit::ActionController)
    RTTI_PROPERTY("Model", &nap::edit::ActionController::mModel, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("NewAction", &nap::edit::ActionController::mNewAction, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("OpenAction", &nap::edit::ActionController::mOpenAction, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("SaveAction", &nap::edit::ActionController::mSaveAction, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("SaveAsAction", &nap::edit::ActionController::mSaveAsAction, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{

    namespace edit
    {

        bool ActionController::init(utility::ErrorState& errorState)
        {
            mNewAction->performSignal.connect(mNewActionSlot);
            mOpenAction->performSignal.connect(mOpenActionSlot);
            mSaveAction->performSignal.connect(mSaveActionSlot);
            mSaveAsAction->performSignal.connect(mSaveAsActionSlot);
            return true;
        }


        void ActionController::onNewAction(gui::Action&)
        {
            mModel->clear();
            mPath.clear();
        }


        void ActionController::onOpenAction(gui::Action&)
        {
            if (utility::openFileDialog("json", utility::getCWD(), mPath) == utility::FileDialogResult::Ok)
            {
                utility::ErrorState errorState;
                if (!mModel->loadFromFile(mPath, errorState))
                    Logger::error(errorState.toString().c_str());
            }
        }


        void ActionController::onSaveAction(gui::Action&)
        {
            if (mPath.empty())
                if (utility::saveFileDialog("json", utility::getCWD(), mPath) != utility::FileDialogResult::Ok)
                    return;
            utility::ErrorState errorState;
            if (!mModel->saveToFile(mPath, errorState))
                Logger::error(errorState.toString().c_str());

        }


        void ActionController::onSaveAsAction(gui::Action&)
        {
            if (utility::saveFileDialog("json", utility::getCWD(), mPath) != utility::FileDialogResult::Ok)
            {
                utility::ErrorState errorState;
                if (!mModel->saveToFile(mPath, errorState))
                    Logger::error(errorState.toString().c_str());
            }
        }
    
    }

}