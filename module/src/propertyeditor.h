#pragma once

#include <Gui/Gui.h>
#include <set>

namespace nap
{
    namespace edit
    {
        /**
         * Interface for property editors.
         * Used by the Inspector to draw and edit the value of a property of a certain rtti::TypeInfo.
         */
        class IPropertyEditor : public rtti::Object
        {
            RTTI_ENABLE(rtti::Object)

        public:
            IPropertyEditor() = default;
            virtual ~IPropertyEditor() = default;

            /**
             * Draws the value of the property.
             * @param value The value of the property stored in a Variant.
             * @param label The text label of the property.
             * @param width The width of the property.
             * @return True if the value stored in the Variant was changed.
             */
            virtual bool drawValue(rtti::Variant& value, const std::string& label, float width) = 0;

            /**
             * @return The type that this editor can view and edit.
             */
            virtual rtti::TypeInfo getType() const = 0;
        };


        class PropertyEditorString : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorString() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(std::string); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorBool : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorBool() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(bool); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorFloat : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorFloat() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(float); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorDouble : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorDouble() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(double); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        template<typename T>
        class PropertyEditorInt : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorInt() = default;

            rtti::TypeInfo getType() const override final { return RTTI_OF(T); }

            bool drawValue(rtti::Variant& var, const std::string& label, float width) override final
            {
                T value = var.to_int();
                ImGui::SetNextItemWidth(width);
                int input = value;
                if (ImGui::InputInt(label.c_str(), &input, 1, 1))
                {
                    value = input;
                    var = value;
                    return true;
                }
                return false;
            }
        };


        class PropertyEditorVec2 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec2() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec2); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorVec3 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec3() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec3); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorVec4 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec4() = default;
            rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec4); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };

    }
}