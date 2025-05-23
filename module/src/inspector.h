#pragma once

#include <resourcelist.h>

namespace nap
{
    namespace edit
    {
    
        class Inspector : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            Inspector() = default;

            ResourcePtr<Model> mModel; ///< Property: 'Model'
            ResourcePtr<ResourceList> mResourceListGui; ///< Property: 'ResourceListGui'
            
        private:
            void draw() override;

            float mValueColumnOffset = 0.f;
            float mTypeColumnOffset = 0.f;
        };
    
    }
}