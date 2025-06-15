#include "layoutconstants.h"

#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::LayoutConstants)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("ColumnContentShift", &nap::edit::LayoutConstants::mColumnContentShift, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ListOffset", &nap::edit::LayoutConstants::mListOffset, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("NameColumnIndent", &nap::edit::LayoutConstants::mNameColumnIndent, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("TreeNodeArrowShift", &nap::edit::LayoutConstants::mTreeNodeArrowShift, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ValueSpacing", &nap::edit::LayoutConstants::mValueSpacing, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ValueEditorButtonWidth", &nap::edit::LayoutConstants::mValueEditorButtonWidth, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("PointerEditorButtonWidth", &nap::edit::LayoutConstants::mPointerEditorButtonWidth, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        LayoutConstants::LayoutConstants(Core &core) : Resource()
        {
            mGuiService = core.getService<IMGuiService>();
        }

    }

}