#include "inspector.h"
#include <model.h>
#include <imguifunctions.h>
#include <imguiservice.h>
#include <imgui/imgui.h>
#include "imgui_internal.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::Inspector)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("ResourceSelector", &nap::edit::Inspector::mResourceSelector, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Controller", &nap::edit::Inspector::mController, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("LayoutConstants", &nap::edit::Inspector::mLayoutConstants, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        Inspector::Inspector(Core& core) : mSelection(), mCore(core)
        {
            mGuiService = core.getService<IMGuiService>();
        }


        bool Inspector::init(utility::ErrorState &errorState)
        {
            mModel = mResourceSelector->mModel.get();

            // Scope for linked property editors
            auto editorTypes = RTTI_OF(IPropertyEditor).get_derived_classes();
            for (auto& editorType : editorTypes)
                if (editorType.can_create_instance())
                {
                    auto editor = editorType.create<IPropertyEditor>();
                    mPropertyEditors[editor->getType()] = std::unique_ptr<IPropertyEditor>(editor);
                }

            mPostResourcesLoadedSlot.setFunction([this](){ onPostResourcesLoaded(); });
            mCore.getResourceManager()->mPostResourcesLoadedSignal.connect(mPostResourcesLoadedSlot);

            return true;
        }


        void Inspector::draw()
        {
            // Draw column headers
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::BeginColumns("##InspectorColumns", 3);
            auto nameOffset = ImGui::GetCursorPosX();
            ImGui::Text("Name");
            ImGui::NextColumn();

            auto valueOffset = ImGui::GetCursorPosX() + mLayoutConstants->columnContentShift();
            ImGui::Text("Value");
            ImGui::NextColumn();

            auto typeOffset = ImGui::GetCursorPosX() + mLayoutConstants->columnContentShift();
            ImGui::Text("Type");
            ImGui::EndColumns();
            ImGui::PopStyleColor();

            if (mResourceSelector->empty())
                return;

            ImGui::SetNextWindowBgAlpha(0.1);
            ImGui::BeginChild("##InspectorChild", ImVec2(0, 0), true);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mLayoutConstants->listOffset());

            // Check if selected resource has changed
            if (mResourceSelector->get() != mInspectedResourceID)
            {
                mSelection.clear();
                mInspectedResourceID = mResourceSelector->get();
                mInspectedResource = mModel->findResource(mResourceSelector->get());
                assert(mInspectedResource != nullptr);
            }

            // Draw selected resource
            rtti::Path path;
            rtti::Variant var = mInspectedResource;
            rtti::TypeInfo type = mInspectedResource->get_type();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
            drawObject(var, type, path, nameOffset, valueOffset, typeOffset);
            ImGui::PopStyleVar();

            ImGui::EndChild();

            // Popup resource selection
            if (mOpenResourceMenu)
            {
                ImGui::OpenPopup("##ChooseResourcePopup");
                mOpenResourceMenu = false;
            }
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::BeginPopup("##ChooseResourcePopup"))
            {
                if (mFilteredMenu.show())
                {
                    auto resource = mModel->findResource(mFilteredMenu.getSelectedItem());
                    if (mSelection.isPointer())
                        mController->setValue(mSelection, resource);
                    else if (mSelection.isArrayElement())
                        mController->insertArrayElement(mSelection, resource);
                    else if (mSelection.isArray())
                        mController->insertArrayElement(mSelection, resource);
                }
                ImGui::EndPopup();
            }

            // Popup resource type selection
            if (mOpenResourceTypeMenu)
            {
                ImGui::OpenPopup("##ChooseResourceTypePopup");
                mOpenResourceTypeMenu = false;
            }
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::BeginPopup("##ChooseResourceTypePopup"))
            {
                if (mFilteredMenu.show())
                    mController->createEmbeddedObject(mSelection, rtti::TypeInfo::get_by_name(mFilteredMenu.getSelectedItem()));
                ImGui::EndPopup();
            }

            // Popup context menu
            drawContextMenu();
        }


        void Inspector::drawContextMenu()
        {
            // Popup context menu on right click
            if (mSelection.isResolved())
            {
                if (mSelection.isArrayElement())
                {
                    ImGui::SetNextWindowBgAlpha(0.5f);
                    if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
                    {
                        if (ImGui::Selectable("Remove Element"))
                            removeArrayElement();

                        if (ImGui::Selectable("Insert Element"))
                            insertArrayElement();

                        auto array = mSelection.getResolvedPath().getValue();
                        auto view = array.create_array_view();
                        if (mSelection.getArrayIndex() < view.get_size() - 1)
                            if (ImGui::Selectable("Move Element Down"))
                                moveArrayElementDown();

                        if (mSelection.getArrayIndex() > 0)
                            if (ImGui::Selectable("Move Element Up"))
                                moveArrayElementUp();

                        ImGui::EndPopup();
                    }
                }
                else if (mSelection.isArray())
                {
                    if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
                    {
                        if (ImGui::Selectable("Add Element"))
                            addArrayElement();

                        ImGui::EndPopup();
                    }
                }
            }
        }


        bool Inspector::drawObject(rtti::Variant& object, rtti::TypeInfo type, const rtti::Path& path, float nameOffset, float valueOffset, float typeOffset)
        {
            bool changed = false;
            for (auto& property : type.get_properties())
            {
                auto propertyValue = property.get_value(object);
                auto propertyType = property.get_type();
                auto propertyName = property.get_name().to_string();
                bool embeddedPointer = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Embedded);

                if (drawValue(propertyValue, propertyType, path, propertyName, false, 0, embeddedPointer, nameOffset, valueOffset, typeOffset))
                {
                    property.set_value(object, propertyValue);
                    changed = true;
                }
            }
            return changed;
        }


        bool Inspector::drawValue(rtti::Variant &value, rtti::TypeInfo type, const rtti::Path& parentPath, const std::string &name, bool isArrayElement, int arrayIndex, bool isEmbeddedPointer, float nameOffset, float valueOffset, float typeOffset)
        {
            auto path = parentPath;
            if (isArrayElement)
                path.pushArrayElement(arrayIndex);
            else
                path.pushAttribute(name);

            bool valueChanged = false;
            bool opened = false;
            auto valueWidth = typeOffset - valueOffset - mLayoutConstants->valueSpacing();
            auto propertyEditor = mPropertyEditors.find(type);

            // Draw tree node for objects and arrays
            bool isCollapsedObject = propertyEditor == mPropertyEditors.end() && type.is_class() && !type.is_wrapper();
            bool isEmbeddedObject = type.is_derived_from<rtti::ObjectPtrBase>() && isEmbeddedPointer;
            if (isCollapsedObject || isEmbeddedObject)
            {
                ImGui::SetCursorPosX(nameOffset + mLayoutConstants->treeNodeArrowShift());
                std::string label = "###" + name;
                opened = TreeNodeArrow(label.c_str());
                ImGui::SameLine();
            }
            else
                ImGui::SetCursorPosX(nameOffset);

            // Draw name
            bool selected = (mSelection.getPath() == path);
            if (Selectable(name.c_str(), selected, valueOffset - ImGui::GetCursorPosX() - mLayoutConstants->valueSpacing()))
            {
                if (isArrayElement)
                    mSelection.set(parentPath, arrayIndex, mInspectedResource.get());
                else
                    mSelection.set(path, mInspectedResource.get());
            }
            ImGui::SameLine();

            ImGui::SetCursorPosX(valueOffset);
            if (name == "mID")
            {
                // Draw mID input field
                drawID(value, parentPath, valueWidth);
            }
            else if (propertyEditor != mPropertyEditors.end())
            {
                // Draw property editor
                std::string label = "##" + parentPath.toString() + name;
                if (propertyEditor->second->drawValue(value, label, valueWidth))
                    valueChanged = true;
            }
            else if (type.is_enumeration())
            {
                // Draw enum
                if (drawEnum(value, type, path, name, valueWidth))
                    valueChanged = true;
            }
            else if (type.is_derived_from<rtti::ObjectPtrBase>())
            {
                // Draw resource pointer
                if (drawPointer(value, type, path, name, isEmbeddedPointer, valueWidth))
                    valueChanged = true;
            }
            else if (!type.is_array() && !type.is_class())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                ImGui::Text("No editor for type");
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();

            // Draw type
            ImGui::SetCursorPosX(typeOffset);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::Text(type.get_name().to_string().c_str());
            ImGui::PopStyleColor();

            if (opened)
            {
                if (type.is_array())
                {
                    // Draw array elements
                    if (drawArray(value, path, name, isEmbeddedPointer, nameOffset + mLayoutConstants->nameColumnIndent(), valueOffset, typeOffset))
                        valueChanged = true;
                }
                else if (isEmbeddedObject)
                {
                    // Draw nested embedded pointer object
                    assert(value.get_type().is_wrapper());
                    rtti::Object* embeddedObject = value.get_value<rtti::ObjectPtr<rtti::Object>>().get();
                    if (embeddedObject != nullptr)
                    {
                        rtti::Variant var = embeddedObject;
                        if (drawObject(var, embeddedObject->get_type(), path, nameOffset + mLayoutConstants->nameColumnIndent(), valueOffset, typeOffset))
                        {
                            valueChanged = true;
                        }
                    }
                }
                else {
                    // Draw nested object
                    if (drawObject(value, type, path, nameOffset + mLayoutConstants->nameColumnIndent(), valueOffset, typeOffset))
                        valueChanged = true;
                }
                ImGui::TreePop();
            }
            return valueChanged;
        }


        bool Inspector::drawArray(rtti::Variant &var, const rtti::Path& path, const std::string &name, bool isEmbeddedPointerArray, float nameOffset, float valueOffset, float typeOffset)
        {
            bool valueChanged = false;
            assert(var.is_array());
            auto array = var.create_array_view();
            for (auto i = 0; i < array.get_size(); ++i)
            {
                auto element = array.get_value(i);
                if (drawValue(element, element.get_type(), path, std::to_string(i), true, i, isEmbeddedPointerArray, nameOffset, valueOffset, typeOffset))
                {
                    array.set_value(i, element);
                    valueChanged = true;
                }
            }
            return valueChanged;
        }


        bool Inspector::drawEnum(rtti::Variant &var, rtti::TypeInfo type, const rtti::Path& path, const std::string &name, float valueWidth)
        {
            bool valueChanged = false;
            auto enumeration = type.get_enumeration();
            auto names = enumeration.get_names();
            ImGui::SetNextItemWidth(valueWidth);
            auto label = "##" + path.toString() + name;
            if (ImGui::BeginCombo(label.c_str(), enumeration.value_to_name(var).to_string().c_str()))
            {
                for (auto& name : names)
                {
                    if (ImGui::Selectable(name.to_string().c_str()))
                    {
                        var = enumeration.name_to_value(name);
                        valueChanged = true;
                    }
                }
                ImGui::EndCombo();
            }
            return valueChanged;
        }


        bool Inspector::drawPointer(rtti::Variant &var, rtti::TypeInfo type, const rtti::Path &path, const std::string& name, bool isEmbedded, float valueWidth)
        {
            assert(var.get_type().is_wrapper());
            rtti::Object* resource = var.get_value<rtti::ObjectPtr<rtti::Object>>().get();

            std::string label = resource == nullptr ? "Not set" : resource->mID;
            auto x = ImGui::GetCursorPosX();
            ImGui::SetNextItemWidth(valueWidth - mLayoutConstants->pointerEditorButtonWidth());
            ImGui::Text(label.c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(x + valueWidth - mLayoutConstants->pointerEditorButtonWidth());
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

            if (isEmbedded)
            {
                if (resource == nullptr)
                {
                    label = "Create##" + path.toString() + name;
                    if (ImGui::Button(label.c_str(), ImVec2(mLayoutConstants->pointerEditorButtonWidth(), ImGui::GetFrameHeight())))
                    {
                        mSelection.set(path, mInspectedResource.get());
                        createEmbeddedObject(type);
                    }
                }
                else {
                    label = "Remove##" + path.toString() + name;
                    if (ImGui::Button(label.c_str(), ImVec2(mLayoutConstants->pointerEditorButtonWidth(), ImGui::GetFrameHeight())))
                    {
                        mSelection.set(path, mInspectedResource.get());
                        mController->removeEmbeddedObject(mSelection);
                    }
                }
            }
            else {
                if (resource == nullptr)
                {
                    label = "Set##" + path.toString() + name;
                    if (ImGui::Button(label.c_str(), ImVec2(mLayoutConstants->pointerEditorButtonWidth(), ImGui::GetFrameHeight())))
                    {
                        mSelection.set(path, mInspectedResource.get());
                        choosePointer(type);
                    }
                }
                else {
                    label = "Clear##" + path.toString() + name;
                    if (ImGui::Button(label.c_str(), ImVec2(mLayoutConstants->pointerEditorButtonWidth(), ImGui::GetFrameHeight())))
                    {
                        mSelection.set(path, mInspectedResource.get());
                        mController->setValue(mSelection, nullptr);
                    }
                }
            }

            ImGui::PopStyleVar();
            return false;
        }


        void Inspector::drawID(rtti::Variant &value, const rtti::Path& parentPath, float width)
        {
            auto oldID = value.to_string();
            std::string label = "##ID" + parentPath.toString();
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "%s", oldID.c_str());
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                auto newID = std::string(buffer);
                mModel->renameResource(oldID, newID);
                if (parentPath.getLength() == 0) // Are we editing the ID of the selected resource?
                    mResourceSelector->set(value.to_string());
            }
        }


        void Inspector::insertArrayElement()
        {
            auto array = mSelection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto elementType = view.get_rank_type(1);
            if (elementType.is_derived_from<rtti::ObjectPtrBase>())
            {
                if (rtti::hasFlag(mSelection.getResolvedPath().getProperty(), rtti::EPropertyMetaData::Embedded))
                    createEmbeddedObject(elementType);
                else
                    choosePointer(elementType);
            }
            else
            {
                mController->insertArrayElement(mSelection);
            }
        }


        void Inspector::removeArrayElement()
        {
            mController->removeArrayElement(mSelection);
            mSelection.clear();
        }


        void Inspector::moveArrayElementUp()
        {
            mController->moveArrayElementUp(mSelection);
            auto arrayPath = mSelection.getPath();
            mSelection.set(arrayPath, mSelection.getArrayIndex() - 1, mInspectedResource.get());
        }


        void Inspector::moveArrayElementDown()
        {
            mController->moveArrayElementDown(mSelection);
            auto arrayPath = mSelection.getPath();
            mSelection.set(arrayPath, mSelection.getArrayIndex() + 1, mInspectedResource.get());
        }


        void Inspector::addArrayElement()
        {
            auto array = mSelection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto elementType = view.get_rank_type(1);
            if (elementType.is_derived_from<rtti::ObjectPtrBase>())
            {
                if (rtti::hasFlag(mSelection.getResolvedPath().getProperty(), rtti::EPropertyMetaData::Embedded))
                    createEmbeddedObject(elementType);
                else
                    choosePointer(elementType);
            }
            else {
                assert(elementType.can_create_instance());
                auto element = elementType.create();
                mController->insertArrayElement(mSelection, element);
            }
        }


        void Inspector::addArrayPtrElement(Resource *resource)
        {
            mController->insertArrayElement(mSelection, resource);
        }


        void Inspector::choosePointer(const rtti::TypeInfo& type)
        {
            auto& resources = mModel->getResources();
            auto targetType = type.get_wrapped_type().get_raw_type();
            std::vector<std::string> menuItems;
            for (auto& resource : resources)
            {
                auto resourceType = resource->get_type().get_raw_type();
                if (resourceType.is_derived_from(targetType) || resourceType == targetType)
                    menuItems.emplace_back(resource->mID);
            }
            mFilteredMenu.init(std::move(menuItems));
            mOpenResourceMenu = true;
        }


        void Inspector::createEmbeddedObject(const rtti::TypeInfo &type)
        {
            auto targetType = type.get_wrapped_type().get_raw_type();
            std::vector<std::string> menuItems;
            for (auto& pair : mModel->getResourceTypes())
                if (pair.second->is_derived_from(targetType) || *pair.second == targetType)
                    menuItems.push_back(pair.first);
            mFilteredMenu.init(std::move(menuItems));
            mOpenResourceTypeMenu = true;
        }


        void Inspector::onPostResourcesLoaded()
        {
            if (!mInspectedResourceID.empty())
            {
                mInspectedResource = mModel->findResource(mInspectedResourceID);
                assert(mInspectedResource != nullptr);
                mSelection.clear();
            }
        }


    }

}