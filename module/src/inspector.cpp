#include "inspector.h"

RTTI_BEGIN_CLASS(nap::edit::Inspector)
    RTTI_PROPERTY("Model", &nap::edit::Inspector::mModel, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ResourceListGui", &nap::edit::Inspector::mResourceListGui, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace edit
    {

        void Inspector::draw()
        {
            ImGui::BeginChild("###InspectorChild", ImVec2(0, 0), false);
            ImGui::Text("Inspector");

            if (mResourceListGui->getSelectedID().empty())
            {
                ImGui::EndChild();
                return;
            }

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

            auto resource = mModel->findResource(mResourceListGui->getSelectedID());
            assert(resource != nullptr);
            auto type = resource->get_type();
            for (auto& property : type.get_properties())
            {
                ImGui::SetCursorPosX(nameOffset);
                ImGui::Text(property.get_name().to_string().c_str());
                ImGui::SameLine();
                ImGui::SetCursorPosX(typeOffset);
                ImGui::Text(property.get_type().get_name().to_string().c_str());
            }


            ImGui::EndChild();
        }

    }

}