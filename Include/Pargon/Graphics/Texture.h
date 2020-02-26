#pragma once

#include "Pargon/Application/Log.h"
#include "Pargon/Containers/Array.h"
#include "Pargon/Containers/Buffer.h"
#include "Pargon/Containers/List.h"
#include "Pargon/Containers/Map.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Files/File.h"
#include "Pargon/Graphics/GraphicsResource.h"
#include "Pargon/Math/Vector.h"

#include <memory>

namespace Pargon
{
	class Texture;

	using TextureId = GraphicsId<Texture>;
	using TextureHandle = GraphicsHandle<Texture>;

	enum class TextureFormat
	{
		Unknown,
		ColorBuffer4x8,
		ColorTarget4x8,
		DepthStencilTarget24_8
	};

	struct TextureLocation
	{
		static constexpr auto Invalid() -> TextureLocation;

		unsigned int X;
		unsigned int Y;
	};

	struct TextureSize
	{
		static constexpr auto FullWidth = std::numeric_limits<unsigned int>::max();
		static constexpr auto FullHeight = std::numeric_limits<unsigned int>::max();

		static constexpr auto Full() -> TextureSize;
		static constexpr auto Invalid() -> TextureSize;

		unsigned int Width;
		unsigned int Height;
	};

	struct TextureCoordinates
	{
		float U1;
		float V1;
		float U2;
		float V2;
	};

	struct TextureReservation
	{
		TextureLocation Location;
		TextureSize Size;
		unsigned int Pitch;

		BufferReference Data;
	};

	struct TextureRegionId
	{
	public:
		TextureRegionId();

		auto IsAssigned() const -> bool;

	private:
		friend class Texture;

		TextureRegionId(int id);

		int _id;
	};

	struct TextureLoadResult
	{
		bool Success;
		String Identifier;
		List<String> Errors;

		void WriteResult(Log& log);
	};

	class TextureRegion
	{
	public:
		TextureRegion() = delete;
		TextureRegion(const TextureRegion& copy) = delete;
		TextureRegion(TextureRegion&& move) = delete;
		virtual ~TextureRegion() = default;

		auto operator=(const TextureRegion& copy) -> TextureRegion& = delete;
		auto operator=(TextureRegion&& move) -> TextureRegion& = delete;

		Texture& Texture;

		auto Id() const -> TextureRegionId;
		auto Identifier() const -> StringView;

		auto Location() const -> TextureLocation;
		auto Size() const -> TextureSize;

		auto GetReservation() const -> TextureReservation;
		auto GetCoordinates() const -> TextureCoordinates;

		auto Reset(TextureLocation location, const File& file) -> TextureLoadResult;
		auto Reset(TextureLocation location, BufferView data, StringView identifier) -> TextureLoadResult;
		auto Reset(TextureLocation location, TextureSize size, StringView identifier) -> TextureLoadResult;

	protected:
		TextureRegion(Pargon::Texture& texture, TextureRegionId id);

	private:
		friend class Texture;

		TextureRegionId _id;
		String _identifier;

		TextureLocation _location = TextureLocation::Invalid();
		TextureSize _size = TextureSize::Invalid();
	};

	class FramesRegion : public TextureRegion
	{
	public:
		auto CellSize() const -> TextureSize;
		auto HorizontalCount() const -> unsigned int;
		auto VerticalCount() const -> unsigned int;

		void SetCells(TextureSize cellSize, unsigned int horizontalCount, unsigned int verticalCount);
		auto GetCoordinates(unsigned int frame) const -> TextureCoordinates;
		auto GetCoordinates(unsigned int column, unsigned int row) const -> TextureCoordinates;

	protected:
		friend class Texture;
		using TextureRegion::TextureRegion;

	private:
		TextureSize _cellSize = { 0, 0 };
		unsigned int _horizontalCount = 0;
		unsigned int _verticalCount = 0;
	};

	class NineSliceRegion : public TextureRegion
	{
	public:
		void SetInset(unsigned int inset);
		void SetInset(unsigned int horizontalInset, unsigned int verticalInset);
		void SetInset(unsigned int leftInset, unsigned int rightInset, unsigned int topInset, unsigned int bottomInset);

