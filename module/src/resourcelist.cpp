#include "resourcelist.h"

#include "imguiservice.h"
#include "nap/logger.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::ResourceList)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Model", &nap::edit::ResourceList::mModel, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LayoutConstants", &nap::edit::ResourceList::mLayoutConstants, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ResourceIcon", &nap::edit::ResourceList::mResourceIcon, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("GroupIcon", &nap::edit::ResourceList::mGroupIcon, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EntityIcon", &nap::edit::ResourceList::mEntityIcon, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComponentIcon", &nap::edit::ResourceList::mComponentIcon, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace edit
	{
		ResourceList::ResourceList(Core &core): mCore(core)
		{
			memset(mRenameBuffer, 0, sizeof(mRenameBuffer));
			mGuiService = core.getService<IMGuiService>();
		}


		bool ResourceList::init(utility::ErrorState &errorState)
		{
			return true;
		}


		void ResourceList::draw()
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));

			ImGui::BeginColumns("##ResourcesListColumns", 2);
			mNameColumnOffset = ImGui::GetCursorPosX();
			ImGui::Text("Name");

			ImGui::NextColumn();
			mTypeColumnOffset = ImGui::GetCursorPosX() + mLayoutConstants->columnContentShift();
			ImGui::Text("Type");
			ImGui::EndColumns();
			ImGui::PopStyleColor();

			// List of all resources
			ImGui::BeginChild("##ResourcesListBox", ImVec2(0, 0), true);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mLayoutConstants->listOffset());

			// Draw resources tree
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
			ImGui::SetCursorPosX(mNameColumnOffset + mLayoutConstants->treeNodeArrowShift());
			bool resourceTreeOpen = TreeNodeArrow("###ResourcesNode");
			ImGui::SameLine();
			Icon(*mResourceIcon, mGuiService);
			ImGui::SameLine();
			if (Selectable("Resources##Selectable", mResourcesNodeSelected, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
			{
				mResourcesNodeSelected = true;
				mEntitiesNodeSelected = false;
				mSelectedID.clear();
			}
			if (resourceTreeOpen)
			{
				drawTree(mModel->getTree().mGroups, mNameColumnOffset + mLayoutConstants->nameColumnIndent());
				drawTree(mModel->getTree().mResources, mNameColumnOffset + mLayoutConstants->nameColumnIndent());
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();

			// Draw entity tree
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
			ImGui::SetCursorPosX(mNameColumnOffset + mLayoutConstants->treeNodeArrowShift());
			bool entityTreeOpen = TreeNodeArrow("##EntitiesNode");
			ImGui::SameLine();
			Icon(*mEntityIcon, mGuiService);
			ImGui::SameLine();
			if (Selectable("Entities##Selectable", mEntitiesNodeSelected, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
			{
				mEntitiesNodeSelected = true;
				mResourcesNodeSelected = false;
				mSelectedID.clear();
			}
			if (entityTreeOpen)
			{
				drawTree(mModel->getTree().mEntities, mNameColumnOffset + mLayoutConstants->nameColumnIndent());
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();

			if (!mSelectedID.empty())
			{
				mEntitiesNodeSelected = false;
				mResourcesNodeSelected = false;
			}

			ImGui::EndChild();

			// Handle renaming
			if (!mEditedID.empty() && ImGui::IsMouseClicked(0))
				mEnteredID = mRenameBuffer;
			if (!mEnteredID.empty())
			{
				if (mEnteredID != mSelectedID && mModel->findResource(mEnteredID) == nullptr)
				{
					mModel->renameResource(mSelectedID, mEnteredID);
					mSelectedID = mEnteredID;
				}
				mEnteredID.clear();
				mEditedID.clear();
			}

			// Popup context menu on right click
			std::string chosenPopup;
			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
			{
				IGroup *selectedGroup = nullptr;
				Entity* selectedEntity = nullptr;
				if (!mSelectedID.empty())
				{
					selectedGroup = rtti_cast<IGroup>(mModel->findResource(mSelectedID));
					selectedEntity = rtti_cast<Entity>(mModel->findResource(mSelectedID));
				}

				if (mResourcesNodeSelected)
				{
					Icon(*mResourceIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Resource..."))
					{
						std::vector<std::string> menuItems;
						for (auto& pair : mModel->getResourceTypes())
							menuItems.push_back(pair.first);
						mFilteredMenu.init(std::move(menuItems));
						chosenPopup = "##AddResourcePopup";
					}

					Icon(*mGroupIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Group..."))
					{
						auto selectedGroup = rtti_cast<ResourceGroup>(mModel->findResource(mSelectedID));
						if (selectedGroup != nullptr)
						{
							auto type = selectedGroup->get_type();
							mModel->createGroup(type);
							mSelectedID.clear();
						} else
						{
							std::vector<std::string> menuItems;
							for (auto& pair : mModel->getGroupTypes())
								menuItems.push_back(pair.first);
							mFilteredMenu.init(std::move(menuItems));
							chosenPopup = "##AddGroupPopup";
						}
					}
				}

				if (mEntitiesNodeSelected)
				{
					Icon(*mEntityIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Entity..."))
						mModel->createEntity();
				}

				// When a group is selected
				if (selectedGroup != nullptr)
				{
					auto type = selectedGroup->getMemberType();
					// Create member
					Icon(*mResourceIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create member..."))
					{
						std::vector<std::string> menuItems;
						for (auto& pair : mModel->getResourceTypes())
							if (pair.second->is_derived_from(type))
								menuItems.push_back(pair.first);
						mFilteredMenu.init(std::move(menuItems));
						chosenPopup = "##AddResourcePopup";
					}
					// Create child
					Icon(*mGroupIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create child..."))
					{
						auto mID = mModel->createGroup(selectedGroup->get_type());
						mModel->moveGroupToParent(mID, mSelectedID);
						mSelectedID.clear();
					}
				}

				// When an entity is selected
				else if (selectedEntity != nullptr)
				{
					// Create component
					Icon(*mComponentIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create component..."))
					{
						std::vector<std::string> menuItems;
						for (auto& pair : mModel->getResourceTypes())
							if (pair.second->is_derived_from(RTTI_OF(Component)))
								menuItems.push_back(pair.first);
						mFilteredMenu.init(std::move(menuItems));
						chosenPopup = "##AddComponentPopup";
					}

					// Add child entity
					Icon(*mEntityIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Add child..."))
					{
						std::vector<std::string> entities;
						for (auto& resource : mModel->getResources())
							if (resource.get() != selectedEntity && resource->get_type() == RTTI_OF(Entity))
								entities.emplace_back(resource->mID);
						if (!entities.empty())
						{
							mFilteredMenu.init(std::move(entities));
							chosenPopup = "##AddChildEntityPopup";
						}
					}
				}

				// For all selections
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

			// Popup sub menus
			if (!chosenPopup.empty())
				ImGui::OpenPopup(chosenPopup.c_str());

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddResourcePopup"))
			{
				if (mFilteredMenu.show())
				{
					auto typeName = mFilteredMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					auto mID = mModel->createResource(type, typeName);
					if (!mSelectedID.empty())
						mModel->moveResourceToGroup(mID, mSelectedID);
					mSelectedID.clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddGroupPopup"))
			{
				if (mFilteredMenu.show())
				{
					auto typeName = mFilteredMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					auto mID = mModel->createGroup(type, typeName);
					if (!mSelectedID.empty())
						mModel->moveGroupToParent(mID, mSelectedID);
					mSelectedID.clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddChildEntityPopup"))
			{
				if (mFilteredMenu.show())
				{
					auto child = mFilteredMenu.getSelectedItem();
					if (!mSelectedID.empty())
						mModel->moveEntityToParent(mSelectedID, child);
					mSelectedID.clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddComponentPopup"))
			{
				if (mFilteredMenu.show())
				{
					auto typeName = mFilteredMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					auto mID = mModel->createComponent(type, mSelectedID);
					mSelectedID.clear();
				}
				ImGui::EndPopup();
			}

		}
	}
}
