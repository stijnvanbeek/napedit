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
				void set(int arrayIndex);
				void clear() { mIsResolved = false; }
				void resolve(Model& model);
				bool isResolved() const { return mIsResolved; }
				bool isArrayElement() const { return mIsArrayElement; }
				bool isArray() const { return mResolvedPath.getType().is_array(); }
				int getArrayIndex() const { return mArrayIndex; }
				bool isPointer() const { return mResolvedPath.getType().is_derived_from<rtti::ObjectPtrBase>(); }
				rtti::ResolvedPath& getResolvedPath() { return mResolvedPath; }
				const rtti::Path& getPath() const;

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
			void createEmbeddedObject(ValuePath& path, const rtti::TypeInfo& type);
			void removeEmbeddedObject(ValuePath& path);

			template <typename T>
			void setValue(ValuePath& path, const T& value);

			template <typename T>
			void insertArrayElement(ValuePath& path, T element);

			void insertArrayElement(ValuePath& path);
			void removeArrayElement(ValuePath& path);
			void moveArrayElementUp(ValuePath& path);
			void moveArrayElementDown(ValuePath& path);

			void undo();
			void redo();

		private:
			template <typename T>
			void doInsertArrayElement(ValuePath& path, T element);
			void doRemoveArrayElement(ValuePath& path);
			bool doMoveArrayElementUp(ValuePath& path);
			bool doMoveArrayElementDown(ValuePath& path);

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


		template <typename T>
		void Controller::insertArrayElement(ValuePath& path, T element)
		{
			assert(path.isArray());
			assert(path.isResolved());

			doInsertArrayElement(path, element);
			addUndoStack(
				[this, path, element]() mutable
				{
					path.resolve(*mModel);
					doInsertArrayElement(path, element);
				},
				[this, path]() mutable
				{
					path.resolve(*mModel);
					doRemoveArrayElement(path);
				}
			);
		}


		template <typename T>
		void Controller::doInsertArrayElement(ValuePath& path, T element)
		{
			auto array = path.getResolvedPath().getValue();
			auto view = array.create_array_view();
			if (path.isArrayElement())
			{
				assert(path.getArrayIndex() <= view.get_size());
				view.insert_value(path.getArrayIndex(), element);
			}
			else if (path.isArray())
				view.insert_value(view.get_size(), element);
			path.getResolvedPath().setValue(array);
		}


	}
}