		auto GetTopLeftCoordinates() const -> TextureCoordinates;
		auto GetTopCenterCoordinates() const -> TextureCoordinates;
		auto GetTopRightCoordinates() const -> TextureCoordinates;
		auto GetMiddleLeftCoordinates() const -> TextureCoordinates;
		auto GetMiddleCenterCoordinates() const -> TextureCoordinates;
		auto GetMiddleRightCoordinates() const -> TextureCoordinates;
		auto GetBottomLeftCoordinates() const -> TextureCoordinates;
		auto GetBottomCenterCoordinates() const -> TextureCoordinates;
		auto GetBottomRightCoordinates() const -> TextureCoordinates;

	protected:
		friend class Texture;
		using TextureRegion::TextureRegion;

	private:
		unsigned int _leftInset = 0;
		unsigned int _rightInset = 0;
		unsigned int _bottomInset = 0;
		unsigned int _topInset = 0;
	};

	class FontRegion : public TextureRegion
	{
	public:
		class Glyph
		{
		public:
			TextureLocation Location;
			TextureSize Size;

			int Left;
			int Bottom;
			int Advance;

			Map<char32_t, int> Kerning;
		};

		String Font;
		bool Bold;
		bool Italic;
		int Outline;

		auto Load(const File& file) -> TextureLoadResult;
		auto AddGlyph(char32_t character) -> Glyph&;
		auto GetGlyph(char32_t character) -> Glyph*;
		auto GetGlyph(char32_t character) const -> const Glyph*;

		void SetKerning(Glyph& glyph, char32_t from, int amount);
		auto GetKerning(const Glyph& glyph, char32_t from) const -> int;
		auto GetCoordinates(const Glyph& glyph) const -> TextureCoordinates;

	protected:
		friend class Texture;
		using TextureRegion::TextureRegion;

	private:
		Map<char32_t, Glyph> _glyphs;
	};

	class Texture : public GraphicsResource<Texture>
	{
	public:
		auto Size() const -> TextureSize;
		auto Depth() const -> unsigned int;
		auto Format() const -> TextureFormat;
		auto SampleCount() const -> int;
		auto Identifier() const -> StringView;
		auto Reservations() const -> List<TextureReservation>;

		auto Reset(const File& file) -> TextureLoadResult;
		auto Reset(BufferView data, StringView identifier) -> TextureLoadResult;
		auto Reset(TextureSize size, TextureFormat format, StringView identifier) -> TextureLoadResult;
		auto Reserve(TextureLocation location, TextureSize size) -> TextureReservation;

		void SetSampleCount(int count);

		template<typename RegionType> auto CreateRegion(StringView name) -> RegionType&;
		template<typename RegionType> auto GetRegion(TextureRegionId id) const -> RegionType*;
		template<typename RegionType> auto GetRegion(StringView name) const -> RegionType*;
		auto DestroyRegion(TextureRegionId id) -> bool;

		auto GetCoordinates(TextureLocation location, TextureSize size) const -> TextureCoordinates;

	protected:
		void Clear() override;

	private:
		friend class GraphicsDevice;

		struct Reservation
		{
			TextureLocation Location;
			TextureSize Size;
			unsigned int Pitch;

			Buffer Data;
		};

		using GraphicsResource<Texture>::GraphicsResource;

		TextureSize _size = { 0, 0 };
		unsigned int _depth = 0;
		TextureFormat _format = TextureFormat::Unknown;
		int _sampleCount = 1;

		String _identifier;

		List<Reservation> _reservations;
		List<std::unique_ptr<TextureRegion>> _regions;
		Map<String, int> _names;
	};
}

constexpr
auto Pargon::TextureLocation::Invalid() -> TextureLocation
{
	auto invalid = std::numeric_limits<unsigned int>::max();
	return { invalid, invalid };
}

constexpr
auto Pargon::TextureSize::Full() -> TextureSize
{
	return { FullWidth, FullHeight };
}

