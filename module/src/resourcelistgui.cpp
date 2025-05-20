#include "resourcelistgui.h"

#include "nap/logger.h"

// #include "imgui_internal.h"

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
            auto groupBase = RTTI_OF(IGroup);
            auto allGroups = groupBase.get_derived_classes();
            for (auto& group : allGroups)
                if (mCore.getResourceManager()->getFactory().canCreate(group))
                    mGroupTypes[group.get_name().to_string()] = &group;

            auto resourceBase = RTTI_OF(Resource);
            auto allResources = resourceBase.get_derived_classes();
            for (auto& resource : allResources)
                if (mCore.getResourceManager()->getFactory().canCreate(resource))
                    if (mGroupTypes.find(resource.get_name().to_string()) == mGroupTypes.end())
                        mResourceTypes[resource.get_name().to_string()] = &resource;

            return true;
        }


        void ResourceListGui::draw()
        {
            // List of all resources
            ImGui::BeginChild("##ResourcesListBox", ImVec2(0, 0), false);

            ImGui::Columns(2, "##ResourcesListColumns", true);
            ImGui::Text("Name");
            ImGui::NextColumn();
            mTypeColumnOffset = ImGui::GetCursorPosX();
            ImGui::Text("Type");
            ImGui::Columns(1);

            drawTree(mModel->getTree().mGroups);
            drawTree(mModel->getTree().mResources);
            ImGui::EndChild();

            if (ImGui::IsMouseDoubleClicked(0))
            {
                mEditedID = mSelectedID;
                strcpy(mRenameBuffer, mEditedID.c_str());
            }

            if (!mEnteredID.empty())
            {
                mModel->renameResource(mSelectedID, mEnteredID);
                mSelectedID = mEnteredID;
                mEnteredID.clear();
                mEditedID.clear();
            }

            // Popup when right clicked on the resources list
            std::string chosenPopup;
            if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
            {
                IGroup* selectedGroup = nullptr;
                if (!mSelectedID.empty())
                    selectedGroup = rtti_cast<IGroup>(mModel->findResource(mSelectedID));

                // For groups
                if (selectedGroup != nullptr)
                {
                    auto type = selectedGroup->getMemberType();
                    // Create member
                    if (ImGui::Selectable("Create member..."))
                    {
                        mTypeMenu.init(mResourceTypes, &type);
                        chosenPopup = "##AddResourcePopup";
                    }
                    // Create child
                    if (ImGui::Selectable("Create child..."))
                    {
                        auto mID = mModel->createGroup(selectedGroup->get_type());
                        mModel->moveGroupToParent(mID, mSelectedID);
                        mSelectedID.clear();
                    }
                }

                // No group selected
                else {
                    if (ImGui::Selectable("Create Resource..."))
                    {
                        mTypeMenu.init(mResourceTypes);
                        chosenPopup = "##AddResourcePopup";
                    }
                    if (ImGui::Selectable("Create Group..."))
                    {
                        auto selectedGroup = rtti_cast<ResourceGroup>(mModel->findResource(mSelectedID));
                        if (selectedGroup != nullptr)
                        {
                            auto type = selectedGroup->get_type();
                            mModel->createGroup(type);
                            mSelectedID.clear();
                        }
                        else {
                            mTypeMenu.init(mGroupTypes);
                            chosenPopup = "##AddGroupPopup";
                        }
                    }
                }

                // For selections
                if (!mSelectedID.empty())
                {
                    if (ImGui::Selectable(std::string("Rename " + mSelectedID).c_str()))
                    {
                        mEditedID = mSelectedID;
                        strcpy(mRenameBuffer, mEditedID.c_str());
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
                if (mTypeMenu.show())
                {
                    auto mID = mModel->createResource(*mTypeMenu.getSelectedType(), mTypeMenu.getSelectedTypeID());
                    if (!mSelectedID.empty())
                        mModel->moveResourceToGroup(mID, mSelectedID);
                    mSelectedID.clear();
                }
                ImGui::EndPopup();
            }
            if (ImGui::BeginPopup("##AddGroupPopup"))
            {
                if (mTypeMenu.show())
                {
                    auto mID = mModel->createGroup(*mTypeMenu.getSelectedType(), mTypeMenu.getSelectedTypeID());
                    if (!mSelectedID.empty())
                        mModel->moveGroupToParent(mID, mSelectedID);
                    mSelectedID.clear();
                }
                ImGui::EndPopup();
            }
        }


    }

}