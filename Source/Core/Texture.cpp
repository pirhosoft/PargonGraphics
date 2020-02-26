#include "Pargon/Application/Log.h"
#include "Pargon/Files/File.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "Pargon/Graphics/Texture.h"
#include "Pargon/Serialization/BlueprintReader.h"
#include "Pargon/Serialization/BufferReader.h"
#include "Pargon/Serialization/StringReader.h"
#include "Pargon/Serialization/StringWriter.h"

#include <png.h>

using namespace Pargon;

void TextureLoadResult::WriteResult(Log& log)
{
	if (Success)
	{
		log.Write("successfully loaded {}", Identifier);
	}
	else
	{
		log.Write("failed to load {}", Identifier);

		for (auto& error : Errors)
			log.Write(" - {}", error);
	}
}

auto TextureRegion::Reset(TextureLocation location, const File& file) -> TextureLoadResult
{
	auto contents = file.ReadData();

	if (!contents.Exists)
		return { false, file.Path(), { "file could not be read"_s } };

	return Reset(location, contents.Data, file.Path());
}

auto TextureRegion::Reset(TextureLocation location, BufferView data, StringView identifier) -> TextureLoadResult
{
	png_image png;
	memset(&png, 0, sizeof(png));
	png.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_memory(&png, data.begin(), data.Size()))
		return { false, identifier, { "data is not a png"_s } };

	if (auto result = Reset(location, { static_cast<unsigned int>(png.width), static_cast<unsigned int>(png.height) }, identifier); !result.Success)
		return result;

	auto reservation = Texture.Reserve(_location, _size);

	png.format = PNG_FORMAT_RGBA;
	if (!png_image_finish_read(&png, NULL, reservation.Data.begin(), reservation.Pitch, NULL))
		return { false, identifier, { "failed to read the png data"_s } };

	return { true, identifier, {} };
}

auto TextureRegion::Reset(TextureLocation location, TextureSize size, StringView identifier) -> TextureLoadResult
{
	auto maximumSize = Texture.Size();

	if ((location.X + size.Width > maximumSize.Width) || (location.Y + size.Height > maximumSize.Height))
		return { false, identifier, { FormatString("failed to place region at {},{} with size {}x{} inside texture of size {}x{}", location.X, location.Y, size.Width, size.Height, maximumSize.Width, maximumSize.Height) } };

	_location = location;
	_size = size;

	return { true, identifier, {} };
}

void FramesRegion::SetCells(TextureSize cellSize, unsigned int horizontalCount, unsigned int verticalCount)
{
	_cellSize.Width = cellSize.Width == TextureSize::FullWidth ? Texture.Size().Width / horizontalCount : cellSize.Width;
	_cellSize.Height = cellSize.Height == TextureSize::FullHeight ? Texture.Size().Height / horizontalCount : cellSize.Height;

	_horizontalCount = horizontalCount == TextureSize::FullWidth ? Texture.Size().Width / cellSize.Width : horizontalCount;
	_verticalCount = verticalCount == TextureSize::FullHeight ? Texture.Size().Height / cellSize.Height : verticalCount;
}

auto FramesRegion::GetCoordinates(unsigned int frame) const -> TextureCoordinates
{
	auto column = frame % _horizontalCount;
	auto row = frame / _horizontalCount;

	return GetCoordinates(column, row);
}

auto FramesRegion::GetCoordinates(unsigned int column, unsigned int row) const -> TextureCoordinates
{
	if (column < _horizontalCount && row < _verticalCount)
	{
		auto x = Location().X + column * _cellSize.Width;
		auto y = Location().Y + row * _cellSize.Height;

		return Texture.GetCoordinates({ x, y }, _cellSize);
	}

	return TextureRegion::GetCoordinates();
}

