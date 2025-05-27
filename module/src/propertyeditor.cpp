#include "propertyeditor.h"

RTTI_DEFINE_CLASS(nap::edit::PropertyEditorString)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorBool)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorFloat)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorDouble)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorInt)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorVec2)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorVec3)
RTTI_DEFINE_CLASS(nap::edit::PropertyEditorVec4)

namespace nap
{

    namespace edit
    {

        void PropertyEditorSimple::show(float nameOffset, float valueOffset, float typeOffset, rtti::Variant &var, const rtti::Property &property)
        {
            ImGui::SetCursorPosX(nameOffset);
            ImGui::Text(property.get_name().to_string().c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(valueOffset - ImGui::GetStyle().FramePadding.x);
            drawValue(typeOffset - valueOffset - ImGui::GetStyle().FramePadding.x * 2, var, property);
            ImGui::SameLine();
            ImGui::SetCursorPosX(typeOffset);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
            ImGui::Text(property.get_type().get_name().to_string().c_str());
            ImGui::PopStyleColor();
        }


        void PropertyEditorString::drawValue(float width, rtti::Variant &var, const rtti::Property &property)
        {
            std::string value = property.get_value(var).to_string();
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "%s", value.c_str());
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
                property.set_value(var, std::string(buffer));
        }


        void PropertyEditorBool::drawValue(float width, rtti::Variant &var, const rtti::Property &property)
        {
            bool value = property.get_value(var).to_bool();
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::Checkbox(name.c_str(), &value))
                property.set_value(var, value);
        }


        void PropertyEditorFloat::drawValue(float width, rtti::Variant &var, const rtti::Property &property)
        {
            float value = property.get_value(var).to_float();
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputFloat(name.c_str(), &value, 0.f, 0.f, "%.3f"))
                property.set_value(var, value);
        }


        void PropertyEditorDouble::drawValue(float width, rtti::Variant &var, const rtti::Property &property)
        {
            double value = property.get_value(var).to_double();
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputDouble(name.c_str(), &value, 0.f, 0.f, "%.3f"))
                property.set_value(var, value);
        }


        void PropertyEditorInt::drawValue(float width, rtti::Variant &var, const rtti::Property &property)
        {
            int value = property.get_value(var).to_int();
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputInt(name.c_str(), &value, 1, 1))
                property.set_value(var, value);
        }


        void PropertyEditorVec2::drawValue(float width, rtti::Variant &object, const rtti::Property &property)
        {
            auto value = property.get_value(object).get_value<glm::vec2>();
            float float2[2] = {value.x, value.y};
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputFloat2(name.c_str(), float2, "%.3f"))
            {
                value.x = float2[0];
                value.y = float2[1];
                property.set_value(object, value);
            }
        }


        void PropertyEditorVec3::drawValue(float width, rtti::Variant &object, const rtti::Property &property)
        {
            auto value = property.get_value(object).get_value<glm::vec3>();
            float float3[3] = {value.x, value.y, value.z};
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputFloat3(name.c_str(), float3, "%.3f"))
            {
                value.x = float3[0];
                value.y = float3[1];
                value.z = float3[2];
                property.set_value(object, value);
            }
        }


        void PropertyEditorVec4::drawValue(float width, rtti::Variant &object, const rtti::Property &property)
        {
            auto value = property.get_value(object).get_value<glm::vec4>();
            float float4[4] = { value[0], value[1], value[2], value[3] };
            ImGui::SetNextItemWidth(width);
            auto name = "###" + property.get_name().to_string();
            if (ImGui::InputFloat4(name.c_str(), float4, "%.3f"))
            {
                value[0] = float4[0];
                value[1] = float4[1];
                value[2] = float4[2];
                value[3] = float4[3];
                property.set_value(object, value);
            }
        }

    }

}