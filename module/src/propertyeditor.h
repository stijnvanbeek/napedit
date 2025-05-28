#pragma once

#include <Gui/Gui.h>

namespace nap
{
    namespace edit
    {

        class IPropertyEditor : public rtti::Object
        {
            RTTI_ENABLE(rtti::Object)

        public:
            IPropertyEditor() = default;
            virtual ~IPropertyEditor() = default;

            virtual bool drawValue(rtti::Variant& value, const std::string& label, float width) = 0;
            virtual const rtti::TypeInfo getType() const = 0;
        };


        class PropertyEditorString : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorString() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(std::string); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorBool : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorBool() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(bool); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorFloat : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorFloat() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(float); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorDouble : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorDouble() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(double); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorInt : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorInt() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(int); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorVec2 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec2() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec2); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorVec3 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec3() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec3); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


        class PropertyEditorVec4 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec4() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec4); }
            bool drawValue(rtti::Variant& value, const std::string& label, float width) override final;
        };


    }
}