auto NineSliceRegion::GetTopLeftCoordinates() const -> TextureCoordinates
{
	auto x = Location().X;
	auto y = Location().Y;
	auto w = _leftInset;
	auto h = _topInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetTopCenterCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + _leftInset;
	auto y = Location().Y;
	auto w = Size().Width - _leftInset - _rightInset;
	auto h = _topInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetTopRightCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + Size().Width - _rightInset;
	auto y = Location().Y;
	auto w = _rightInset;
	auto h = _topInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetMiddleLeftCoordinates() const -> TextureCoordinates
{
	auto x = Location().X;
	auto y = Location().Y + _topInset;
	auto w = _leftInset;
	auto h = Size().Height - _topInset - _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetMiddleCenterCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + _leftInset;
	auto y = Location().Y + _topInset;
	auto w = Size().Width - _leftInset - _rightInset;
	auto h = Size().Height - _topInset - _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetMiddleRightCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + Size().Width - _rightInset;
	auto y = Location().Y + _topInset;
	auto w = _rightInset;
	auto h = Size().Height - _topInset - _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetBottomLeftCoordinates() const -> TextureCoordinates
{
	auto x = Location().X;
	auto y = Location().Y + Size().Height - _bottomInset;
	auto w = _leftInset;
	auto h = _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetBottomCenterCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + _leftInset;
	auto y = Location().Y + Size().Height - _bottomInset;
	auto w = Size().Width - _leftInset - _rightInset;
	auto h = _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

auto NineSliceRegion::GetBottomRightCoordinates() const -> TextureCoordinates
{
	auto x = Location().X + Size().Width - _rightInset;
	auto y = Location().Y + Size().Height - _bottomInset;
	auto w = _rightInset;
	auto h = _bottomInset;

	return Texture.GetCoordinates({ x, y }, { w, h });
}

namespace
{
	auto ReadBmfHeader(BufferReader& reader) -> bool
	{
		auto b = reader.ReadByte();
		auto m = reader.ReadByte();
		auto f = reader.ReadByte();
		auto v = reader.ReadByte();

		return !reader.HasFailed() && b == 'B' && m == 'M' && f == 'F' && v == 3;
	}

	void ReadBmfInfo(BufferReader& reader, FontRegion& font, uint32_t size)
	{
		auto fontSize = reader.Read<int16_t>();
		auto bitField = reader.Read<uint8_t>();
		auto charSet = reader.Read<uint8_t>();
		auto stretchH = reader.Read<uint16_t>();
		auto aa = reader.Read<uint8_t>();
		auto paddingUp = reader.Read<uint8_t>();
		auto paddingRight = reader.Read<uint8_t>();
		auto paddingDown = reader.Read<uint8_t>();
		auto paddingLeft = reader.Read<uint8_t>();
		auto spacingHoriz = reader.Read<uint8_t>();
		auto spacingVert = reader.Read<uint8_t>();
		auto outline = reader.Read<uint8_t>();
		auto fontName = ""_s;

		for (auto c = reader.ReadByte(); c != '\0'; c = reader.ReadByte())
			fontName.Append(c);

		if (!reader.HasFailed())
		{
			font.Font = std::move(fontName);
			font.Bold = (bitField & 0x3) != 0;
			font.Italic = (bitField & 0x2) != 0;
			font.Outline = outline;
		}
	}

	void ReadBmfFont(BufferReader& reader, FontRegion& font, uint32_t size)
	{
		auto lineHeight = reader.Read<uint16_t>();
		auto base = reader.Read<uint16_t>();
		auto scaleW = reader.Read<uint16_t>();
		auto scaleH = reader.Read<uint16_t>();
		auto pages = reader.Read<uint16_t>();
		auto bitField = reader.Read<uint8_t>();
		auto alphaChnl = reader.Read<uint8_t>();
		auto redChnl = reader.Read<uint8_t>();
		auto greenChnl = reader.Read<uint8_t>();
		auto blueChnl = reader.Read<uint8_t>();

		if (!reader.HasFailed())
		{
			//font.Baseline = base;
			//font.LineHeight = lineHeight;
		}
	}

	void ReadBmfKerning(BufferReader& reader, FontRegion& font, uint32_t size)
	{
		auto count = size / 10u;

		for (auto i = 0u; i < count; i++)
		{
			auto first = reader.Read<uint32_t>();
			auto second = reader.Read<uint32_t>();
			auto amount = reader.Read<int16_t>();

			auto glyph = font.GetGlyph(second);

			if (glyph != nullptr)
				font.SetKerning(*glyph, first, static_cast<int>(amount));
		}
	}

	void ReadBmfCharacters(BufferReader& reader, uint32_t count, FontRegion& font)
	{
		for (auto i = 0u; i < count && !reader.HasFailed(); i++)
		{
			auto id = reader.Read<uint32_t>();
			auto x = reader.Read<uint16_t>();
			auto y = reader.Read<uint16_t>();
			auto width = reader.Read<uint16_t>();
			auto height = reader.Read<uint16_t>();
			auto xoffset = reader.Read<int16_t>();
			auto yoffset = reader.Read<int16_t>();
			auto xadvance = reader.Read<int16_t>();
			auto page = reader.Read<uint8_t>();
			auto chnl = reader.Read<uint8_t>();

			if (!reader.HasFailed())
			{
				auto& glyph = font.AddGlyph(static_cast<char32_t>(id));

				glyph.Location.X = font.Location().X + x;
				glyph.Location.Y = font.Location().Y + y;
				glyph.Size.Width = width;
				glyph.Size.Height =  height;
				glyph.Left = xoffset;
				glyph.Bottom = yoffset;
				glyph.Advance = xadvance;

				//if (id == 32)
				//	font.SpaceWidth = xadvance;
			}
		}
	}

	void ReadBmfSection(BufferReader& reader, FontRegion& font)
	{
		auto id = reader.ReadByte();
		auto size = reader.Read<uint32_t>();

		switch (id)
		{
			case 1: ReadBmfInfo(reader, font, size); break;
			case 2: ReadBmfFont(reader, font, size); break;
			case 4: ReadBmfCharacters(reader, size / 20u, font); break;
			case 5: ReadBmfKerning(reader, font, size); break;
			default: reader.Advance(size); break;
		}
	}
}

auto FontRegion::Load(const File& file) -> TextureLoadResult
{
	auto data = file.ReadData();

	if (!data.Exists)
		return { false, file.Path(), { "file could not be read"_s } };

	auto reader = BufferReader(data.Data);
	reader.SetEndian(Endian::Little);

	if (!ReadBmfHeader(reader))
		return { false, file.Path(), { "file is not a version 3 bmf"_s } };

	while (!reader.HasFailed() && !reader.AtEnd())
		ReadBmfSection(reader, *this);

	if (reader.HasFailed())
		return { false, file.Path(), { "bmf data in file is invalid or incomplete"_s } };

	return { true, file.Path(), {} };
}

auto FontRegion::AddGlyph(char32_t character) -> Glyph&
{
	return _glyphs.AddOrSet(character, {});
}

auto FontRegion::GetGlyph(char32_t character) -> Glyph*
{
	auto index = _glyphs.GetIndex(character);
	return index != Sequence::InvalidIndex ? std::addressof(_glyphs.ItemAtIndex(index)) : nullptr;
}

auto FontRegion::GetGlyph(char32_t character) const -> const Glyph*
{
	auto index = _glyphs.GetIndex(character);
	return index != Sequence::InvalidIndex ? std::addressof(_glyphs.ItemAtIndex(index)) : nullptr;
}

void FontRegion::SetKerning(Glyph& glyph, char32_t from, int amount)
{
	glyph.Kerning.AddOrSet(from, amount);
}

auto FontRegion::GetKerning(const Glyph& glyph, char32_t from) const -> int
{
	auto index = glyph.Kerning.GetIndex(from);
	return index != Sequence::InvalidIndex ? glyph.Kerning.ItemAtIndex(index) : 0;
}

auto FontRegion::GetCoordinates(const Glyph& glyph) const -> TextureCoordinates
{
	return Texture.GetCoordinates(glyph.Location, glyph.Size);
}

auto Texture::Reset(const File& file) -> TextureLoadResult
{
	assert(IsLocked());

	auto contents = file.ReadData();

	if (!contents.Exists)
		return { false, file.Path(), { "file could not be read"_s } };

	return Reset(contents.Data, file.Path());
}

auto Texture::Reset(BufferView data, StringView identifier) -> TextureLoadResult
{
	assert(IsLocked());

	png_image png;
	memset(&png, 0, sizeof(png));
	png.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_memory(&png, data.begin(), data.Size()))
		return { false, identifier, { "data is not a png"_s } };

	if (auto result = Reset({ static_cast<unsigned int>(png.width), static_cast<unsigned int>(png.height) }, TextureFormat::ColorBuffer4x8, identifier); !result.Success)
		return result;

	auto reservation = Reserve({ 0, 0 }, _size);

	png.format = PNG_FORMAT_RGBA;
	if (!png_image_finish_read(&png, NULL, reservation.Data.begin(), reservation.Pitch, NULL))
		return { false, identifier, { "failed to read the png data"_s } };

	return { true, identifier, {} };
}

namespace
{
	auto GetDepth(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::ColorBuffer4x8: return 4;
			case TextureFormat::ColorTarget4x8: return 4;
			case TextureFormat::DepthStencilTarget24_8: return 4;
			case TextureFormat::Unknown: return 0;
		}

		return 0;
	}
}

auto Texture::Reset(TextureSize size, TextureFormat format, StringView identifier) -> TextureLoadResult
{
	assert(IsLocked());

	_size = size;
	_depth = GetDepth(format);
	_format = format;
	_identifier = identifier;
	_reservations.Clear();
	_regions.Clear();
	_names.Clear();

	return { true, identifier, {} };
}

auto Texture::Reserve(TextureLocation location, TextureSize size) -> TextureReservation
{
	auto x = location.X;
	auto y = location.Y;
	auto width = size.Width == TextureSize::FullWidth ? _size.Width - location.X : size.Width;
	auto height = size.Height == TextureSize::FullHeight ? _size.Height - location.Y : size.Height;

	assert(IsLocked());
	assert(x + width <= _size.Width && y + height <= _size.Height);

	for (auto& reservation : _reservations)
	{
		auto right = reservation.Location.X + reservation.Size.Width;
		auto bottom = reservation.Location.Y + reservation.Size.Height;

		if (x >= reservation.Location.X && y >= reservation.Location.Y && x + width <= right && y + height <= bottom)
		{
			auto xDifference = x - reservation.Location.X;
			auto yDifference = y - reservation.Location.Y;
			auto data = reservation.Data.GetReference(yDifference * reservation.Pitch + xDifference * _depth, height * reservation.Pitch);

			return { { x, y }, { width, height }, reservation.Pitch, data };
		}
	}

	auto& reservation = _reservations.Increment();
	reservation.Location = { x, y };
	reservation.Size = { width, height };
	reservation.Pitch = width * _depth;
	reservation.Data.Reserve(width * height * _depth);

	return { reservation.Location, reservation.Size, reservation.Pitch, reservation.Data };
}

void Texture::SetSampleCount(int count)
{
	assert(IsLocked());
	assert(_format == TextureFormat::ColorTarget4x8 || _format == TextureFormat::DepthStencilTarget24_8);

	_sampleCount = count; // TODO: clamp to valid counts
}

auto Texture::DestroyRegion(TextureRegionId id) -> bool
{
	if (id._id < _regions.Count() && _regions.Item(id._id) != nullptr)
	{
		_regions.Item(id._id).reset();
		return true;
	}

	return false;
}

auto Texture::GetCoordinates(TextureLocation location, TextureSize size) const -> TextureCoordinates
{
	auto w = 1.0f / _size.Width;
	auto h = 1.0f / _size.Height;
	auto u = location.X * w;
	auto v = location.Y * h;

	return { u, v, u + size.Width * w, v + size.Height * h };
}

void Texture::Clear()
{
	_reservations.Clear();
}
