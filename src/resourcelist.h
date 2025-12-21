#pragma once

#include <filteredmenu.h>
#include <layoutconstants.h>
#include <model.h>
#include <Gui/Gui.h>
#include <Gui/Action.h>
#include <nap/core.h>

#include "imguifunctions.h"
#include "imgui_internal.h"
#include <imguiservice.h>

#include "controller.h"

namespace nap
{
	namespace edit
	{
		/**
		 * GUI that presents the list of resources kept in the Model for editing.
		 * The resources are presented in a tree view that reflects the tree structure administered by the Model.
		 * The ResourceList allows for adding, removing and renaming resources in the tree.
		 */
		class NAPAPI ResourceList : public gui::Gui
		{
			RTTI_ENABLE(Gui)

		public:
			ResourceList(Core& core);

			ResourcePtr<Selector> mSelector; ///< Property: 'Selector' Link to Selector resource to keep track of the selected object
			ResourcePtr<Controller> mController; ///< Property: 'Controller' Link to the Controller resource that controls the editing of the model.
			ResourcePtr<LayoutConstants> mLayoutConstants; ///< Property: 'LayoutConstants' Link to a set of values used to layout the gui.

			ResourcePtr<Texture2D> mEntityIcon; ///< Property: 'EntityIcon' Icon showed at the entity main tree node.
			ResourcePtr<Texture2D> mResourceIcon; ///< Property: 'ResourceIcon' Icon showed at resource tree nodes.
			ResourcePtr<Texture2D> mComponentIcon; ///< Property: 'ComponentIcon' Icon showed at component tree nodes.
			ResourcePtr<Texture2D> mGroupIcon; ///< Property: 'GroupIcon' Icon showed at group tree nodes.

			// Inherited
			bool init(utility::ErrorState& errorState) override;

		private:
			// Inherited from Gui
			void draw() override;

			/**
			 * Draws a tree branch of the resource tree.
			 * This function is called recursively to draw sub branches.
			 * @tparam T Type of the resources in the branch.
			 * @param branch Vector of resources in the branch.
			 * @param nameOffset Horizontal offset of the name column of the current (sub) branch. This offset increases with the depth within the tree.
			 */
			template <typename T>
			void drawTree(const std::vector<ResourcePtr<T>>& branch, float nameOffset);

			/**
			 * Filters a tree branch of the resource tree.
			 * This function is called recursively to filter sub branches.
			 * @tparam T Type of the resources in the branch.
			 * @param filter Lower case filter string.
			 * @param branch Vector of resources in the branch.
			 * @param filteredResources Set of resources that have been filtered. This set is modified by this function.
			 * @return True if any resource in the branch has been filtered, false otherwise.
			 */
			template <typename T>
			bool filterTree(const std::string& filter, const std::vector<ResourcePtr<T>>& branch, std::set<Resource*>& filteredResources);

			/**
			 * @return True if a filter is applied to the resource tree.
			 */
			bool isFiltering() const { return mSearchFilter[0] != '\0'; }

			char mRenameBuffer[128]; // Buffer for renaming a resource.

			std::string mEditedID; // mID of the resource that is being renamed.
			std::string mEnteredID; // New mID that has been entered for the mID that is being renamed (stored in mEditedID)

			float mTypeColumnOffset = 0.f; // Horizontal offset of the type column.
			float mNameColumnOffset = 0.f; // Horizontal offset of the name column.

			FilteredMenu mFilterMenu; // Menu used to select the type of a resource to create.

			bool mStartEditing = false;
			bool mResourcesNodeSelected = false;
			bool mEntitiesNodeSelected = false;

			std::set<Resource*> mFilteredResources; // Resources that have been filtered by the search filter.
			char mSearchFilter[128];				// Search filter string.

			Core& mCore;
			ResourcePtr<Model> mModel;
			IMGuiService* mGuiService = nullptr;
		};


