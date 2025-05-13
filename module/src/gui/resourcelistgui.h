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
            std::string mEnteredID;

            Core& mCore;
        };


        template <typename T>
        void ResourceListGui::drawTree(const std::vector<ResourcePtr<T>>& branch)
        {
            // Members
            for (auto& resource : branch)
            {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
                if (!resource.get_type().is_derived_from(rtti::TypeInfo::get<ResourceGroup>()))
                    flags |= ImGuiTreeNodeFlags_Leaf;

                std::string label = "###" + resource->mID;
                bool opened = ImGui::TreeNodeEx(label.c_str(), flags);

                ImGui::SameLine();
                if (mEditedID == resource->mID)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    if (ImGui::InputText("###RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                        mEnteredID = mRenameBuffer;
                    ImGui::PopStyleVar();
                }
                else {
                    if (ImGui::Selectable(resource->mID.c_str()))
                    {
                        mSelectedID = resource->mID;
                        mEditedID.clear();
                    }
                }

                if (opened)
                {
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