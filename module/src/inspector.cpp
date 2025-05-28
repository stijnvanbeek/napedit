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
            ImGui::BeginChild("###InspectorChild", ImVec2(0, 0), false);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::BeginColumns("###InspectorColumns", 3);
            auto nameOffset = ImGui::GetCursorPosX();
            ImGui::Text("Name");
            ImGui::NextColumn();

            mValueColumnOffset = ImGui::GetCursorPosX();
            auto valueOffset = ImGui::GetCursorPosX();
            ImGui::Text("Value");
            ImGui::NextColumn();

            mTypeColumnOffset = ImGui::GetCursorPosX();
            auto typeOffset = ImGui::GetCursorPosX();
            ImGui::Text("Type");
            ImGui::EndColumns();
            ImGui::PopStyleColor();

            if (mResourceList->getSelectedID().empty())
            {
                ImGui::EndChild();
                return;
            }

            // Draw selected resource
            auto resource = mModel->findResource(mResourceList->getSelectedID());
            assert(resource != nullptr);
            rtti::Variant var = resource;
            rtti::TypeInfo type = resource->get_type();
            drawObject(var, type, nameOffset, valueOffset, typeOffset);

            ImGui::EndChild();
        }


        bool Inspector::drawObject(rtti::Variant& object, rtti::TypeInfo type, float nameOffset, float valueOffset, float typeOffset)
        {
            bool changed = false;
            for (auto& property : type.get_properties())
            {
                auto propertyValue = property.get_value(object);
                auto propertyType = property.get_type();
                auto propertyName = property.get_name().to_string();

                if (drawValue(propertyValue, propertyType, propertyName, nameOffset, valueOffset, typeOffset))
                {
                    property.set_value(object, propertyValue);
                    changed = true;
                }
            }
            return changed;
        }


        bool Inspector::drawValue(rtti::Variant &value, rtti::TypeInfo type, const std::string &name, float nameOffset,
            float valueOffset, float typeOffset)
        {
            bool valueChanged = false;
            bool opened = false;
            auto valueWidth = typeOffset - valueOffset - ImGui::GetStyle().FramePadding.x * 2;
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

            // Draw name
            ImGui::Text(name.c_str());
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
                if (drawEnum(value, type, name, valueWidth))
                    valueChanged = true;
            }
            else if (type.is_class() && type.is_wrapper())
            {
                // Pointer
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                ImGui::Text("No editor for pointer");
                ImGui::PopStyleColor();
            }
            else if (!opened)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                ImGui::Text("No editor for type");
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();

            // Draw type
            ImGui::SetCursorPosX(typeOffset);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
            ImGui::Text(type.get_name().to_string().c_str());
            ImGui::PopStyleColor();

            if (opened)
            {
                if (type.is_array())
                {
                    // Draw array elements
                    if (drawArray(value, name, nameOffset + 25, valueOffset, typeOffset))
                        valueChanged = true;
                }
                else {
                    // Draw nested object
                    if (drawObject(value, type, nameOffset + 25, valueOffset, typeOffset))
                        valueChanged = true;
                }
                ImGui::TreePop();
            }
            return valueChanged;
        }


        bool Inspector::drawArray(rtti::Variant &var, const std::string &name, float nameOffset, float valueOffset,
            float typeOffset)
        {
            bool valueChanged = false;
            assert(var.is_array());
            auto array = var.create_array_view();
            for (auto i = 0; i < array.get_size(); ++i)
            {
                auto element = array.get_value(i);
                if (drawValue(element, element.get_type(), std::to_string(i), nameOffset + 25, valueOffset, typeOffset))
                {
                    array.set_value(i, element);
                    valueChanged = true;
                }
            }
            return valueChanged;
        }


        bool Inspector::drawEnum(rtti::Variant &var, rtti::TypeInfo type, const std::string &name, float valueWidth)
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


    }

}