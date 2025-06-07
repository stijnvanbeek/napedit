#pragma once

#include <nap/resource.h>
#include <imguiservice.h>

namespace nap
{
    namespace edit
    {
    
        class NAPAPI LayoutConstants : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            LayoutConstants(Core& core);

            float mColumnContentShift = -30.f;
            float mListOffset = 20.f;
            float mNameColumnIndent = 50.f;
            float mTreeNodeArrowShift = -15.f;
            float mValueSpacing = 50.f;
            float mValueEditorButtonWidth = 50.f;

            float columnContentShift() { return mColumnContentShift * mGuiService->getScale(); }
            float listOffset() { return mListOffset * mGuiService->getScale(); }
            float nameColumnIndent() { return mNameColumnIndent * mGuiService->getScale(); }
            float treeNodeArrowShift() { return mTreeNodeArrowShift * mGuiService->getScale(); }
            float valueSpacing() { return mValueSpacing * mGuiService->getScale(); }
            float valueEditorButtonWidth() { return mValueEditorButtonWidth * mGuiService->getScale(); }

        private:
            IMGuiService* mGuiService = nullptr;
        };
    
    }
}