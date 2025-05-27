#pragma once

#include <resourcelist.h>
#include <propertyeditor.h>

namespace nap
{
    namespace edit
    {
    
        class Inspector : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            Inspector() = default;

            bool init(utility::ErrorState& errorState) override;

            ResourcePtr<Model> mModel; ///< Property: 'Model'
            ResourcePtr<ResourceList> mResourceList; ///< Property: 'ResourceListGui'

            template <typename T>
            void registerPropertyEditor()
            {
                auto editor = std::make_unique<T>();
                mPropertyEditors[editor->getType()] = std::move(editor);
            }
            
        private:
            void draw() override;

            float mValueColumnOffset = 0.f;
            float mTypeColumnOffset = 0.f;

            std::map<const rtti::TypeInfo, std::unique_ptr<IPropertyEditor>> mPropertyEditors;
        };
    
    }
}