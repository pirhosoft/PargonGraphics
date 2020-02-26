#pragma once

#include "Pargon/Containers/Buffer.h"
#include "Pargon/Containers/Sequence.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Graphics/GraphicsResource.h"

namespace Pargon
{
	class Geometry;
	class GraphicsDevice;
	class StringReader;
	class StringWriter;

	using GeometryId = GraphicsId<Geometry>;
	using GeometryHandle = GraphicsHandle<Geometry>;

	enum class GeometryTopology
	{
		Unknown,
		TriangleList,
		TriangleStrip,
		PointList,
		LineList,
		IndexList,
		InstanceData,
		ConstantData
	};

	template<typename ElementType>
	struct GeometryReservation
	{
		GeometryId Geometry;

		int Offset;
		int ElementCount;
		std::size_t ElementSize;

		SequenceReference<ElementType> Elements;
	};

	class Geometry : public GraphicsResource<Geometry>
	{
	public:
		struct Reservation
		{
			int Offset;
			int ElementCount;
			std::size_t ElementSize;

			BufferReference Buffer;
		};

		auto Topology() const -> GeometryTopology;
		auto Data() const -> BufferView;
		auto Size() const -> std::size_t;

		void Reset(GeometryTopology topology, int capacity);
		template<typename ElementType> auto Reset(GeometryTopology topology, SequenceView<ElementType> elements) -> GeometryReservation<ElementType>;

		auto GetStart(std::size_t size) const -> int;
		template<typename ElementType> auto GetStart() const -> int;

		auto Reserve(std::size_t size, int count) -> Reservation;
		template<typename ElementType> auto Reserve(int count) -> GeometryReservation<ElementType>;
		template<typename ElementType> auto Reserve(SequenceView<ElementType> elements) -> GeometryReservation<ElementType>;

		auto Retreive(std::size_t size, int offset, int count) -> Reservation;
		template<typename ElementType> auto Retreive(int start, int count) -> GeometryReservation<ElementType>;

	protected:
		void Clear() override;

	private:
		friend class GraphicsDevice;
		using GraphicsResource<Geometry>::GraphicsResource;

		static constexpr std::size_t _constantDataAlignment = 256;
		static constexpr std::size_t _constantDataOffset = 16;

		GeometryTopology _topology = GeometryTopology::Unknown;
		Buffer _data;
		std::size_t _size = 0;

		void ValidateReservation(std::size_t size, int count);
	};
}

inline
auto Pargon::Geometry::Topology() const -> GeometryTopology
{
	return _topology;
}

inline
auto Pargon::Geometry::Data() const -> BufferView
{
	return _data;
}

inline
auto Pargon::Geometry::Size() const -> std::size_t
{
	return _size;
}

template<typename ElementType>
auto Pargon::Geometry::Reset(GeometryTopology topology, SequenceView<ElementType> elements) -> GeometryReservation<ElementType>
{
	Reset(topology, elements.Count());
	return Reserve<ElementType>(elements);
}

template<typename ElementType>
auto Pargon::Geometry::GetStart() const -> int
{
	return GetStart(sizeof ElementType);
}

template<typename ElementType>
auto Pargon::Geometry::Reserve(int count) -> GeometryReservation<ElementType>
{
	auto reservation = Reserve(sizeof(ElementType), count);
	return { Id(), reservation.Offset, reservation.ElementCount, reservation.ElementSize, { reinterpret_cast<ElementType*>(reservation.Buffer.begin()), count } };
}

template<typename ElementType>
auto Pargon::Geometry::Reserve(SequenceView<ElementType> elements) -> GeometryReservation<ElementType>
{
	auto reservation = Reserve<ElementType>(elements.Count());
	std::copy(elements.begin(), elements.end(), reservation.Elements.begin());
	return reservation;
}

template<typename ElementType>
auto Pargon::Geometry::Retreive(int offset, int count) -> GeometryReservation<ElementType>
{
	auto reservation = Retreive(sizeof ElementType, offset, count);
	return { Id(), reservation.Offset, reservation.ElementCount, reservation.ElementSize, { reinterpret_cast<ElementType*>(reservation.Buffer.begin()), count } };
}