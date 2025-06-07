  #pragma once

#include <resourcelist.h>
#include <propertyeditor.h>
#include <filteredmenu.h>

  namespace nap
{
    namespace edit
    {
    
        class Inspector : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            Inspector(Core&);

            bool init(utility::ErrorState& errorState) override;

            ResourcePtr<Model> mModel; ///< Property: 'Model'
            ResourcePtr<ResourceList> mResourceList; ///< Property: 'ResourceListGui'
            ResourcePtr<LayoutConstants> mLayoutConstants;

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
            bool drawValue(rtti::Variant& value, rtti::TypeInfo type, const rtti::Path& path, const std::string& name, bool isArrayElement, int arrayIndex, float nameOffset, float valueOffset, float typeOffset);
            bool drawArray(rtti::Variant& array, const rtti::Path& path, const std::string& name, float nameOffset, float valueOffset, float typeOffset);
            bool drawEnum(rtti::Variant& var, rtti::TypeInfo type, const rtti::Path& path, const std::string& name, float valueWidth);
            bool drawPointer(rtti::Variant& var, rtti::TypeInfo type, const rtti::Path& path, float valueWidth);

            void insertArrayElement();
            void removeArrayElement();
            void moveArrayElementUp();
            void moveArrayElementDown();
            void addArrayElement();
            void choosePointer();

            std::string mInspectedResourceID;
            Resource* mInspectedResource = nullptr;
            Selection mSelection;
            FilteredMenu mResourceMenu;
            bool mOpenResourceMenu = false;
            IMGuiService* mGuiService = nullptr;

            std::map<const rtti::TypeInfo, std::unique_ptr<IPropertyEditor>> mPropertyEditors;
        };

    
    }
}
