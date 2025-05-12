#pragma once

#include <Gui/Gui.h>
#include <model.h>
#include <nap/core.h>

namespace nap
{
    namespace edit
    {
    
        class ResourceListGui : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            ResourceListGui(Core& core);

            ResourcePtr<edit::Model> mModel; ///< Property: 'Model'

            // Inherited
            bool init(utility::ErrorState& errorState) override;

            void addResourcePopup();
            
        private:
            void draw() override;

            template <typename T>
            void drawTree(const std::vector<ResourcePtr<T>>& branch);

            std::map<std::string, const rtti::TypeInfo*> mTypes;
            std::map<std::string, const rtti::TypeInfo*> mFilteredTypes;
            int mSelectedType = -1;
            char mSearchFilter[128];
            char mRenameBuffer[128];

            std::set<std::string> mExpandedIDs;

            std::string mSelectedID;
            std::string mEditedID;
            std::string mRenamedID;
            Core& mCore;
        };


        template <typename T>
        void ResourceListGui::drawTree(const std::vector<ResourcePtr<T>>& branch)
        {
            // Members
            for (auto& resource : branch)
            {
                if (mEditedID == resource->mID)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetTreeNodeToLabelSpacing(), 0));
                    if (ImGui::InputText("##RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                        mRenamedID = mRenameBuffer;
                    ImGui::PopStyleVar();
                }
                else {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
                    if (mSelectedID == resource->mID)
                        flags |= ImGuiTreeNodeFlags_Selected;
                    if (ImGui::TreeNodeEx(resource->mID.c_str(), flags))
                    {
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            mEditedID = mSelectedID;
                            strcpy(mRenameBuffer, mEditedID.c_str());
                        }
                        if (ImGui::IsItemClicked())
                        {
                            mSelectedID = resource->mID;
                            mEditedID.clear();
                        }
                        auto group = rtti_cast<ResourceGroup>(resource.get());
                        if (group != nullptr)
                        {
                            drawTree(group->mMembers);
                            drawTree(group->mChildren);
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
    
    }
}