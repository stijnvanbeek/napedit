#include "testresource.h"

RTTI_BEGIN_STRUCT(nap::TestStruct)
    RTTI_PROPERTY("Int", &nap::TestStruct::mInt, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Float", &nap::TestStruct::mFloat, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("String", &nap::TestStruct::mString, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_ENUM(nap::TestEnum)
    RTTI_ENUM_VALUE(nap::TestEnum::EEN, "One"),
    RTTI_ENUM_VALUE(nap::TestEnum::TWEE, "Two"),
    RTTI_ENUM_VALUE(nap::TestEnum::DRIE, "Three")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::TestResource)
    RTTI_PROPERTY("Struct", &nap::TestResource::mStruct, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Enum", &nap::TestResource::mEnum, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Vector", &nap::TestResource::mVector, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Pointer", &nap::TestResource::mPointer, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EmbeddedPointer", &nap::TestResource::mEmbeddedPointer, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("PointerVector", &nap::TestResource::mPointerVector, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EmbeddedPointerVector", &nap::TestResource::mEmbeddedPointerVector, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Array", &nap::TestResource::mArray, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace 
    {
    
    }

}