#include "model.h"

#include <nap/group.h>
#include <utility/stringutils.h>
#include <rtti/writer.h>
#include <entity.h>
#include <rtti/jsonwriter.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonreader.h>

#include "nap/logger.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::edit::Model)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::edit::Selector)
	RTTI_PROPERTY("Model", &nap::edit::Selector::mModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

	namespace edit
	{


		template <typename ClassType, typename MemberType>
			MemberType* getRawMemberPointer(ClassType& instance, MemberType ClassType::*memberPointer)
		{
			return &(instance.*memberPointer);
		}


		bool Model::init(utility::ErrorState &errorState)
		{
			auto groupBase = RTTI_OF(IGroup);
			auto allGroups = groupBase.get_derived_classes();
			for (auto &group: allGroups)
				if (mCore.getResourceManager()->getFactory().canCreate(group))
					mGroupTypes[group.get_name().to_string()] = &group;

			auto resourceBase = RTTI_OF(Resource);
			auto allResources = resourceBase.get_derived_classes();
			for (auto &resource: allResources)
				if (mCore.getResourceManager()->getFactory().canCreate(resource))
					if (mGroupTypes.find(resource.get_name().to_string()) == mGroupTypes.end())
						mResourceTypes[resource.get_name().to_string()] = &resource;

			mPreResourcesLoadedSlot.setFunction([this](){ onPreResourcesLoaded(); });
			mPostResourcesLoadedSlot.setFunction([this](){ onPostResourcesLoaded(); });
			mCore.getResourceManager()->mPreResourcesLoadedSignal.connect(mPreResourcesLoadedSlot);
			mCore.getResourceManager()->mPostResourcesLoadedSignal.connect(mPostResourcesLoadedSlot);

			return true;
		}


		std::string Model::createResource(const rttr::type& resourceType, const std::string& aID)
		{
			auto object = mCore.getResourceManager()->getFactory().create(resourceType);
			auto resource = std::unique_ptr<Resource>(rtti_cast<Resource>(object));
			assert(resource != nullptr);
			auto typeName = resourceType.get_name().to_string();
			auto mID = typeName;
			if (!aID.empty())
				mID = aID;

			mID = getUniqueID(mID);
			if (!mID.empty())
			{
				resource->mID = mID;
				mResources.emplace_back(std::move(resource));
				mTree.mResources.emplace_back(mResources.back().get());
				return mResources.back()->mID;
			}
			return "";
		}



		std::string Model::createGroup(const rttr::type &groupType, const std::string &aID)
		{
			// Create the group
			auto object = mCore.getResourceManager()->getFactory().create(groupType);
			auto group = rtti_cast<IGroup>(object);
			assert(group != nullptr);
			auto groupPtr = std::unique_ptr<Resource>(group);

			// Generate the ID
			auto typeName = groupType.get_name().to_string();
			auto mID = typeName;
			if (!aID.empty())
				mID = aID;

			mID = getUniqueID(mID);
			if (!mID.empty())
			{
				// Add to mResources and to mTree
				groupPtr->mID = mID;
				mResources.emplace_back(std::move(groupPtr));
				mTree.mGroups.emplace_back(static_cast<ResourceGroup*>(group));
				return mID;
			}

			return "";
		}


		std::string Model::createEntity(const std::string &aID)
		{
			std::string mID = "Entity";
			if (!aID.empty())
				mID = aID;
			mID = getUniqueID(mID);
			if (mID.empty())
				return "";

			auto entity = std::make_unique<Entity>();
			entity->mID = mID;
			mResources.emplace_back(std::move(entity));
			mTree.mEntities.emplace_back(dynamic_cast<Entity*>(mResources.back().get()));
			return mID;
		}


		std::string Model::createComponent(const rttr::type &componentType, const std::string &entityID,
			const std::string &componentID)
		{
			auto entity = findResource<Entity>(entityID);
			assert(entity != nullptr);
			auto object = mCore.getResourceManager()->getFactory().create(componentType);
			auto component = rtti_cast<Component>(object);
			assert(component != nullptr);
			auto componentPtr = std::unique_ptr<Component>(component);

			auto typeName = componentType.get_name().to_string();
			auto mID = typeName;
			if (!componentID.empty())
				mID = componentID;
			mID = getUniqueID(mID);
			if (!mID.empty())
			{
				componentPtr->mID = mID;
				entity->mComponents.emplace_back(rtti_cast<Component>(componentPtr.get()));
				mResources.emplace_back(std::move(componentPtr));
				return mID;
			}
			return "";
		}


		Resource* Model::createEmbeddedObject(const rttr::type &type, const std::string &aID)
		{
			auto object = mCore.getResourceManager()->getFactory().create(type);
			auto resource = std::unique_ptr<Resource>(rtti_cast<Resource>(object));
			assert(resource != nullptr);
			auto typeName = type.get_name().to_string();
			auto mID = typeName;
			if (!aID.empty())
				mID = aID;

			mID = getUniqueID(mID);
			if (!mID.empty())
			{
				resource->mID = mID;
				mResources.emplace_back(std::move(resource));
				return mResources.back().get();
			}
			return nullptr;
		}


		std::unique_ptr<Resource> Model::removeEmbeddedObject(const std::string &mID)
		{
			auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
			assert(it != mResources.end());

			// Make sure it's not in the tree
			auto resource = it->get();
			bool found = eraseFromTree(*resource);
			assert(found == false);

			// Save the resource to return it
			auto result = std::move(*it);

			// Remove from the owned resources list
			mResources.erase(it);

			return result;
		}


		void Model::addEmbeddedObject(Resource *resource)
		{
			auto it = std::find_if(mResources.begin(), mResources.end(), [&resource](const auto& element) { return resource->mID == element->mID; });
			assert(it == mResources.end());
			mResources.emplace_back(std::unique_ptr<Resource>(resource));
		}


		void Model::moveResourceToGroup(const std::string &mID, const std::string &groupID)
		{
			auto resource = findResource(mID);
			assert(resource != nullptr);
			bool found = eraseFromTree(*resource);
			assert(found);
			auto group = findGroup(groupID);
			if (group != nullptr)
				group->mMembers.emplace_back(resource);
			else
				mTree.mResources.emplace_back(resource);
		}


		void Model::moveGroupToParent(const std::string &groupID, const std::string &parentGroupID)
		{
			auto group = findGroup(groupID);
			assert(group != nullptr);
			auto found = eraseFromTree(*group);
			assert(found);
			auto parent = findGroup(parentGroupID);
			if (parent != nullptr)
			{
				parent->mChildren.emplace_back(group);
			}
			else
				mTree.mGroups.emplace_back(group);
		}


		void Model::addEntityToParent(const std::string &entityID, const std::string &parentID)
		{
			auto entity = findResource<Entity>(entityID);
			assert(entity != nullptr);
			auto parent = findResource<Entity>(parentID);
			if (parent != nullptr)
			{
				parent->mChildren.emplace_back(entity);
			}
		}


		void Model::removeEntityFromParent(const std::string &entityID, const std::string &parentID)
		{
			auto entity = findResource<Entity>(entityID);
			assert(entity != nullptr);
			auto parent = findResource<Entity>(parentID);
			assert(parent != nullptr);
			if (parent != nullptr)
			{
				auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(), [&](auto& child){ return child->mID == entityID; });
				if (it != parent->mChildren.end())
					parent->mChildren.erase(it);
			}
		}


		void Model::removeResource(const std::string &mID)
		{
			// Find the resource to remove
			auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
			assert(it != mResources.end());
			auto resource = it->get();

			// Erase it from the tree
			eraseFromTree(*resource);

			// Recursively remove all its embedded objects by traversing the properties of the resource
			auto type = resource->get_type();
			for (auto& property : type.get_properties())
			{
				auto propertyValue = property.get_value(*resource);
				auto propertyType = property.get_type();
				// Check if the property is an embedded object
				bool embeddedPointer = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Embedded);
				if (embeddedPointer)
				{
					// If it is an object pointer and if it is set, remove the object
					if (propertyType.is_derived_from<rtti::ObjectPtrBase>())
					{
						rtti::Object* embeddedObject = propertyValue.get_value<rtti::ObjectPtr<rtti::Object>>().get();
						if (embeddedObject != nullptr)
							removeResource(embeddedObject->mID);
					}
					// If it is an array of objects, remove all elements
					else if (propertyType.is_array())
					{
						auto array = propertyValue.create_array_view();
						for (auto i = 0; i < array.get_size(); ++i)
						{
							auto element = array.get_value(i);
							auto elementType = element.get_type();
							if (elementType.is_derived_from<rtti::ObjectPtrBase>())
							{
								rtti::Object* embeddedObject = element.get_value<rtti::ObjectPtr<rtti::Object>>().get();
								if (embeddedObject != nullptr)
									removeResource(embeddedObject->mID);
							}
						}

					}
				}
			}

			// Finally remove the resource itself
			mResources.erase(it);
		}


		void Model::renameResource(const std::string &mID, const std::string &aNewName)
		{
			auto resource = findResource(mID);
			assert(resource != nullptr);
			auto newName = getUniqueID(aNewName);
			if (!newName.empty())
				resource->mID = newName;
		}


		Resource* Model::findResource(const std::string &mID)
		{
			auto it = std::find_if(mResources.begin(), mResources.end(), [&mID](const auto& resource) { return resource->mID == mID; });
			if (it != mResources.end())
				return it->get();

			return nullptr;
		}


		ResourceGroup* Model::findGroup(const std::string &mID)
		{
			auto resource = findResource(mID);
			if (resource == nullptr)
				return nullptr;
			auto group = rtti_cast<IGroup>(resource);
			if (group != nullptr)
				return static_cast<ResourceGroup*>(group);
			return nullptr;
		}


		void Model::clear()
		{
			mResources.clear();
			mTree.mResources.clear();
			mTree.mGroups.clear();
			mTree.mEntities.clear();
			mClearedSignal.trigger();
		}


		bool Model::serialize(std::string &output, utility::ErrorState &errorState)
		{
			std::vector<Object*> objects;
			for (auto& resource : mTree.mGroups)
				objects.emplace_back(resource.get());
			for (auto& resource : mTree.mResources)
				objects.emplace_back(resource.get());
			for (auto& resource : mTree.mEntities)
				objects.emplace_back(resource.get());

			rtti::JSONWriter writer;
			if (serializeObjects(objects, writer, errorState))
			{
				output = writer.GetJSON();
				return true;
			}
			else
				return false;
		}


		bool Model::deserialize(const std::string &input, utility::ErrorState &errorState)
		{
			rtti::DeserializeResult result;
			if (!rtti::deserializeJSON(input, rtti::EPropertyValidationMode::AllowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, mCore.getResourceManager()->getFactory(), result, errorState))
				return false;

			if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
			{
				errorState.fail("Failed to resolve links.");
				return false;
			}

			clear(); // Prepare to populate the model with the loaded objects

			// Move objects to flat resource list
			for (auto& object : result.mReadObjects)
			{
				auto raw = dynamic_cast<Resource*>(object.release());
				mResources.emplace_back(std::move(std::unique_ptr<Resource>(raw)));
			}

			// Populate the roots of the tree
			for (auto& resource : mResources)
			{
				// Look for an embedded pointer to the resource
				bool embeddedPointerFound = false;
				for (auto& unresolvedPointer : result.mUnresolvedPointers)
				{
					if (resource->mID == unresolvedPointer.mTargetID)
					{
						rtti::ResolvedPath path;
						if (unresolvedPointer.mRTTIPath.resolve(unresolvedPointer.mObject, path))
						{
							if (rtti::hasFlag(path.getProperty(), nap::rtti::EPropertyMetaData::Embedded))
							{
								embeddedPointerFound = true;
								break;
							}
						}
						else {
							errorState.fail("Failed to resolve pointer: %s", unresolvedPointer.mRTTIPath.toString().c_str());
							return false;
						}
					}
				}

				// If the resource is not embedded, it needs to be added to the root of the tree
				if (!embeddedPointerFound)
				{
					// Is the resource a group?
					if (resource->get_type().is_derived_from(RTTI_OF(IGroup)))
					{
						// Avoid adding the same group more than once
						auto it = std::find_if(mTree.mGroups.begin(), mTree.mGroups.end(), [&](const auto& group)
						{
							return group->mID == resource->mID;
						});
						if (it == mTree.mGroups.end())
							mTree.mGroups.emplace_back(static_cast<ResourceGroup*>(resource.get()));
					}

					// Is the resource an entity?
					else if (resource->get_type().is_derived_from(RTTI_OF(Entity)))
					{
						// Avoid adding the same entity more than once
						auto it = std::find_if(mTree.mEntities.begin(), mTree.mEntities.end(), [&](const auto& entity)
						{
							return entity->mID == resource->mID;
						});
						if (it == mTree.mEntities.end())
							mTree.mEntities.emplace_back(static_cast<Entity*>(resource.get()));
					}

					// Is the resource a resource?
					else
					{
						// Avoid adding the same resource more than once
						auto it = std::find_if(mTree.mResources.begin(), mTree.mResources.end(), [&](const auto& element)
						{
							return resource->mID == element->mID;
						});
						if (it == mTree.mResources.end())
							mTree.mResources.emplace_back(resource.get());
					}
				}
			}

			return true;
		}


		bool Model::loadFromFile(const std::string &path, utility::ErrorState &errorState)
		{
			if (!utility::fileExists(path))
			{
				errorState.fail("File not found: %s", path.c_str());
				return false;
			}
			std::string jsonString;
			if (!utility::readFileToString(path, jsonString, errorState))
			{
				errorState.fail("Failed to read file: %s", path.c_str());
				return false;
			}
			return deserialize(jsonString, errorState);
		}


		bool Model::saveToFile(const std::string &path, utility::ErrorState &errorState)
		{
			std::string jsonString;
			if (!serialize(jsonString, errorState))
				return false;
			utility::writeStringToFile(path, jsonString);
			return true;
		}


		bool Model::eraseFromTree(std::vector<ResourcePtr<Resource>>& branch, Object &object)
		{;
			auto it = std::find(branch.begin(), branch.end(), &object);
			if (it != branch.end())
			{
				branch.erase(it);
				return true;
			}

			for (auto& element : branch)
			{
				auto group = rtti_cast<ResourceGroup>(element.get());
				if (group != nullptr)
				{
					if (eraseFromTree(group->mMembers, object))
						return true;
					if (eraseFromTree(group->mChildren, object))
						return true;
				}
			}

			return false;
		}


		bool Model::eraseFromTree(std::vector<ResourcePtr<ResourceGroup>> &branch, Object &resource)
		{
			auto it = std::find(branch.begin(), branch.end(), rtti_cast<Resource>(&resource));
			if (it != branch.end())
			{
				branch.erase(it);
				return true;
			}
			for (auto& element : branch)
			{
				if (eraseFromTree(element->mMembers, resource))
					return true;
				if (eraseFromTree(element->mChildren, resource))
					return true;
			}
			return false;
		}


		bool Model::eraseFromTree(std::vector<ResourcePtr<Entity>> &branch, Object &resource)
		{
			auto it = std::find(branch.begin(), branch.end(), rtti_cast<Resource>(&resource));
			if (it != branch.end())
			{
				branch.erase(it);
				return true;
			}
			for (auto& element : branch)
			{
				if (eraseFromTree(element->mChildren, resource))
					return true;
				auto it = std::find(element->mComponents.begin(), element->mComponents.end(), rtti_cast<Resource>(&resource));
				if (it != element->mComponents.end())
				{
					element->mComponents.erase(it);
					return true;
				}
			}
			return false;
		}


		bool Model::eraseFromTree(Object &resource)
		{
			return eraseFromTree(mTree.mResources, resource) || eraseFromTree(mTree.mGroups, resource) || eraseFromTree(mTree.mEntities, resource);
		}


		std::string Model::getUniqueID(const std::string &aBaseID)
		{
			auto baseID = aBaseID;
			baseID = utility::replaceAllInstances(utility::trim(baseID), " ", "_");
			int idCounter = 2;
			auto mID = baseID;
			while (findResource(mID) != nullptr)
			{
				mID = baseID + std::to_string(idCounter);
				idCounter++;
			}
			return mID;
		}


		void Model::onPreResourcesLoaded()
		{
			// The model needs to be serialized and cleared before the resources are loaded, in order to rebuild it after the resources are loaded
			// This is because the ObjectPtrManager patches all ObjectPtrs, which is undesirable for the ObjectPtrs that are part of the model
			utility::ErrorState errorState;
			if (!serialize(mSerializedData, errorState))
				nap::Logger::warn("Failed to serialize model: %s", errorState.toString().c_str());
			// clear();
		}


		void Model::onPostResourcesLoaded()
		{
			// The model needs to be rebuilt from a serialized version of itself, after the resources are loaded
			utility::ErrorState errorState;
			if (!deserialize(mSerializedData, errorState))
				nap::Logger::warn("Failed to deserialize model: %s", errorState.toString().c_str());
			mSerializedData.clear();
		}


	}

}