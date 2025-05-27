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

            virtual void show(float nameOffset, float valueOffset, float typeOffset, rtti::Variant& object, const rtti::Property& property) = 0;
            virtual const rtti::TypeInfo getType() const = 0;
        };


        class PropertyEditorSimple : public IPropertyEditor
        {
            RTTI_ENABLE(IPropertyEditor)
            
        public:
            PropertyEditorSimple() = default;
            void show(float nameOffset, float valueOffset, float typeOffset, rtti::Variant& object, const rtti::Property& property) override final;
            virtual const rtti::TypeInfo getType() const = 0;

        private:
            virtual void drawValue(float width, rtti::Variant& object, const rtti::Property& property) = 0;
        };


        class PropertyEditorString : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorString() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(std::string); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorBool : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorBool() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(bool); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorFloat : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorFloat() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(float); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorDouble : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorDouble() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(double); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorInt : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorInt() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(int); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec2 : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorVec2() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec2); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec3 : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorVec3() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec3); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


        class PropertyEditorVec4 : public PropertyEditorSimple
        {
            RTTI_ENABLE(PropertyEditorSimple)

        public:
            PropertyEditorVec4() = default;
            const rtti::TypeInfo getType() const override final { return RTTI_OF(glm::vec4); }
            void drawValue(float width, rtti::Variant& object, const rtti::Property &property) override final;
        };


    }
}