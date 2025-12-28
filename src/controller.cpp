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
            if (mModel->findResource(newID) != nullptr)
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


        void Controller::createEmbeddedObject(ValuePath &path, const rtti::TypeInfo &type)
        {
            auto resource = mModel->createEmbeddedObject(type);
            assert(resource != nullptr);
            auto mID = resource->mID;

            if (path.isPointer())
                path.getResolvedPath().setValue(resource);
            else if (path.isArrayElement() || path.isArray())
                doInsertArrayElement(path, resource);

            addUndoStack(
                [this, type, path, mID]() mutable
                {
                    auto resource = mModel->createEmbeddedObject(type, mID);
                    if (path.isPointer())
                        path.getResolvedPath().setValue(resource);
                    else if (path.isArrayElement() || path.isArray())
                        doInsertArrayElement(path, resource);
                },
                [this, path, mID]() mutable
                {
                    if (path.isPointer())
                        path.getResolvedPath().setValue(nullptr);
                    else if (path.isArrayElement() || path.isArray())
                        doRemoveArrayElement(path);
                    mModel->removeResource(mID);
                }
            );
        }


        void Controller::removeEmbeddedObject(ValuePath &path)
        {
            rtti::Object* resource = path.getResolvedPath().getValue().get_value<rtti::ObjectPtr<rtti::Object>>().get();
            Resource* removed_object = mModel->removeEmbeddedObject(resource->mID).release();
            assert(removed_object == resource);
            path.getResolvedPath().setValue(nullptr);

            addUndoStack(
                [this, path, removed_object]() mutable
                {
                    path.getResolvedPath().setValue(removed_object);
                    mModel->addEmbeddedObject(removed_object);
                },
                [this, path, removed_object]() mutable
                {
                    removed_object = mModel->removeEmbeddedObject(removed_object->mID).release();
                    path.getResolvedPath().setValue(nullptr);
                }
            );
        }


        void Controller::insertArrayElement(ValuePath &path)
        {
            auto array = path.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto elementType = view.get_rank_type(1);
            assert(elementType.can_create_instance());
            auto element = elementType.create();

            doInsertArrayElement(path, element);

            addUndoStack(
                [this, path, element]() mutable
                {
                    doInsertArrayElement(path, element);
                },
                [this, path]() mutable
                {
                    doRemoveArrayElement(path);
                }
            );
        }


        void Controller::removeArrayElement(ValuePath &path)
        {
            auto element = path.getResolvedPath().getValue();
            doRemoveArrayElement(path);
            addUndoStack(
                [this, path]() mutable
                {
                    path.resolve(*mModel);
                    doRemoveArrayElement(path);
                },
                [this, path, element]() mutable
                {
                    path.resolve(*mModel);
                    doInsertArrayElement(path, element);
                }
            );
        }


        void Controller::moveArrayElementUp(ValuePath &path)
        {
            auto oldIndex = path.getArrayIndex();
            auto newIndex = path.getArrayIndex() - 1;
            if (!doMoveArrayElementUp(path))
                return;
            addUndoStack(
                [this, path, oldIndex]() mutable
                {
                    path.set(oldIndex);
                    doMoveArrayElementUp(path);
                },
                [this, path, newIndex]() mutable
                {
                    path.set(newIndex);
                    doMoveArrayElementDown(path);
                }
            );
        }


        void Controller::moveArrayElementDown(ValuePath &path)
        {
            auto oldIndex = path.getArrayIndex();
            auto newIndex = path.getArrayIndex() + 1;
            if (!doMoveArrayElementDown(path))
                return;
            addUndoStack(
                [this, path, oldIndex]() mutable
                {
                    path.set(oldIndex);
                    doMoveArrayElementDown(path);
                },
                [this, path, newIndex]() mutable
                {
                    path.set(newIndex);
                    doMoveArrayElementUp(path);
                }
            );
        }


        void Controller::doRemoveArrayElement(ValuePath &path)
        {
            auto array = path.getResolvedPath().getValue();
            auto view = array.create_array_view();
            if (path.isArrayElement())
            {
                view.remove_value(path.getArrayIndex());
            }
            else if (path.isArray())
                view.remove_value(view.get_size() - 1);
            path.getResolvedPath().setValue(array);
        }


        bool Controller::doMoveArrayElementUp(ValuePath &path)
        {
            if (path.getArrayIndex() < 1)
                return false;
            auto array = path.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto size = view.get_size();
            assert(path.getArrayIndex() < size);
            auto element = view.get_value(path.getArrayIndex());
            view.remove_value(path.getArrayIndex());
            view.insert_value(path.getArrayIndex() - 1, element);
            path.getResolvedPath().setValue(array);
            return true;
        }


        bool Controller::doMoveArrayElementDown(ValuePath &path)
        {
            auto array = path.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto size = view.get_size();
            assert(path.getArrayIndex() < size);
            if (path.getArrayIndex() == size - 1)
                return false;
            auto element = view.get_value(path.getArrayIndex());
            view.remove_value(path.getArrayIndex());
            view.insert_value(path.getArrayIndex() + 1, element);
            path.getResolvedPath().setValue(array);
            return true;
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
            command->mUndo = std::move(undoFunction);
            command->mRedo = std::move(doFunction);
            mUndoStack.emplace_back(std::move(command));
            mRedoStack.clear();
        }


        template <>
        void Controller::insertArrayElement(ValuePath& path, Resource* element)
        {
            assert(path.isArray());
            assert(path.isResolved());

            doInsertArrayElement(path, element);
            auto mID = element->mID;
            addUndoStack(
                [this, path, mID]() mutable
                {
                    path.resolve(*mModel);
                    auto element = mModel->findResource(mID);
                    if (element != nullptr)
                        doInsertArrayElement(path, element);
                },
                [this, path]() mutable
                {
                    path.resolve(*mModel);
                    if (path.isResolved())
                        doRemoveArrayElement(path);
                }
            );
        }


        Controller::ValuePath::ValuePath(const ValuePath &other) : mResolvedPath()
        {
            mRootID = other.mRootID;
            mPath = other.mPath;
            mResolvedPath = other.mResolvedPath;
            mIsArrayElement = other.mIsArrayElement;
            mArrayIndex = other.mArrayIndex;
            mIsResolved = false;
        }


        Controller::ValuePath::ValuePath(ValuePath &&other) : mResolvedPath()
        {
            mRootID = other.mRootID;
            mPath = other.mPath;
            mResolvedPath = other.mResolvedPath;
            mIsArrayElement = other.mIsArrayElement;
            mArrayIndex = other.mArrayIndex;
            mIsResolved = false;
        }


        void Controller::ValuePath::set(const rtti::Path &path, Resource *root)
        {
            mRootID = root->mID;
            mPath = path;
            mIsArrayElement = false;
            resolve(root);
        }


        void Controller::ValuePath::set(const rtti::Path &arrayPath, int index, Resource *root)
        {
            mRootID = root->mID;
            mPath = arrayPath;
            mIsArrayElement = true;
            mArrayIndex = index;
            resolve(root);
        }


        void Controller::ValuePath::set(int arrayIndex)
        {
            assert(mIsArrayElement);
            mArrayIndex = arrayIndex;
        }


        const rtti::Path & Controller::ValuePath::getPath() const
        {
            return mPath;
        }


        void Controller::ValuePath::resolve(Resource* root)
        {
            mIsResolved = mPath.resolve(root, mResolvedPath);
            if (mIsArrayElement)
            {
                // mPath.pushArrayElement(mArrayIndex);
                assert(mResolvedPath.getType().is_array());
            }
            assert(mIsResolved);
        }


        void Controller::ValuePath::resolve(Model& model)
        {
            auto root = model.findResource(mRootID);
            assert(root != nullptr);
            resolve(root);
        }

    }

}