constexpr
auto Pargon::TextureSize::Invalid() -> TextureSize
{
	return { 0, 0 };
}

inline
Pargon::TextureRegionId::TextureRegionId() :
	_id(-1)
{
}

inline
auto Pargon::TextureRegionId::IsAssigned() const -> bool
{
	return _id >= 0;
}

inline
Pargon::TextureRegionId::TextureRegionId(int id) :
	_id(id)
{
}

inline
auto Pargon::TextureRegion::Id() const -> TextureRegionId
{
	return _id;
}

inline
auto Pargon::TextureRegion::Identifier() const -> StringView
{
	return _identifier;
}

inline
auto Pargon::TextureRegion::Location() const -> TextureLocation
{
	return _location;
}

inline
auto Pargon::TextureRegion::Size() const -> TextureSize
{
	return _size;
}

inline
auto Pargon::TextureRegion::GetReservation() const -> TextureReservation
{
	return Texture.Reserve(_location, _size);
}

inline
auto Pargon::TextureRegion::GetCoordinates() const -> TextureCoordinates
{
	return Texture.GetCoordinates(_location, _size);
}

inline
Pargon::TextureRegion::TextureRegion(Pargon::Texture& texture, TextureRegionId id) :
	Texture(texture),
	_id(id)
{
}

inline
auto Pargon::FramesRegion::CellSize() const -> TextureSize
{
	return _cellSize;
}

inline
auto Pargon::FramesRegion::HorizontalCount() const -> unsigned int
{
	return _horizontalCount;
}

inline
auto Pargon::FramesRegion::VerticalCount() const -> unsigned int
{
	return _verticalCount;
}

inline
void Pargon::NineSliceRegion::SetInset(unsigned int inset)
{
	_leftInset = inset;
	_rightInset = inset;
	_bottomInset = inset;
	_topInset = inset;
}

inline
void Pargon::NineSliceRegion::SetInset(unsigned int horizontal, unsigned int vertical)
{
	_leftInset = horizontal;
	_rightInset = horizontal;
	_bottomInset = vertical;
	_topInset = vertical;
}

inline
void Pargon::NineSliceRegion::SetInset(unsigned int left, unsigned int right, unsigned int bottom, unsigned int top)
{
	_leftInset = left;
	_rightInset = right;
	_bottomInset = bottom;
	_topInset = top;
}

inline
auto Pargon::Texture::Size() const -> TextureSize
{
	return _size;
}

inline
auto Pargon::Texture::Depth() const -> unsigned int
{
	return _depth;
}

inline
auto Pargon::Texture::Format() const -> TextureFormat
{
	return _format;
}

inline
auto Pargon::Texture::SampleCount() const -> int
{
	return _sampleCount;
}

inline
auto Pargon::Texture::Identifier() const -> StringView
{
	return _identifier;
}

inline
auto Pargon::Texture::Reservations() const -> List<TextureReservation>
{
	List<TextureReservation> reservations;

	for (auto& reservation : _reservations)
		reservations.Add({ reservation.Location, reservation.Size, reservation.Pitch, reservation.Data });

	return reservations;
}

template<typename RegionType>
auto Pargon::Texture::CreateRegion(StringView name) -> RegionType&
{
	auto id = _regions.Count();
	auto region = new RegionType(*this, id);
	
	_regions.Add(std::unique_ptr<RegionType>(region));

	if (!name.IsEmpty())
		_names.AddOrSet(name, id);

	return *region;
}

template<typename RegionType>
auto Pargon::Texture::GetRegion(TextureRegionId id) const -> RegionType*
{
	return id._id >= 0 && id._id < _regions.Count() ? dynamic_cast<RegionType*>(_regions.Item(id._id).get()) : nullptr;
}

template<typename RegionType>
auto Pargon::Texture::GetRegion(StringView name) const -> RegionType*
{
	auto index = _names.GetIndex(name);
	return index != Sequence::InvalidIndex ? GetRegion<RegionType>(TextureRegionId{ _names.ItemAtIndex(index) }) : nullptr;
}
