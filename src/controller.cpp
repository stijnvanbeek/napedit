#include "controller.h"

RTTI_BEGIN_CLASS(nap::edit::Controller)
    RTTI_PROPERTY("Model", &nap::edit::Controller::mModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        bool Controller::renameResource(const std::string &oldID, const std::string &newID)
        {
            if (newID.empty())
                return false;
            if (oldID == newID)
                return false;
            if (mModel->findResource(newID) == nullptr)
                return false;
            mModel->renameResource(oldID, newID);
            addUndoStack(
                [this, oldID, newID]{ mModel->renameResource(oldID, newID); },
                [this, oldID, newID]{ mModel->renameResource(newID, oldID); }
            );
            return true;
        }


        void Controller::createGroup(const rtti::TypeInfo &type)
        {
            assert(type.is_derived_from<IGroup>());
            auto mID = mModel->createGroup(type);
            addUndoStack(
                [this, type, mID]{ mModel->createGroup(type, mID); },
                [this, mID]{ mModel->removeResource(mID); }
            );
        }


        void Controller::createEntity()
        {
            auto mID = mModel->createEntity();
            addUndoStack(
                [this, mID](){ mModel->createEntity(mID); },
                [this, mID](){ mModel->removeResource(mID); }
            );
        }


        void Controller::createChildGroup(const std::string &parentID)
        {
            auto parentGroup = mModel->findGroup(parentID);
            assert(parentGroup != nullptr);
            auto mID = mModel->createGroup(parentGroup->get_type());
            mModel->moveGroupToParent(mID, parentGroup->mID);
            addUndoStack(
                [this, mID, parentGroup]()
                {
                    mModel->createGroup(parentGroup->get_type(), mID);
                    mModel->moveGroupToParent(mID, parentGroup->mID);
                },
                [this, mID]()
                {
                    mModel->removeResource(mID);
                }
            );
        }


        void Controller::removeResource(const std::string &mID)
        {
            auto type = mModel->findResource(mID)->get_type();
            mModel->removeResource(mID);
            addUndoStack(
                [this, mID]{ mModel->removeResource(mID); },
                [this, mID, type]{ mModel->createResource(type, mID); }
            );
        }


        void Controller::createResource(const rtti::TypeInfo &type, const std::string &parentID)
        {
            auto typeName = type.get_name().to_string();
            auto mID = mModel->createResource(type, typeName);
            if (!parentID.empty())
                mModel->moveResourceToGroup(mID, parentID);
            addUndoStack(
                [this, mID, type, parentID]()
                {
                    mModel->createResource(type, mID);
                    if (!parentID.empty())
                        mModel->moveResourceToGroup(mID, parentID);
                },
                [this, mID]()
                {
                    mModel->removeResource(mID);
                }
            );
        }


        void Controller::createGroup(const rtti::TypeInfo &type, const std::string &parentID)
        {
            auto mID = mModel->createGroup(type, type.get_name().to_string());
            if (!parentID.empty())
                mModel->moveGroupToParent(mID, parentID);
            addUndoStack(
                [this, mID, type, parentID]()
                {
                    mModel->createGroup(type, mID);
                    if (!parentID.empty())
                        mModel->moveGroupToParent(mID, parentID);
                },
                [this, mID]()
                {
                    mModel->removeResource(mID);
                }
            );
        }


        void Controller::addChildEntity(const std::string &childID, const std::string &parentID)
        {
            mModel->addEntityToParent(childID, parentID);
            addUndoStack(
                [this, childID, parentID](){
                    mModel->addEntityToParent(childID, parentID);
                },
                [this, childID, parentID]()
                {
                    mModel->removeEntityFromParent(childID, parentID);
                }
            );
        }


        void Controller::createComponent(const rtti::TypeInfo &type, const std::string &entityID)
        {
            auto mID = mModel->createComponent(type, entityID);
            addUndoStack(
                [this, mID, type, entityID]()
                {
                    mModel->createComponent(type, entityID, mID);
                },
                [this, mID]()
                {
                    mModel->removeResource(mID);
                }
            );
        }


        void Controller::undo()
        {
            if (!mUndoStack.empty())
            {
                mUndoStack.back()->mUndo();
                mRedoStack.emplace_back(std::move(mUndoStack.back()));
                mUndoStack.pop_back();
            }
        }


        void Controller::redo()
        {
            if (!mRedoStack.empty())
            {
                mRedoStack.back()->mRedo();
                mUndoStack.emplace_back(std::move(mRedoStack.back()));
                mRedoStack.pop_back();
            }
        }


        void Controller::addUndoStack(std::function<void()> doFunction, std::function<void()> undoFunction)
        {
            auto command = std::make_unique<Command>();
            command->mUndo = undoFunction;
            command->mRedo = doFunction;
            mUndoStack.emplace_back(std::move(command));
            mRedoStack.clear();
        }

    }

}