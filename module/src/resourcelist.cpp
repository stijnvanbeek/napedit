#include "resourcelist.h"

#include "nap/logger.h"

// #include "imgui_internal.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::ResourceList)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Model", &nap::edit::ResourceList::mModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace edit
	{
		ResourceList::ResourceList(Core &core): mCore(core)
		{
			memset(mRenameBuffer, 0, sizeof(mRenameBuffer));
		}


		bool ResourceList::init(utility::ErrorState &errorState)
		{
			auto groupBase = RTTI_OF(IGroup);
			auto allGroups = groupBase.get_derived_classes();
			for (auto &group: allGroups)
				if (mCore.getResourceManager()->getFactory().canCreate(group))
					mGroupTypes[group.get_name().to_string()] = &group;

			auto resourceBase = RTTI_OF(Resource);
			auto allResources = resourceBase.get_derived_classes();
			for (auto &resource: allResources)
				if (mCore.getResourceManager()->getFactory().canCreate(resource))
					if (mGroupTypes.find(resource.get_name().to_string()) == mGroupTypes.end())
						mResourceTypes[resource.get_name().to_string()] = &resource;

			return true;
		}


		void ResourceList::draw()
		{
			ImGui::SetNextWindowBgAlpha(0.5f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));

			ImGui::BeginColumns("##ResourcesListColumns", 2);
			mNameColumnOffset = ImGui::GetCursorPosX();
			ImGui::Text("Name");

			ImGui::NextColumn();
			mTypeColumnOffset = ImGui::GetCursorPosX() - 30;
			ImGui::Text("Type");
			ImGui::EndColumns();
			ImGui::PopStyleColor();

			// List of all resources
			ImGui::BeginChild("##ResourcesListBox", ImVec2(0, 0), true);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
			drawTree(mModel->getTree().mGroups, mNameColumnOffset);
			drawTree(mModel->getTree().mResources, mNameColumnOffset);
			ImGui::PopStyleVar();

			ImGui::EndChild();

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

			// Popup when right clicked on the resources list
			std::string chosenPopup;
			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
			{
				IGroup *selectedGroup = nullptr;
				if (!mSelectedID.empty())
					selectedGroup = rtti_cast<IGroup>(mModel->findResource(mSelectedID));

				// For groups
				if (selectedGroup != nullptr)
				{
					auto type = selectedGroup->getMemberType();
					// Create member
					if (ImGui::Selectable("Create member..."))
					{
						std::vector<std::string> menuItems;
						for (auto& pair : mResourceTypes)
							if (pair.second->is_derived_from(type))
								menuItems.push_back(pair.first);
						mTypeMenu.init(std::move(menuItems));
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
						std::vector<std::string> menuItems;
						for (auto& pair : mResourceTypes)
							menuItems.push_back(pair.first);
						mTypeMenu.init(std::move(menuItems));
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
						} else
						{
							std::vector<std::string> menuItems;
							for (auto& pair : mGroupTypes)
								menuItems.push_back(pair.first);
							mTypeMenu.init(std::move(menuItems));
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

			// Popup sub menus
			if (!chosenPopup.empty())
				ImGui::OpenPopup(chosenPopup.c_str());
			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::BeginPopup("##AddResourcePopup"))
			{
				if (mTypeMenu.show())
				{
					auto typeName = mTypeMenu.getSelectedItem();
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
				if (mTypeMenu.show())
				{
					auto typeName = mTypeMenu.getSelectedItem();
					auto type = rtti::TypeInfo::get_by_name(typeName);
					auto mID = mModel->createGroup(type, typeName);
					if (!mSelectedID.empty())
						mModel->moveGroupToParent(mID, mSelectedID);
					mSelectedID.clear();
				}
				ImGui::EndPopup();
			}
		}
	}
}
