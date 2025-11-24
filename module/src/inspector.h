#pragma once

#include <model.h>
#include <propertyeditor.h>
#include <filteredmenu.h>
#include <layoutconstants.h>

  namespace nap
{
    namespace edit
    {
        /**
         * Inspector Gui for editing properties of a resource.
         */
        class Inspector : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            Inspector(Core&);

            bool init(utility::ErrorState& errorState) override;

            ResourcePtr<Selector> mResourceSelector; ///< Property: 'ResourceSelector' Link to the Selector that selects a resource from the data model being edited.
            ResourcePtr<LayoutConstants> mLayoutConstants; ///< Property: 'LayoutConstants' Link to a set of values used to layout the gui.

            /**
             * Register a property editor for a custom type.
             * @tparam T The type of the property editor.
             */
            template <typename T>
            void registerPropertyEditor()
            {
                auto editor = std::make_unique<T>();
                mPropertyEditors[editor->getType()] = std::move(editor);
            }

        private:
            class Selection
            {
            public:
                Selection() = default;
                void set(const rtti::Path& path, Resource* root);
                void set(const rtti::Path& arrayPath, int index, Resource* root);
                void clear() { mIsValid = false; }
                bool isValid() const { return mIsValid; }
                bool isArrayElement() const { return mIsArrayElement; }
                bool isArray() const { return mResolvedPath.getType().is_array(); }
                int getArrayIndex() const { return mArrayIndex; }
                bool isPointer() const { return mResolvedPath.getType().is_derived_from<rtti::ObjectPtrBase>(); }
                rtti::ResolvedPath& getResolvedPath() { return mResolvedPath; }
                const rtti::Path& getPath() const { return mPath; }

            private:
                rtti::Path mPath;
                rtti::ResolvedPath mResolvedPath;
                bool mIsArrayElement = false;
                int mArrayIndex = -1;
                bool mIsValid = false;
            };

        private:
            void draw() override;

            void drawContextMenu();

            bool drawObject(rtti::Variant& object, rtti::TypeInfo type, const rtti::Path& path, float nameOffset, float valueOffset, float typeOffset);
            bool drawValue(rtti::Variant& value, rtti::TypeInfo type, const rtti::Path& path, const std::string& name, bool isArrayElement, int arrayIndex, bool isEmbeddedPointer, float nameOffset, float valueOffset, float typeOffset);
            bool drawArray(rtti::Variant& array, const rtti::Path& path, const std::string& name, bool isEmbeddedPointerArray, float nameOffset, float valueOffset, float typeOffset);
            bool drawEnum(rtti::Variant& var, rtti::TypeInfo type, const rtti::Path& path, const std::string& name, float valueWidth);
            bool drawPointer(rtti::Variant& var, rtti::TypeInfo type, const rtti::Path& path, const std::string& name, bool isEmbedded, float valueWidth);
            void drawID(rtti::Variant& value, const rtti::Path& parentPath, float width);

            void insertArrayElement();
            void removeArrayElement();
            void moveArrayElementUp();
            void moveArrayElementDown();
            void addArrayElement();
            void addArrayPtrElement(Resource* resource);
            void choosePointer(const rtti::TypeInfo& type);
            void createEmbeddedObject(const rtti::TypeInfo& type);

            template <typename T>
            void insertArrayElement(T element);

            Slot<> mPostResourcesLoadedSlot;
            void onPostResourcesLoaded();

            std::string mInspectedResourceID;
            ResourcePtr<Resource> mInspectedResource;
            Selection mSelection;
            FilteredMenu mFilteredMenu;
            bool mOpenResourceMenu = false;
            bool mOpenResourceTypeMenu = false;
            IMGuiService* mGuiService = nullptr;
            ResourcePtr<Model> mModel;

            std::map<const rtti::TypeInfo, std::unique_ptr<IPropertyEditor>> mPropertyEditors;

            Core& mCore;
        };


        template <typename T>
        void Inspector::insertArrayElement(T element)
        {
            auto array = mSelection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            assert(mSelection.getArrayIndex() <= view.get_size());
            view.insert_value(mSelection.getArrayIndex(), element);
            mSelection.getResolvedPath().setValue(array);
            auto arrayPath = mSelection.getPath();
            arrayPath.popBack();
            mSelection.set(arrayPath, mSelection.getArrayIndex(), mInspectedResource.get());
        }


    
    }
}
