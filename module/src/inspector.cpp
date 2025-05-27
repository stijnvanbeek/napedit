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
            auto type = resource->get_type();
            for (auto& property : type.get_properties())
            {
                auto propertyType = property.get_type();
                auto propertyEditor = mPropertyEditors.find(propertyType);
                if (propertyEditor != mPropertyEditors.end())
                {
                    rtti::Variant var = resource;
                    propertyEditor->second->show(nameOffset, valueOffset, typeOffset, var, property);
                    if (property.get_name().to_string() == "mID")
                        mResourceList->setSelectedID(property.get_value(var).to_string());
                }
                else {
                    ImGui::SetCursorPosX(nameOffset);
                    ImGui::Text("No editor for property type '%s'", propertyType.get_name().to_string().c_str());
                }
            }


            ImGui::EndChild();
        }

    }

}