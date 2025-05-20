#include "inspector.h"

RTTI_BEGIN_CLASS(nap::edit::Inspector)
    RTTI_PROPERTY("Model", &nap::edit::Inspector::mModel, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ResourceListGui", &nap::edit::Inspector::mResourceListGui, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        void Inspector::draw()
        {
            if (mResourceListGui->getSelectedID().empty())
                return;

            auto resource = mModel->findResource(mResourceListGui->getSelectedID());
            assert(resource != nullptr);


        }

    }

}