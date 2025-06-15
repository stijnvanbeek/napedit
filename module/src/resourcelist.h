#pragma once

#include <filteredmenu.h>
#include <layoutconstants.h>
#include <Gui/Gui.h>
#include <model.h>
#include <nap/core.h>

#include "imguifunctions.h"

#include "imgui_internal.h"
#include <imguiservice.h>

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
            ResourcePtr<LayoutConstants> mLayoutConstants;

            // Inherited
            bool init(utility::ErrorState& errorState) override;

            void setSelectedID(const std::string& id) { mSelectedID = id; }
            const std::string& getSelectedID() const { return mSelectedID; }

        private:
            void draw() override;

            template <typename T>
            void drawTree(const std::vector<ResourcePtr<T>>& branch, float nameOffset);

            char mRenameBuffer[128];

            std::string mSelectedID;
            std::string mEditedID;
            std::string mEnteredID;

            float mTypeColumnOffset = 0.f;
            float mNameColumnOffset = 0.f;
            bool mStartEditing = false;

            FilteredMenu mFilteredMenu;
            bool mResourcesNodeSelected = false;
            bool mEntitiesNodeSelected = false;

            Core& mCore;
            IMGuiService* mGuiService = nullptr;
        };


        template <typename T>
        void ResourceList::drawTree(const std::vector<ResourcePtr<T>>& branch, float nameOffset)
        {
            for (auto& resource : branch)
            {
                bool opened = false;
                if (resource->get_type().template is_derived_from<IGroup>() || resource->get_type().template is_derived_from<Entity>())
                {
                    ImGui::SetCursorPosX(nameOffset + mLayoutConstants->treeNodeArrowShift());
                    std::string label = "###" + resource->mID;
                    opened = TreeNodeArrow(label.c_str());
                    ImGui::SameLine();
                }
                else
                    ImGui::SetCursorPosX(nameOffset);

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
                    // ImGui::SetNextItemWidth((ImGui::GetWindowWidth() / 2) - ImGui::GetCursorPosX() - 10 * mGuiService->getScale());
                    ImGui::SetNextItemWidth(mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX());
                    if (ImGui::InputText("###RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                        mEnteredID = mRenameBuffer;
                    ImGui::PopStyleVar();
                }
                else {
                    if (Selectable(resource->mID.c_str(), mSelectedID == resource->mID, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
                    {
                        mSelectedID = resource->mID;
                        mEditedID.clear();
                    }
                    if (ImGui::IsItemHovered())
                        if (ImGui::IsMouseDoubleClicked(0))
                            mStartEditing = true;
                }

                auto type = resource->get_type();
                ImGui::SameLine();
                ImGui::SetCursorPosX(mTypeColumnOffset);
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                ImGui::Text(type.get_name().to_string().c_str());
                ImGui::PopStyleColor();

                if (opened)
                {
                    IGroup* igroup = rtti_cast<IGroup>(resource.get());
                    if (igroup != nullptr)
                    {
                        auto group = static_cast<ResourceGroup*>(igroup);
                        drawTree(group->mMembers, nameOffset + mLayoutConstants->nameColumnIndent());
                        drawTree(group->mChildren, nameOffset + mLayoutConstants->nameColumnIndent());
                    }

                    Entity* entity = rtti_cast<Entity>(resource.get());
                    if (entity != nullptr)
                    {
                        drawTree(entity->mComponents, nameOffset + mLayoutConstants->nameColumnIndent());
                        drawTree(entity->mChildren, nameOffset + mLayoutConstants->nameColumnIndent());
                    }
                    ImGui::TreePop();
                }
            }
        }
    
    }
}
