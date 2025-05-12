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
            memset(mRenameBuffer, 0, sizeof(mRenameBuffer));
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
            ImGui::BeginChild("##ResourcesListBox", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            int i = 0;
            std::string newName;
            for (auto& resource : mModel->getResources())
            {
                bool renaming = false;
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
                if (mSelectedID == resource.first)
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                    if (mRenameMode)
                    {
                        renaming = true;
                        if (ImGui::InputText("##RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                            newName = mRenameBuffer;
                    }
                }
                if (!renaming && ImGui::TreeNodeEx(resource.first.c_str(), flags))
                {
                    if (ImGui::IsItemClicked())
                        mSelectedID = resource.first;

                    ImGui::TreePop();
                }
                i++;
            }
            ImGui::EndChild();

            if (!newName.empty())
            {
                mModel->renameResource(mSelectedID, newName);
                mSelectedID = newName;
                mRenameMode = false;
            }

            // Popup when right clicked on the resources list
            std::string chosenPopup;
            if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
            {
                if (ImGui::Selectable("Add resource"))
                {
                    chosenPopup = "##AddResourcePopup";
                }
                if (!mSelectedID.empty())
                {
                    if (ImGui::Selectable("Rename"))
                    {
                        strcpy(mRenameBuffer, mSelectedID.c_str());
                        mRenameMode = true;
                    }
                    if (ImGui::Selectable(std::string("Remove " + mSelectedID).c_str()))
                    {
                        mModel->removeResource(mSelectedID);
                        mSelectedID.clear();
                    }
                }
                ImGui::EndPopup();
            }

            if (!chosenPopup.empty())
                ImGui::OpenPopup(chosenPopup.c_str());

            if (ImGui::BeginPopup("##AddResourcePopup"))
            {
                addResourcePopup();
                ImGui::EndPopup();
            }
        }

    }

}