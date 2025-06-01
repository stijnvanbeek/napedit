#include "inspector.h"

RTTI_BEGIN_CLASS(nap::edit::Inspector)
    RTTI_PROPERTY("Model", &nap::edit::Inspector::mModel, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ResourceListGui", &nap::edit::Inspector::mResourceList, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        bool Inspector::init(utility::ErrorState &errorState)
        {
            // Scope for linked property editors
            auto allPropertyEditors = RTTI_OF(IPropertyEditor).get_derived_classes();
            for (auto& type : allPropertyEditors)
                if (type.can_create_instance())
                {
                    auto instance = type.create<IPropertyEditor>();
                    mPropertyEditors[instance->getType()] = std::unique_ptr<IPropertyEditor>(instance);
                }

            return true;
        }


        void Inspector::draw()
        {
            // Draw column headers
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::BeginColumns("###InspectorColumns", 3);
            auto nameOffset = ImGui::GetCursorPosX();
            ImGui::Text("Name");
            ImGui::NextColumn();

            auto valueOffset = ImGui::GetCursorPosX() - 30;
            ImGui::Text("Value");
            ImGui::NextColumn();

            auto typeOffset = ImGui::GetCursorPosX() - 30;
            ImGui::Text("Type");
            ImGui::EndColumns();
            ImGui::PopStyleColor();

            if (mResourceList->getSelectedID().empty())
                return;

            ImGui::BeginChild("###InspectorChild", ImVec2(0, 0), true);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);

            // Check if selected resource has changed
            if (mResourceList->getSelectedID() != mSelectedResourceID)
            {
                mSelection.clear();
                mSelectedResourceID = mResourceList->getSelectedID();
                mSelectedResource = mModel->findResource(mResourceList->getSelectedID());
                assert(mSelectedResource != nullptr);
            }

            // Draw selected resource
            rtti::Path path;
            rtti::Variant var = mSelectedResource;
            rtti::TypeInfo type = mSelectedResource->get_type();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
            drawObject(var, type, path, nameOffset, valueOffset, typeOffset);
            ImGui::PopStyleVar();

            ImGui::EndChild();

            // Popup context menu
            drawContextMenu();
        }


        void Inspector::drawContextMenu()
        {
            // Popup context menu on right click
            if (mSelection.isValid())
            {
                if (mSelection.isArrayElement())
                {
                    if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
                    {
                        if (ImGui::Selectable("Remove Element"))
                            removeArrayElement(mSelection);

                        if (ImGui::Selectable("Insert Element"))
                            insertArrayElement(mSelection);

                        auto array = mSelection.getResolvedPath().getValue();
                        auto view = array.create_array_view();
                        if (mSelection.getArrayIndex() < view.get_size() - 1)
                            if (ImGui::Selectable("Move Element Down"))
                                moveArrayElementDown(mSelection);

                        if (mSelection.getArrayIndex() > 0)
                            if (ImGui::Selectable("Move Element Up"))
                                moveArrayElementUp(mSelection);

                        ImGui::EndPopup();
                    }
                }
                else if (mSelection.isArray())
                {
                    if (ImGui::BeginPopupContextItem("##ResourcesListPopupContextItem", ImGuiMouseButton_Right))
                    {
                        if (ImGui::Selectable("Add Element"))
                            addArrayElement(mSelection);

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

                if (drawValue(propertyValue, propertyType, path, propertyName, false, 0, nameOffset, valueOffset, typeOffset))
                {
                    property.set_value(object, propertyValue);
                    changed = true;
                }
            }
            return changed;
        }


        bool Inspector::drawValue(rtti::Variant &value, rtti::TypeInfo type, const rtti::Path& parentPath, const std::string &name, bool isArrayElement, int arrayIndex, float nameOffset, float valueOffset, float typeOffset)
        {
            auto path = parentPath;
            if (isArrayElement)
                path.pushArrayElement(arrayIndex);
            else
                path.pushAttribute(name);

            bool valueChanged = false;
            bool opened = false;
            auto valueWidth = typeOffset - valueOffset - 50;
            auto propertyEditor = mPropertyEditors.find(type);

            // Draw tree node for objects and arrays
            if (propertyEditor == mPropertyEditors.end() && type.is_class() && !type.is_wrapper())
            {
                std::string label = "###" + name;
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.f, 0.f, 0.f, 0.f));
                opened = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap);
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::SameLine();
            }
            else
                ImGui::SetCursorPosX(nameOffset);

            // Draw name selectable
            bool selected = (mSelection.getPath() == path);
            if (ImGui::Selectable(name.c_str(), selected, ImGuiSelectableFlags_SpanAvailWidth | ImGuiSelectableFlags_AllowItemOverlap) ||
                (ImGui::IsItemClicked(1)))
            {
                if (isArrayElement)
                    mSelection.set(parentPath, arrayIndex, mSelectedResource);
                else
                    mSelection.set(path, mSelectedResource);
            }
            ImGui::SameLine();

            // Draw property editor
            ImGui::SetCursorPosX(valueOffset);
            if (propertyEditor != mPropertyEditors.end())
            {
                std::string label = "###" + name;
                if (propertyEditor->second->drawValue(value, label, valueWidth))
                {
                    valueChanged = true;
                    if (name == "mID")
                        mResourceList->setSelectedID(value.to_string());
                }
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
                    if (drawArray(value, path, name, nameOffset + 25, valueOffset, typeOffset))
                        valueChanged = true;
                }
                else {
                    // Draw nested object
                    if (drawObject(value, type, path, nameOffset + 25, valueOffset, typeOffset))
                        valueChanged = true;
                }
                ImGui::TreePop();
            }
            return valueChanged;
        }


        bool Inspector::drawArray(rtti::Variant &var, const rtti::Path& path, const std::string &name, float nameOffset, float valueOffset, float typeOffset)
        {
            bool valueChanged = false;
            assert(var.is_array());
            auto array = var.create_array_view();
            for (auto i = 0; i < array.get_size(); ++i)
            {
                auto element = array.get_value(i);
                if (drawValue(element, element.get_type(), path, std::to_string(i), true, i, nameOffset + 25, valueOffset, typeOffset))
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
            auto label = "###" + name;
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


        void Inspector::insertArrayElement(Selection &selection)
        {
            auto array = selection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto elementType = view.get_rank_type(1);
            assert(elementType.can_create_instance());
            auto element = elementType.create();
            assert(selection.getArrayIndex() <= view.get_size());
            view.insert_value(selection.getArrayIndex(), element);
            selection.getResolvedPath().setValue(array);
            auto arrayPath = mSelection.getPath();
            arrayPath.popBack();
            selection.set(arrayPath, mSelection.getArrayIndex(), mSelectedResource);
        }


        void Inspector::removeArrayElement(Selection &selection)
        {
            auto array = selection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            assert(array.is_array());
            view.remove_value(mSelection.getArrayIndex());
            selection.getResolvedPath().setValue(array);
            selection.clear();
        }


        void Inspector::moveArrayElementUp(Selection &selection)
        {
            auto array = selection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto element = view.get_value(selection.getArrayIndex());
            view.remove_value(selection.getArrayIndex());
            view.insert_value(selection.getArrayIndex() - 1, element);
            selection.getResolvedPath().setValue(array);
            auto arrayPath = selection.getPath();
            arrayPath.popBack();
            selection.set(arrayPath, selection.getArrayIndex() - 1, mSelectedResource);
        }


        void Inspector::moveArrayElementDown(Selection &selection)
        {
            auto array = selection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto element = view.get_value(selection.getArrayIndex());
            view.remove_value(selection.getArrayIndex());
            view.insert_value(selection.getArrayIndex() + 1, element);
            selection.getResolvedPath().setValue(array);
            auto arrayPath = selection.getPath();
            arrayPath.popBack();
            selection.set(arrayPath, selection.getArrayIndex() + 1, mSelectedResource);
        }


        void Inspector::addArrayElement(Selection &selection)
        {
            auto array = selection.getResolvedPath().getValue();
            auto view = array.create_array_view();
            auto elementType = view.get_rank_type(1);
            assert(elementType.can_create_instance());
            auto element = elementType.create();
            view.insert_value(view.get_size(), element);
            selection.getResolvedPath().setValue(array);
        }


        void Inspector::Selection::set(const rtti::Path &path, Resource *root)
        {
            mPath = path;
            mIsArrayElement = false;
            mIsValid = mPath.resolve(root, mResolvedPath);
            assert(mIsValid);
        }


        void Inspector::Selection::set(const rtti::Path &arrayPath, int index, Resource *root)
        {
            mPath = arrayPath;
            mIsArrayElement = true;
            mArrayIndex = index;
            mIsValid = mPath.resolve(root, mResolvedPath);
            mPath.pushArrayElement(index);
            assert(mIsValid);
            assert(mResolvedPath.getType().is_array());
        }


    }

}