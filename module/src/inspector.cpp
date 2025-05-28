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


        void Inspector::drawObject(rtti::Variant& object, rtti::TypeInfo type, float nameOffset, float valueOffset, float typeOffset)
        {
            for (auto& property : type.get_properties())
            {
                auto propertyType = property.get_type();
                auto propertyEditor = mPropertyEditors.find(propertyType);
                bool opened = false;

                // Draw tree node for objects and arrays
                if (propertyEditor == mPropertyEditors.end() && propertyType.is_class() && !propertyType.is_wrapper())
                {
                    std::string label = "###" + property.get_name().to_string();
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
                ImGui::Text(property.get_name().to_string().c_str());
                ImGui::SameLine();

                // Draw editor
                ImGui::SetCursorPosX(valueOffset);
                auto width = typeOffset - valueOffset - ImGui::GetStyle().FramePadding.x * 2;
                if (propertyEditor != mPropertyEditors.end())
                {
                    propertyEditor->second->drawValue(width, object, property);
                    if (property.get_name().to_string() == "mID")
                        mResourceList->setSelectedID(property.get_value(object).to_string());
                }
                else if (propertyType.is_enumeration())
                {
                    auto enumeration = property.get_enumeration();
                    auto names = enumeration.get_names();
                    ImGui::SetNextItemWidth(width);
                    auto label = "###" + property.get_name().to_string();
                    if (ImGui::BeginCombo(label.c_str(), enumeration.value_to_name(property.get_value(object)).to_string().c_str()))
                    {
                        for (auto& name : names)
                        {
                            if (ImGui::Selectable(name.to_string().c_str()))
                                property.set_value(object, enumeration.name_to_value(name));
                        }
                        ImGui::EndCombo();
                    }
                }
                else if (propertyType.is_class() && propertyType.is_wrapper())
                {
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
                ImGui::Text(propertyType.get_name().to_string().c_str());
                ImGui::PopStyleColor();

                if (opened)
                {
                    if (propertyType.is_array())
                    {
                        auto label = "###" + property.get_name().to_string();
                        auto var = property.get_value(object);
                        assert(var.is_array());
                        auto array = var.create_array_view();
                        auto size = array.get_size();
                        for (auto i = 0; i < array.get_size(); ++i)
                        {
                            auto element = array.get_value(i);
                            drawObject(element, element.get_type(), nameOffset + 25, valueOffset, typeOffset);
                        }
                    }
                    else
                    {
                        auto nestedObject = property.get_value(object);
                        drawObject(nestedObject, property.get_type(), nameOffset + 25, valueOffset, typeOffset);
                        property.set_value(object, nestedObject);
                    }
                    ImGui::TreePop();
                }
            }
        }

    }

}