#pragma once

#include <model.h>

namespace nap
{
    namespace edit
    {

        class Controller : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            Controller() = default;

            ResourcePtr<Model> mModel;
            bool renameResource(const std::string& oldID, const std::string& newID);
            void createGroup(const rtti::TypeInfo& type);
            void createEntity();
            void createChildGroup(const std::string& parentID);
            void removeResource(const std::string& mID);
            void createResource(const rtti::TypeInfo& type, const std::string& parentID);
            void createGroup(const rtti::TypeInfo& type, const std::string& parentID);
            void addChildEntity(const std::string& childID, const std::string& parentID);
            void createComponent(const rtti::TypeInfo& type, const std::string& entityID);

            void undo();
            void redo();

        private:
            void addUndoStack(std::function<void()> doFunction, std::function<void()> undoFunction);
            struct Command
            {
                std::function<void()> mUndo;
                std::function<void()> mRedo;
            };
            std::vector<std::unique_ptr<Command>> mUndoStack;
            std::vector<std::unique_ptr<Command>> mRedoStack;
        };
    
    }
}