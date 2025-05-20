#pragma once

#include <resourcelistgui.h>

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
            ResourcePtr<ResourceListGui> mResourceListGui; ///< Property: 'ResourceListGui'
            
        private:
            void draw() override;
        };
    
    }
}