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
            // auto groupType = RTTI_OF(ResourceGroup);
            // mGroupTypes[groupType.get_name().to_string()] = &groupType;

            auto resourceBase = RTTI_OF(Resource);
            auto allResources = resourceBase.get_derived_classes();
            for (auto& resource : allResources)
                if (mCore.getResourceManager()->getFactory().canCreate(resource))
                    if (mGroupTypes.find(resource.get_name().to_string()) == mGroupTypes.end())
                        mResourceTypes[resource.get_name().to_string()] = &resource;

            return true;
        }


        void ResourceListGui::createItemPopup(const std::map<std::string, const rtti::TypeInfo*>& types)
        {
            // Filter the available resource types
            if (ImGui::InputText("Filter", mSearchFilter, sizeof(mSearchFilter)))
            {
                mFilteredTypes.clear();
                for (auto& pair : types)
                    if (utility::contains(pair.first, mSearchFilter))
                        mFilteredTypes[pair.first] = pair.second;
            }

            {
                // List with all available resource types filtered
                auto& showedMap = mFilteredTypes;
                if (showedMap.empty())
                    showedMap = types;

                if (ImGui::ListBoxHeader("##TypesListBox", showedMap.size(), 20))
                {
                    auto resourceTypePair = showedMap.begin();
                    for (int i = 0; i < showedMap.size(); ++i)
                    {
                        if (ImGui::Selectable(resourceTypePair->first.c_str(), i == mSelectedType))
                        {
                            mSelectedType = i;
                            if (resourceTypePair->second->is_derived_from(RTTI_OF(IGroup)))
                                mModel->createGroup(*resourceTypePair->second);
                            else
                                mModel->createResource(*resourceTypePair->second);
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
                if (ImGui::Selectable("Create Resource..."))
                {
                    chosenPopup = "##AddResourcePopup";
                }
                if (ImGui::Selectable("Create Group..."))
                {
                    chosenPopup = "##AddGroupPopup";
                }
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
            {
                mFilteredTypes.clear();
                ImGui::OpenPopup(chosenPopup.c_str());
            }

            if (ImGui::BeginPopup("##AddResourcePopup"))
            {
                createItemPopup(mResourceTypes);
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopup("##AddGroupPopup"))
            {
                createItemPopup(mGroupTypes);
                ImGui::EndPopup();
            }
        }


    }

}