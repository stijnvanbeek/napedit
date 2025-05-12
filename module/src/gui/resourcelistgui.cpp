#include "resourcelistgui.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::ResourceListGui)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Model", &nap::edit::ResourceListGui::mModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        ResourceListGui::ResourceListGui(Core &core): mCore(core)
        {
        }


        bool ResourceListGui::init(utility::ErrorState &errorState)
        {
            auto resourceType = RTTI_OF(Resource);
            auto allResources = resourceType.get_derived_classes();
            for (auto& resource : allResources)
                if (mCore.getResourceManager()->getFactory().canCreate(resource))
                    mTypes[resource.get_name().to_string()] = &resource;
            return true;
        }


        void ResourceListGui::addResourcePopup()
        {
            // Filter the available resource types
            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter)))
            {
                mFilteredTypes.clear();
                for (auto& pair : mTypes)
                    if (utility::contains(pair.first, mSearchFilter))
                        mFilteredTypes[pair.first] = pair.second;
            }

            {
                // List with all available resource types filtered
                auto& showedMap = mFilteredTypes;
                if (showedMap.empty())
                    showedMap = mTypes;

                if (ImGui::ListBoxHeader("##ResourcesTypesListBox", showedMap.size(), 20))
                {
                    auto resourceTypePair = showedMap.begin();
                    for (int i = 0; i < showedMap.size(); ++i)
                    {
                        if (ImGui::Selectable(resourceTypePair->first.c_str(), i == mSelectedType))
                        {
                            mSelectedType = i;
                            mModel->addResource(*resourceTypePair->second);
                            ImGui::CloseCurrentPopup();
                        }
                        resourceTypePair++;
                    }
                    ImGui::ListBoxFooter();
                }
            }
        }


        void ResourceListGui::draw()
        {
            // List of all resources
            ImGui::ListBoxHeader("##ResourcesListBox", mModel->getResources().size(), 20);
            int i = 0;
            for (auto& resource : mModel->getResources())
            {
                if (ImGui::Selectable(resource.first.c_str(), mSelectedID == resource.first))
                    mSelectedID = resource.first;
                i++;
            }
            ImGui::ListBoxFooter();

            if (ImGui::BeginPopup("##AddResourcePopup"))
            {
                addResourcePopup();
                ImGui::EndPopup();
            }

            // Popup when clicked on the resources list
            if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
            {
                if (ImGui::Selectable("Add resource"))
                {
                    ImGui::EndPopup();
                    ImGui::OpenPopup("##AddResourcePopup");
                }
                else if (!mSelectedID.empty())
                {
                    if (ImGui::Selectable(std::string("Remove " + mSelectedID).c_str()))
                    {
                        mModel->removeResource(mSelectedID);
                    }
                    ImGui::EndPopup();
                }
                else {
                    ImGui::EndPopup();
                }
            }
        }

    }

}