		template <typename T>
		void ResourceList::drawTree(const std::vector<ResourcePtr<T>>& branch, float nameOffset)
		{
			for (auto& resource : branch)
			{
				// Apply search filter
				if (isFiltering())
					if (mFilteredResources.find(resource.get()) == mFilteredResources.end())
						continue;

				bool opened = false;
				// For groups or entities draw the tree node arrow.
				if (resource->get_type().template is_derived_from<IGroup>() || resource->get_type().template is_derived_from<Entity>())
				{
					ImGui::SetCursorPosX(nameOffset + mLayoutConstants->treeNodeArrowShift());
					std::string label = "###" + resource->mID;
					opened = TreeNodeArrow(label.c_str());
					ImGui::SameLine();
				}
				else
					ImGui::SetCursorPosX(nameOffset);

				// draw icon
				if (resource->get_type() == RTTI_OF(Entity))
					Icon(*mEntityIcon, mGuiService);
				else if (resource->get_type().template is_derived_from<IGroup>())
					Icon(*mGroupIcon, mGuiService);
				else if (resource->get_type().template is_derived_from<Component>())
					Icon(*mComponentIcon, mGuiService);
				else
					Icon(*mResourceIcon, mGuiService);
				ImGui::SameLine();

				// If this node resource is selected and the user is renaming it, then the input field is focused.
				if (mSelector->get() == resource->mID && mStartEditing)
				{
					mEditedID = resource->mID;
					mStartEditing = false;
					mEnteredID.clear();
					strcpy(mRenameBuffer, mEditedID.c_str());
					ImGui::SetKeyboardFocusHere();
				}

				// If this resource is being renamed, draw the text input field.
				if (mEditedID == resource->mID)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					// ImGui::SetNextItemWidth((ImGui::GetWindowWidth() / 2) - ImGui::GetCursorPosX() - 10 * mGuiService->getScale());
					ImGui::SetNextItemWidth(mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX());
					if (ImGui::InputText("###RenameInput", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
						mEnteredID = mRenameBuffer;
					ImGui::PopStyleVar();
				}

				// If not being renamed, draw the text label.
				else {
					if (Selectable(resource->mID.c_str(), mSelector->get() == resource->mID, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
					{
						mSelector->set(resource->mID);
						mEditedID.clear();
					}
					// Check if the user double clicked on the resource name.
					if (ImGui::IsItemHovered())
						if (ImGui::IsMouseDoubleClicked(0))
							mStartEditing = true;
				}

				// Draw the type name
				auto type = resource->get_type();
				ImGui::SameLine();
				ImGui::SetCursorPosX(mTypeColumnOffset);
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
				ImGui::Text(type.get_name().to_string().c_str());
				ImGui::PopStyleColor();

				// If the tree node is opened, draw the sub tree.
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


		template<typename T>
		bool ResourceList::filterTree(const std::string &lowerCaseFilter, const std::vector<ResourcePtr<T>> &branch, std::set<Resource*> &filteredResources)
		{
			bool result = false;
			for (auto &resource : branch)
			{
				std::string lowerID = resource->mID;
				utility::toLower(lowerID);
				if (lowerID.find(lowerCaseFilter) != std::string::npos)
				{
					filteredResources.emplace(resource.get());
					result = true;
				}

				IGroup* igroup = rtti_cast<IGroup>(resource.get());
				if (igroup != nullptr)
				{
					auto group = static_cast<ResourceGroup*>(igroup);
					if (filterTree(lowerCaseFilter, group->mMembers, filteredResources))
						result = true;
					if (filterTree(lowerCaseFilter, group->mChildren, filteredResources))
						result = true;
				}

				Entity* entity = rtti_cast<Entity>(resource.get());
				if (entity != nullptr)
				{
					if (filterTree(lowerCaseFilter, entity->mComponents, filteredResources))
						result = true;
					if (filterTree(lowerCaseFilter, entity->mChildren, filteredResources))
						result = true;
				}
			}
			return result;
		}


	}
}
