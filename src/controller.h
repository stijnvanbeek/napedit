#pragma once

#include <model.h>

namespace nap
{
	namespace edit
	{

		class Controller : public Resource
		{
			RTTI_ENABLE(Resource)

		public:
			class ValuePath
			{
			public:
				ValuePath() : mResolvedPath() { };
				ValuePath(const ValuePath&);
				ValuePath(ValuePath&&);
				void set(const rtti::Path& path, Resource* root);
				void set(const rtti::Path& arrayPath, int index, Resource* root);
				void clear() { mIsResolved = false; }
				void resolve(Model& model);
				bool isResolved() const { return mIsResolved; }
				bool isArrayElement() const { return mIsArrayElement; }
				bool isArray() const { return mResolvedPath.getType().is_array(); }
				int getArrayIndex() const { return mArrayIndex; }
				bool isPointer() const { return mResolvedPath.getType().is_derived_from<rtti::ObjectPtrBase>(); }
				rtti::ResolvedPath& getResolvedPath() { return mResolvedPath; }
				const rtti::Path& getPath() const { return mPath; }

			private:
				void resolve(Resource* root);

				std::string mRootID;
				rtti::Path mPath;
				rtti::ResolvedPath mResolvedPath;
				bool mIsArrayElement = false;
				int mArrayIndex = -1;
				bool mIsResolved = false;
			};

		public:
			Controller() = default;

			ResourcePtr<Model> mModel;
			bool renameResource(const std::string& oldID, const std::string& newID);
			void createGroup(const rtti::TypeInfo& type);
			void createEntity();
			void createChildGroup(const std::string& parentID);
			void removeResource(const std::string& mID);
			void createResource(const rtti::TypeInfo& type, const std::string& parentID);
			void createGroup(const rtti::TypeInfo& type, const std::string& parentID);
			void addChildEntity(const std::string& childID, const std::string& parentID);
			void createComponent(const rtti::TypeInfo& type, const std::string& entityID);

			template <typename T>
			void setValue(ValuePath& path, const T& value);

			void undo();
			void redo();

		private:
			void addUndoStack(std::function<void()> doFunction, std::function<void()> undoFunction);
			struct Command
			{
				std::function<void()> mUndo;
				std::function<void()> mRedo;
			};
			std::vector<std::unique_ptr<Command>> mUndoStack;
			std::vector<std::unique_ptr<Command>> mRedoStack;
		};


		template<typename T>
		void Controller::setValue(ValuePath &path, const T &value)
		{
			assert(path.isResolved());
			auto oldValue = path.getResolvedPath().getValue();
			path.getResolvedPath().setValue(value);
			addUndoStack(
				[this, path, value]() mutable
				{
					path.resolve(*mModel);
					path.getResolvedPath().setValue(value);
				},
				[this, path, oldValue]() mutable
				{
					path.resolve(*mModel);
					path.getResolvedPath().setValue(oldValue);
				}
			);
		}

	}
}