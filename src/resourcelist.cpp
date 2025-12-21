#include "resourcelist.h"

#include "imguiservice.h"
#include "nap/logger.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::ResourceList)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Selector", &nap::edit::ResourceList::mSelector, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controller", &nap::edit::ResourceList::mController, nap::rtti::EPropertyMetaData::Required)
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
			mModel = mSelector->mModel;
			mSearchFilter[0] = '\0';
			return true;
		}


		void ResourceList::draw()
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));

			// Apply search filter
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
			ImGui::InputText("##SearchInput", mSearchFilter, sizeof(mSearchFilter));
			mFilteredResources.clear();
			if (isFiltering())
			{
				auto lowerCaseFilter = utility::toLower(mSearchFilter);
				filterTree(lowerCaseFilter, mModel->getTree().mResources, mFilteredResources);
				filterTree(lowerCaseFilter, mModel->getTree().mGroups, mFilteredResources);
				filterTree(lowerCaseFilter, mModel->getTree().mEntities, mFilteredResources);
			}

			// Draw column headers
			ImGui::BeginColumns("##ResourcesListColumns", 2);
			mNameColumnOffset = ImGui::GetCursorPosX();
			ImGui::Text("Name");

			ImGui::NextColumn();
			mTypeColumnOffset = ImGui::GetCursorPosX() + mLayoutConstants->columnContentShift();
			ImGui::Text("Type");
			ImGui::EndColumns();
			ImGui::PopStyleColor();

			// List of all resources
			ImGui::SetNextWindowBgAlpha(0.1f);
			ImGui::BeginChild("##ResourcesListBox", ImVec2(0, 0), true);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mLayoutConstants->listOffset());

			// Draw resources tree
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
			ImGui::SetCursorPosX(mNameColumnOffset + mLayoutConstants->treeNodeArrowShift());
			bool resourceTreeOpen = TreeNodeArrow("###ResourcesNode", true);
			ImGui::SameLine();
			Icon(*mResourceIcon, mGuiService);
			ImGui::SameLine();
			if (Selectable("Resources##Selectable", mResourcesNodeSelected, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
			{
				mResourcesNodeSelected = true;
				mEntitiesNodeSelected = false;
				mSelector->clear();
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
			bool entityTreeOpen = TreeNodeArrow("##EntitiesNode", true);
			ImGui::SameLine();
			Icon(*mEntityIcon, mGuiService);
			ImGui::SameLine();
			if (Selectable("Entities##Selectable", mEntitiesNodeSelected, mTypeColumnOffset - mNameColumnOffset - ImGui::GetCursorPosX() - 10 * mGuiService->getScale()))
			{
				mEntitiesNodeSelected = true;
				mResourcesNodeSelected = false;
				mSelector->clear();
			}
			if (entityTreeOpen)
			{
				drawTree(mModel->getTree().mEntities, mNameColumnOffset + mLayoutConstants->nameColumnIndent());
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();

			if (!mSelector->empty())
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
				if (mController->renameResource(mSelector->get(), mEnteredID))
					mSelector->set(mEnteredID);
				mEnteredID.clear();
				mEditedID.clear();
			}

			// Popup context menu on right click
			std::string chosenPopup;
			ImGui::SetNextWindowBgAlpha(0.1f);
			if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
			{
				IGroup *selectedGroup = nullptr;
				Entity* selectedEntity = nullptr;
				if (!mSelector->empty())
				{
					selectedGroup = rtti_cast<IGroup>(mModel->findResource(mSelector->get()));
					selectedEntity = rtti_cast<Entity>(mModel->findResource(mSelector->get()));
				}

				// When a resource is selected
				if (mResourcesNodeSelected)
				{
					Icon(*mResourceIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Resource..."))
					{
						std::vector<std::string> menuItems;
						for (auto& pair : mModel->getResourceTypes())
							menuItems.push_back(pair.first);
						mFilterMenu.init(std::move(menuItems));
						chosenPopup = "##AddResourcePopup";
					}

					Icon(*mGroupIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Group..."))
					{
						auto selectedGroup = rtti_cast<ResourceGroup>(mModel->findResource(mSelector->get()));
						if (selectedGroup != nullptr)
						{
							auto type = selectedGroup->get_type();
							mController->createGroup(type);
							mSelector->clear();
						} else
						{
							std::vector<std::string> menuItems;
							for (auto& pair : mModel->getGroupTypes())
								menuItems.push_back(pair.first);
							mFilterMenu.init(std::move(menuItems));
							chosenPopup = "##AddGroupPopup";
						}
					}
				}

				// When the main entities node is selected
				if (mEntitiesNodeSelected)
				{
					Icon(*mEntityIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create Entity..."))
						mController->createEntity();
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
						mFilterMenu.init(std::move(menuItems));
						chosenPopup = "##AddResourcePopup";
					}
					// Create child
					Icon(*mGroupIcon, mGuiService);
					ImGui::SameLine();
					if (ImGui::Selectable("Create child..."))
					{
						mController->createGroup(selectedGroup->get_type());
						mSelector->clear();
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
						mFilterMenu.init(std::move(menuItems));
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
							mFilterMenu.init(std::move(entities));
							chosenPopup = "##AddChildEntityPopup";
						}
					}
				}

				// For all selections
				if (!mSelector->empty())
				{
					if (ImGui::Selectable(std::string("Rename " + mSelector->get()).c_str()))
					{
						mEditedID = mSelector->get();
						strcpy(mRenameBuffer, mEditedID.c_str());
					}
					if (ImGui::Selectable(std::string("Remove " + mSelector->get()).c_str()))
					{
						mController->removeResource(mSelector->get());
						mSelector->clear();
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
				if (mFilterMenu.show())
				{
					auto typeName = mFilterMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					mController->createResource(type, mSelector->get());
					mSelector->clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddGroupPopup"))
			{
				if (mFilterMenu.show())
				{
					auto typeName = mFilterMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					mController->createGroup(type, mSelector->get());
					mSelector->clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddChildEntityPopup"))
			{
				if (mFilterMenu.show())
				{
					auto child = mFilterMenu.getSelectedItem();
					if (!mSelector->empty())
						mModel->addEntityToParent(child, mSelector->get());
					mSelector->clear();
				}
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddComponentPopup"))
			{
				if (mFilterMenu.show())
				{
					auto typeName = mFilterMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					mController->createComponent(type, mSelector->get());
					mSelector->clear();
				}
				ImGui::EndPopup();
			}

		}
	}
}
