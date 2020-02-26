#pragma once

#include "Pargon/Containers/Array.h"
#include "Pargon/Containers/Sequence.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Serialization/Serialization.h"

#include <memory>
#include <mutex>

namespace Pargon
{
	class GraphicsDevice;
	class GraphicsResource_;

	enum class GraphicsStorage
	{
		CpuOnly,
		CopiedToGpu,
		StreamedToGpu,
		TransferredToGpu,
		GpuOnly
	};

	class GraphicsHandle_
	{
	public:
		virtual ~GraphicsHandle_() = default;

	protected:
		friend class GraphicsDevice;

		virtual auto Update(GraphicsResource_* resource) -> bool = 0;
	};

	template<typename ResourceType>
	class GraphicsHandle : public GraphicsHandle_
	{
	protected:
		auto Update(GraphicsResource_* resource) -> bool override;
		virtual auto Update(ResourceType* resource) -> bool = 0;
	};

	class GraphicsResource_
	{
	public:
		virtual ~GraphicsResource_() = default;

		auto Graphics() const -> GraphicsDevice&;
		auto Storage() const -> GraphicsStorage;
		template<typename HandleType> auto Handle() const -> HandleType*;

		auto IsLocked() const -> bool;
		auto IsChanged() const -> bool;

		void Lock();
		void Unlock();

	protected:
		GraphicsResource_(GraphicsDevice& graphics, GraphicsStorage storage, std::unique_ptr<GraphicsHandle_>&& handle);

		void UpdateComplete();
		virtual void Clear() = 0;

	private:
		friend class GraphicsDevice;

		GraphicsDevice& _graphics;
		GraphicsStorage _storage;
		std::unique_ptr<GraphicsHandle_> _handle;

		bool _isLocked;
		bool _isChanged;
	};

	template<typename ResourceType>
	class GraphicsId
	{
	public:
		GraphicsId() = default;

		auto operator==(GraphicsId<ResourceType> other) const -> bool;
		auto operator!=(GraphicsId<ResourceType> other) const -> bool;
		auto operator<(GraphicsId<ResourceType> other) const -> bool;

		auto IsAssigned() const -> bool;
		auto Assignment() const -> int;

	private:
		friend class GraphicsDevice;
		GraphicsId(int id);
		int _id = -1;
	};

	template<typename ResourceType>
	class GraphicsResource : public GraphicsResource_
	{
	public:
		auto Id() const -> GraphicsId<ResourceType>;
		auto Name() const -> StringView;
		void SetName(StringView name);

	protected:
		GraphicsResource(GraphicsDevice& graphics, GraphicsStorage storage, std::unique_ptr<GraphicsHandle_>&& handle, GraphicsId<ResourceType> id);

	private:
		GraphicsId<ResourceType> _id;
		String _name;
	};
}

template<typename ResourceType>
auto Pargon::GraphicsHandle<ResourceType>::Update(GraphicsResource_* resource) -> bool
{
	return Update(static_cast<ResourceType*>(resource));
}

inline
auto Pargon::GraphicsResource_::Graphics() const -> GraphicsDevice&
{
	return _graphics;
}

inline
auto Pargon::GraphicsResource_::Storage() const -> GraphicsStorage
{
	return _storage;
}

template<typename HandleType>
auto Pargon::GraphicsResource_::Handle() const -> HandleType*
{
	return dynamic_cast<HandleType*>(_handle.get());
}

inline
auto Pargon::GraphicsResource_::IsLocked() const -> bool
{
	return _isLocked;
}

inline
auto Pargon::GraphicsResource_::IsChanged() const -> bool
{
	return _isChanged;
}

template<typename ResourceType>
auto Pargon::GraphicsId<ResourceType>::operator==(GraphicsId<ResourceType> other) const -> bool
{
	return _id == other._id;
}

template<typename ResourceType>
auto Pargon::GraphicsId<ResourceType>::operator!=(GraphicsId<ResourceType> other) const -> bool
{
	return _id != other._id;
}

template<typename ResourceType>
auto Pargon::GraphicsId<ResourceType>::operator<(GraphicsId<ResourceType> other) const -> bool
{
	return _id < other._id;
}

template<typename ResourceType>
auto Pargon::GraphicsId<ResourceType>::IsAssigned() const -> bool
{
	return _id != -1;
}

template<typename ResourceType>
auto Pargon::GraphicsId<ResourceType>::Assignment() const -> int
{
	return _id;
}

template<typename ResourceType>
Pargon::GraphicsId<ResourceType>::GraphicsId(int id) :
	_id(id)
{
}

template<typename ResourceType>
auto Pargon::GraphicsResource<ResourceType>::Id() const -> GraphicsId<ResourceType>
{
	return _id;
}

template<typename ResourceType>
auto Pargon::GraphicsResource<ResourceType>::Name() const -> StringView
{
	return _name;
}

template<typename ResourceType>
void Pargon::GraphicsResource<ResourceType>::SetName(StringView name)
{
	_name = name;
}

template<typename ResourceType>
Pargon::GraphicsResource<ResourceType>::GraphicsResource(GraphicsDevice& graphics, GraphicsStorage storage, std::unique_ptr<GraphicsHandle_>&& handle, GraphicsId<ResourceType> id) : GraphicsResource_(graphics, storage, std::move(handle)),
	_id(id)
{
}
