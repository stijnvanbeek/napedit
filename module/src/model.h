#pragma once

#include <nap/resource.h>
#include <nap/core.h>

#include <entity.h>
#include <nap/group.h>

namespace nap
{

    namespace edit
    {

        /**
         * Data model that is being edited.
         * All resources are owned in a flat list.
         * All resources are also kept in a tree structure.
         */
        class NAPAPI Model : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            /**
             * The tree consists of three lists:
             * - mResources: Resources that are not part of a group.
             * - mGroups: Groups that contain resources and subgroups
             * - mEntities: Entities that contain components and child entities.
             */
            struct Tree
            {
                std::vector<ResourcePtr<Resource>> mResources;
                std::vector<ResourcePtr<ResourceGroup>> mGroups;
                std::vector<ResourcePtr<Entity>> mEntities;
            };

            Model(Core& core) : mCore(core) { }

            bool init(utility::ErrorState &errorState) override;

            /**
             * Create a new resource
             * @param resourceType Type of the new resource. Needs to be a Resource subclass.
             * @param mID Suggestion for the resource's id. If left empty the type name will be used. If either is already in use a unique id will be generated from it using a number postfix.
             * @return The mID used for the new resource.
             */
            std::string createResource(const rttr::type& resourceType, const std::string& mID = "");

            /**
             * Create a new group.
             * @param groupType Type of the new group. Needs to be a IGroup subclass.
             * @param groupID Suggestion for the group's id. If left empty the type name will be used. If either is already in use a unique id will be generated from it using a number postfix.
             * @return The mID used for the new group.
             */
            std::string createGroup(const rttr::type& groupType, const std::string& groupID = "");

            /**
             * Create new entity.
             * @param id suggestion for the entity's id. If left empty the type name will be used. If either is already in use a unique id will be generated from it using a number postfix.
             * @return The mID used for the new entity.
             */
            std::string createEntity(const std::string& id = "");

            /**
             * Create new component.
             * @param componentType new component type
             * @param entityID mID of the entity containing the component.
             * @param componentID suggestion for the component's id. If left empty the type name will be used. If either is already in use a unique id will be generated from it using a number postfix.
             * @return The mID used for the new component.
             */
            std::string createComponent(const rttr::type& componentType, const std::string& entityID, const std::string& componentID = "");

            /**
             * Create a resource without adding it to the tree. Used for creating embedded objects.
             * @param type Resource type
             * @param mID suggestion for the resource's id. If left empty the type name will be used. If either is already in use a unique id will be generated from it using a number postfix.
             * @return The mID used for the new resource.
             */
            Resource* createEmbeddedObject(const rttr::type& type, const std::string& mID = "");

            /**
             * Remove object without that is not present in the tree. Used for deleting embedded objects.
             * @param mID Id of the object to be deleted.
             */
            void removeEmbeddedObject(const std::string& mID = "");

            void moveResourceToGroup(const std::string& mID, const std::string& groupID);
            void moveGroupToParent(const std::string& groupID, const std::string& parentGroupID);
            void moveEntityToParent(const std::string& entityID, const std::string& parentID);

            /**
             * Remove a resource, also deletes it from the tree.
             * @param mID Id of the resource to remove.
             */
            void removeResource(const std::string& mID);

            /**
             * Rename a resource.
             * @param mID Current name.
             * @param newName New name.
             */
            void renameResource(const std::string& mID, const std::string& newName);

            /**
             * @return All objects in the model as a flat list.
             */
            const std::vector<std::unique_ptr<Resource>>& getResources() const { return mResources; }

            /**
             * Find a resource by mID and type
             * @tparam T Type of the resource to find.
             * @param mID mID of the resource to find
             * @return Nullptr if no resource is found of the given type and name.
             */
            template <typename T> T* findResource(const std::string& mID);

            /**
             * Find resource by mID
             * @param mID mID ofn the resource to find.
             * @return Nullptr if no resource is found with this name.
             */
            Resource* findResource(const std::string& mID);

            /**
             * Find resource group by mID.
             * @param mID mID of the group
             * @return nullptr if no group is found with this mID.
             */
            ResourceGroup* findGroup(const std::string& mID);

            /**
             * @return Tree representation of all objects in the model.
             */
            Tree& getTree() { return mTree; }

            /**
             * @return All registered resource types, meaning types derived from Resource.
             */
            const std::map<std::string, const rtti::TypeInfo*>& getResourceTypes() const { return mResourceTypes; }

            /**
             * @return All registered group types, derived from IGroup.
             */
            const std::map<std::string, const rtti::TypeInfo*>& getGroupTypes() const { return mGroupTypes; }

            /**
             * Clear all data, blank model.
             */
            void clear();

            /**
             * Saves current data model to a json file.
             * @param filename Path to the json file.
             * @param errorState Logs errors during serialization
             * @return true on success
             */
            bool saveToFile(const std::string& filenamem, utility::ErrorState &errorState);

            bool loadFromFile(const std::string& filename, utility::ErrorState &errorState);

        private:
            bool eraseFromTree(std::vector<ResourcePtr<Resource>>& branch, Object& resource);
            bool eraseFromTree(std::vector<ResourcePtr<ResourceGroup>>& branch, Object& resource);
            bool eraseFromTree(std::vector<ResourcePtr<Entity>>& branch, Object& resource);
            bool eraseFromTree(Object& resource);

            bool findInTree(const std::string& mID, std::vector<ResourcePtr<Resource>>& branch);

            std::string getUniqueID(const std::string& baseID);

        	std::vector<std::unique_ptr<Resource>> mResources;
            Tree mTree;

            std::map<std::string, const rtti::TypeInfo*> mResourceTypes;
            std::map<std::string, const rtti::TypeInfo*> mGroupTypes;

            Core& mCore;
        };


        /**
         * Represents a selection of a resource owned by the Model
         */
        class NAPAPI Selector : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            ResourcePtr<Model> mModel; ///< Property: 'Model' Pointer to the Model the resource is selected from

            /**
             * Selects a resource
             * @param mID Unique ID of the selected resource
             */
            void set(const std::string& mID)
            {
                assert(mModel->findResource(mID) != nullptr);
                mSelection = mID;
            }

            /**
             * @return Return the selection, returns empty string if nothing is selected.
             */
            const std::string& get() const { return mSelection; }

            /**
             * Clears the selection.
             */
            void clear() { mSelection.clear(); }

            /**
             * @return Whether the selection is not set.
             */
            bool empty() const { return mSelection.empty(); }

        private:
            std::string mSelection;
        };


        template<typename T>
        T * Model::findResource(const std::string &mID)
        {
            auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
            if (it != mResources.end())
            {
                auto result = rtti_cast<T>(it->get());
                if (result != nullptr)
                    return result;
                else
                    return nullptr;
            }
            return nullptr;
        }

    }

}
