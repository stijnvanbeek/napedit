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

        bool PropertyEditorString::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            std::string str = var.to_string();
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "%s", str.c_str());
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
            {
                var = std::string(buffer);
                return true;
            }
            return false;
        }


        bool PropertyEditorBool::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            ImGui::SetNextItemWidth(width);
            auto value = var.to_bool();
            if (ImGui::Checkbox(label.c_str(), &value))
            {
                var = value;
                return true;
            }
            return false;
        }


        bool PropertyEditorFloat::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            ImGui::SetNextItemWidth(width);
            auto value = var.to_float();
            if (ImGui::InputFloat(label.c_str(), &value, 0.f, 0.f, "%.3f"))
            {
                var = value;
                return true;
            }
            return false;
        }



        bool PropertyEditorDouble::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            double value = var.to_double();
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputDouble(label.c_str(), &value, 0.f, 0.f, "%.3f"))
            {
                var = value;
                return true;
            }
            return false;
        }


        bool PropertyEditorInt::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            int value = var.to_int();
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputInt(label.c_str(), &value, 1, 1))
            {
                var = value;
                return true;
            }
            return false;
        }


        bool PropertyEditorVec2::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            auto value = var.get_value<glm::vec2>();
            float float2[2] = {value.x, value.y};
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputFloat2(label.c_str(), float2, "%.3f"))
            {
                value.x = float2[0];
                value.y = float2[1];
                return true;
            }
            return false;
        }


        bool PropertyEditorVec3::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            auto value = var.get_value<glm::vec3>();
            float float3[3] = {value.x, value.y, value.z};
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputFloat3(label.c_str(), float3, "%.3f"))
            {
                value.x = float3[0];
                value.y = float3[1];
                value.z = float3[2];
                return true;
            }
            return false;
        }


        bool PropertyEditorVec4::drawValue(rtti::Variant& var, const std::string& label, float width)
        {
            auto value = var.get_value<glm::vec4>();
            float float4[4] = {value.x, value.y, value.z, value.w};
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputFloat4(label.c_str(), float4, "%.3f"))
            {
                value.x = float4[0];
                value.y = float4[1];
                value.z = float4[2];
                value.w = float4[3];
                return true;
            }
            return false;
        }

    }

}