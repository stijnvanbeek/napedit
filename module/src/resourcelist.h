#pragma once

#include <typemenu.h>
#include <Gui/Gui.h>
#include <model.h>
#include <nap/core.h>

#include "imgui_internal.h"

namespace nap
{
    namespace edit
    {
    
        class NAPAPI ResourceList : public gui::Gui
        {
            RTTI_ENABLE(Gui)
            
        public:
            ResourceList(Core& core);

            ResourcePtr<Model> mModel; ///< Property: 'Model'

            // Inherited
            bool init(utility::ErrorState& errorState) override;

            void setSelectedID(const std::string& id) { mSelectedID = id; }
            const std::string& getSelectedID() const { return mSelectedID; }

        private:
            void draw() override;

            template <typename T>
            void drawTree(const std::vector<ResourcePtr<T>>& branch);

            std::map<std::string, const rtti::TypeInfo*> mResourceTypes;
            std::map<std::string, const rtti::TypeInfo*> mGroupTypes;
            char mRenameBuffer[128];

            std::string mSelectedID;
            std::string mEditedID;
            std::string mEnteredID;

            float mTypeColumnOffset = 0;
            bool mStartEditing = false;

            TypeMenu mTypeMenu;
            Core& mCore;
        };


        template <typename T>
        void ResourceList::drawTree(const std::vector<ResourcePtr<T>>& branch)
        {
            for (auto& resource : branch)
            {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
                if (!resource->get_type().template is_derived_from<IGroup>())
                    flags |= ImGuiTreeNodeFlags_Leaf;

                std::string label = "###" + resource->mID;
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.f, 0.f, 0.f, 0.f));
                bool opened = ImGui::TreeNodeEx(label.c_str(), flags);
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();

                ImGui::SameLine();

                if (mSelectedID == resource->mID && mStartEditing)
                {
                    mEditedID = resource->mID;
                    mStartEditing = false;
                    mEnteredID.clear();
                    strcpy(mRenameBuffer, mEditedID.c_str());
                    ImGui::SetKeyboardFocusHere();
                }

                if (mEditedID == resource->mID)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    ImGui::SetNextItemWidth((ImGui::GetWindowWidth() / 2) - ImGui::GetCursorPosX() - 10);
                    if (ImGui::InputText("###RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                        mEnteredID = mRenameBuffer;
                    ImGui::PopStyleVar();
                }
                else {
                    if (ImGui::Selectable(resource->mID.c_str(), mSelectedID == resource->mID, ImGuiSelectableFlags_SpanAvailWidth))
                    {
                        mSelectedID = resource->mID;
                        mEditedID.clear();
                    }
                }

                auto type = resource->get_type();
                ImGui::SameLine();
                ImGui::SetCursorPosX(mTypeColumnOffset);
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                ImGui::Text(type.get_name().to_string().c_str());
                ImGui::PopStyleColor();

                if (opened)
                {
                    if (rtti_cast<IGroup>(resource.get()) != nullptr)
                    {
                        auto group = static_cast<ResourceGroup*>(resource.get());
                        drawTree(group->mMembers);
                        drawTree(group->mChildren);
                    }
                    ImGui::TreePop();
                }
            }
        }
    
    }
}
