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
                if (propertyEditor != mPropertyEditors.end())
                {
                    propertyEditor->second->show(nameOffset, valueOffset, typeOffset, object, property);
                    if (property.get_name().to_string() == "mID")
                        mResourceList->setSelectedID(property.get_value(object).to_string());
                }
                else if (propertyType.is_class())
                {
                    if (propertyType.is_array())
                    {

                    }
                    else if (propertyType.is_wrapper())
                    {

                    }
                    else {
                        std::string label = "###" + property.get_name().to_string();
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.f, 0.f, 0.f, 0.f));
                        bool opened = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap);
                        ImGui::PopStyleColor();
                        ImGui::PopStyleColor();
                        ImGui::SameLine();

                        ImGui::Text(property.get_name().to_string().c_str());
                        ImGui::SameLine();

                        ImGui::SetCursorPosX(typeOffset);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                        ImGui::Text(propertyType.get_name().to_string().c_str());
                        ImGui::PopStyleColor();

                        if (opened)
                        {
                            auto nestedObject = property.get_value(object);
                            drawObject(nestedObject, property.get_type(), nameOffset, valueOffset, typeOffset);
                            property.set_value(object, nestedObject);
                            ImGui::TreePop();
                        }
                    }
                }
                else {
                    ImGui::SetCursorPosX(nameOffset);
                    ImGui::Text(property.get_name().to_string().c_str());
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
                    ImGui::SetCursorPosX(valueOffset);
                    ImGui::Text("No editor for type");

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(typeOffset);
                    ImGui::Text(propertyType.get_name().to_string().c_str());
                    ImGui::PopStyleColor();
                }
            }
        }

    }

}