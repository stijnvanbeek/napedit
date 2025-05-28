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

            virtual void drawValue(float width, rtti::Variant& object, const rtti::Property& property) = 0;
            virtual const rtti::TypeInfo getType() const = 0;
        };


        class PropertyEditorString : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorString() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(std::string); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorBool : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorBool() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(bool); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorFloat : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorFloat() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(float); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorDouble : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorDouble() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(double); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorInt : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorInt() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(int); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec2 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec2() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec2); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec3 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec3() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec3); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec4 : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)

        public:
            PropertyEditorVec4() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec4); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


    }
}