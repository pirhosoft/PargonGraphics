#include "Pargon/Graphics/Geometry.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "Pargon/Serialization/StringReader.h"
#include "Pargon/Serialization/StringWriter.h"

using namespace Pargon;

void Geometry::Reset(GeometryTopology topology, int capacity)
{
	assert(IsLocked());

	_topology = topology;
	_data.SetSize(capacity);
	_size = 0;
}

namespace
{
	auto GetAlignment(std::size_t location, std::size_t size) -> std::size_t
	{
		auto alignment = location % size;
		return alignment == 0 ? 0 : size - alignment;
	}

	auto GetOffset(std::size_t location, std::size_t alignment, std::size_t size) -> int
	{
		return static_cast<int>((location + alignment) / size);
	}
}

auto Geometry::GetStart(std::size_t size) const -> int
{
	auto alignment = GetAlignment(_size, _topology == GeometryTopology::ConstantData ? _constantDataAlignment : size);
	return GetOffset(_size, alignment, _topology == GeometryTopology::ConstantData ? _constantDataOffset : size);
}

auto Geometry::Reserve(std::size_t size, int count) -> Reservation
{
	ValidateReservation(size, count);

	auto alignment = GetAlignment(_size, _topology == GeometryTopology::ConstantData ? _constantDataAlignment : size);
	auto offset = GetOffset(_size, alignment, _topology == GeometryTopology::ConstantData ? _constantDataOffset : size);
	auto required = count * size + alignment;
	auto location = static_cast<int>(_size + alignment);

	_data.SetSize(static_cast<int>(_size + required));

	_size += required;

	return { offset, count, size, _data.GetReference(location, static_cast<int>(count * size)) };
}

auto Geometry::Retreive(std::size_t size, int start, int count) -> Reservation
{
	ValidateReservation(size, count);

	auto alignment = GetAlignment(_size, _topology == GeometryTopology::ConstantData ? _constantDataAlignment : size);
	auto offset = GetOffset((start * size), alignment, _topology == GeometryTopology::ConstantData ? _constantDataOffset : size);
	auto location = static_cast<int>((start * size) + alignment);

	return { offset, count, size, _data.GetReference(location, static_cast<int>(count * size)) };
}

void Geometry::Clear()
{
	_data.Clear();
}

void Geometry::ValidateReservation(std::size_t size, int count)
{
	assert(IsLocked());
	assert(_topology != GeometryTopology::ConstantData || count == 1);
	assert(_topology != GeometryTopology::ConstantData || (size % _constantDataOffset == 0));
	assert(_topology != GeometryTopology::IndexList || size == 2 || size == 4);
}
