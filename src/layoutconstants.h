#pragma once

#include <nap/resource.h>
#include <imguiservice.h>

namespace nap
{
    namespace edit
    {
        /**
         * Manages bunch of layout constants that are used for spacing the GUI.
         * The constants are scaled with the GUI scale.
         */
        class NAPAPI LayoutConstants : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            LayoutConstants(Core& core);

            float mColumnContentShift = -30.f; ///< Property: 'ColumnContentShift' How much to shift a columns content horizontally.
            float mListOffset = 20.f; ///< Property: 'ListOffset' How much to shift a list offset vertically from the top of the GUI.
            float mNameColumnIndent = 50.f; ///< Property: 'NameColumnIndent' Indentation of the name column when a tree node is opened.
            float mTreeNodeArrowShift = -15.f; ///< Property: 'TreeNodeArrowShift' How much to shift the tree node arrow horizontally.
            float mValueSpacing = 50.f; ///< Property: 'ValueSpacing' How much to shift the value editor horizontally.
            float mValueEditorButtonWidth = 50.f; ///< Property: 'ValueEditorButtonWidth' Width of the value editor buttons.
            float mPointerEditorButtonWidth = 50.f; ///< Property: 'PointerEditorButtonWidth' Width of the pointer editor buttons.

            float columnContentShift() { return mColumnContentShift * mGuiService->getScale(); }
            float listOffset() { return mListOffset * mGuiService->getScale(); }
            float nameColumnIndent() { return mNameColumnIndent * mGuiService->getScale(); }
            float treeNodeArrowShift() { return mTreeNodeArrowShift * mGuiService->getScale(); }
            float valueSpacing() { return mValueSpacing * mGuiService->getScale(); }
            float valueEditorButtonWidth() { return mValueEditorButtonWidth * mGuiService->getScale(); }
            float pointerEditorButtonWidth() { return mPointerEditorButtonWidth * mGuiService->getScale(); }

        private:
            IMGuiService* mGuiService = nullptr;
        };
    
    }
}