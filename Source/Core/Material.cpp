#include "Pargon/Application/Log.h"
#include "Pargon/Containers/Sequence.h"
#include "Pargon/Files/File.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "Pargon/Graphics/Material.h"
#include "Pargon/Serialization/StringReader.h"
#include "Pargon/Serialization/StringWriter.h"

using namespace Pargon;

auto ShaderElement::Size() const -> size_t
{
	switch (Type)
	{
	case ShaderElementType::Color: return sizeof(char) * 4;
	case ShaderElementType::Float: return sizeof(float);
	case ShaderElementType::Int: return sizeof(int);
	case ShaderElementType::Matrix3x3: return sizeof(float) * 9;
	case ShaderElementType::Matrix4x4: return sizeof(float) * 16;
	case ShaderElementType::Point2: return sizeof(int) * 2;
	case ShaderElementType::Point3: return sizeof(int) * 3;
	case ShaderElementType::Point4: return sizeof(int) * 4;
	case ShaderElementType::Vector2: return sizeof(float) * 2;
	case ShaderElementType::Vector3: return sizeof(float) * 3;
	case ShaderElementType::Vector4: return sizeof(float) * 4;
	}

	return 0;
}

void Pargon::ToString(ShaderElement element, StringWriter& writer)
{
	//writer.Format("{} {} {}", element.Type, element.Usage, element.Index);
}

auto Pargon::FromString(ShaderElement& element, StringReader& reader) -> bool
{
	return false;//reader.Parse("{} {} {}", element.Type, element.Usage, element.Index);
}

auto Pargon::operator==(const BlendOptions& left, const BlendOptions& right) -> bool
{
	return left.SourceFactor == right.SourceFactor && left.DestinationFactor == right.DestinationFactor && left.Operation == right.Operation;
}

auto Pargon::operator!=(const BlendOptions& left, const BlendOptions& right) -> bool
{
	return left.SourceFactor != right.SourceFactor || left.DestinationFactor != right.DestinationFactor || left.Operation != right.Operation;
}

namespace
{
	auto WriteBlendOptions(const BlendOptions& value, StringWriter& writer, StringView name, BlendOptions set) -> bool
	{
		if (value == set)
		{
			writer.Write(name, {});
			return true;
		}

		return false;
	}

	auto ReadBlendOptions(BlendOptions& value, StringReader& reader, StringView name, BlendOptions set)
	{
		if (StartsWith(reader.ViewRemaining(), name, true))
		{
			value = set;
			reader.Advance(name.Length());
			return true;
		}

		return false;
	}
}

void Pargon::ToString(BlendOptions options, StringWriter& writer)
{
	if (WriteBlendOptions(options, writer, "Disabled", DisabledBlending)
		|| WriteBlendOptions(options, writer, "Standard", StandardBlending)
		|| WriteBlendOptions(options, writer, "Additive", AdditiveBlending)
		|| WriteBlendOptions(options, writer, "Premultiplied", PremultipliedBlending))
	{
		return;
	}

	//writer.Format("{} {} {}", options.SourceFactor, options.Operation, options.DestinationFactor);
}

auto Pargon::FromString(BlendOptions& options, StringReader& reader) -> bool
{
	if (ReadBlendOptions(options, reader, "Disabled", DisabledBlending)
		|| ReadBlendOptions(options, reader, "Standard", StandardBlending)
		|| ReadBlendOptions(options, reader, "Additive", AdditiveBlending)
		|| ReadBlendOptions(options, reader, "Premultiplied", PremultipliedBlending))
	{
		return true;
	}

	return true;//reader.Parse("{} {} {}", options.SourceFactor, options.Operation, options.DestinationFactor);
}

auto Pargon::operator==(const DepthOptions& left, const DepthOptions& right) -> bool
{
	return left.TestingEnabled == right.TestingEnabled && left.WritingEnabled == right.WritingEnabled && left.Comparison == right.Comparison;
}

auto Pargon::operator!=(const DepthOptions& left, const DepthOptions& right) -> bool
{
	return left.TestingEnabled != right.TestingEnabled || left.WritingEnabled != right.WritingEnabled || left.Comparison != right.Comparison;
}

namespace
{
	auto WriteDepthOptions(const DepthOptions& value, StringWriter& writer, StringView name, DepthOptions set) -> bool
	{
		if (value == set)
		{
			writer.Write(name, {});
			return true;
		}

		return false;
	}

	auto ReadDepthOptions(DepthOptions& value, StringReader& reader, StringView name, DepthOptions set)
	{
		if (StartsWith(reader.ViewRemaining(), name, true))
		{
			value = set;
			reader.Advance(name.Length());
			return true;
		}

		return false;
	}
}

void Pargon::ToString(DepthOptions options, StringWriter& writer)
{
	if (WriteDepthOptions(options, writer, "Disabled", DisabledDepthTesting)
		|| WriteDepthOptions(options, writer, "Standard", StandardDepthTesting)
		|| WriteDepthOptions(options, writer, "ReadOnly", ReadOnlyDepthTesting)
		|| WriteDepthOptions(options, writer, "WriteOnly", WriteOnlyDepthTesting))
	{
		return;
	}

	//writer.Format("{} {} {}", options.TestingEnabled, options.WritingEnabled, options.Comparison);
}

auto Pargon::FromString(DepthOptions& options, StringReader& reader) -> bool
{
	if (ReadDepthOptions(options, reader, "Disabled", DisabledDepthTesting)
		|| ReadDepthOptions(options, reader, "Standard", StandardDepthTesting)
		|| ReadDepthOptions(options, reader, "ReadOnly", ReadOnlyDepthTesting)
		|| ReadDepthOptions(options, reader, "WriteOnly", WriteOnlyDepthTesting))
	{
		return true;
	}

	return false;//reader.Parse("{} {} {}", options.TestingEnabled, options.WritingEnabled, options.Comparison);
}

auto Pargon::operator==(const StencilOptions& left, const StencilOptions& right) -> bool
{
	return left.Enabled == right.Enabled && left.ReferenceValue == right.ReferenceValue && left.Comparison == right.Comparison && left.PassOperation == right.PassOperation && left.FailureOperation == right.FailureOperation && left.DepthFailureOperation == right.DepthFailureOperation;
}

auto Pargon::operator!=(const StencilOptions& left, const StencilOptions& right) -> bool
{
	return left.Enabled != right.Enabled || left.ReferenceValue != right.ReferenceValue || left.Comparison != right.Comparison || left.PassOperation != right.PassOperation || left.FailureOperation != right.FailureOperation || left.DepthFailureOperation != right.DepthFailureOperation;
}

namespace
{
	auto WriteStencilOptions(const StencilOptions& value, StringWriter& writer, StringView name, StencilOptions set) -> bool
	{
		if (value == set)
		{
			writer.Write(name, {});
			return true;
		}

		return false;
	}

	auto ReadStencilOptions(StencilOptions& value, StringReader& reader, StringView name, StencilOptions set)
	{
		if (StartsWith(reader.ViewRemaining(), name, true))
		{
			value = set;
			reader.Advance(name.Length());
			return true;
		}

		return false;
	}
}

void Pargon::ToString(StencilOptions options, StringWriter& writer)
{
	if (WriteStencilOptions(options, writer, "Disabled", DisabledStenciling))
		return;

	//writer.Format("{} {} {} {} {} {}", options.Enabled, options.ReferenceValue, options.Comparison, options.PassOperation, options.FailureOperation, options.DepthFailureOperation);
}

auto Pargon::FromString(StencilOptions& options, StringReader& reader) -> bool
{
	if (ReadStencilOptions(options, reader, "Disabled", DisabledStenciling))
		return true;

	return false;//reader.Parse("{} {} {} {} {} {}", options.Enabled, options.ReferenceValue, options.Comparison, options.PassOperation, options.FailureOperation, options.DepthFailureOperation);
}

auto Pargon::operator==(const SampleOptions& left, const SampleOptions& right) -> bool
{
	return left.FilterMethod == right.FilterMethod && left.HorizontalAddressMode == right.HorizontalAddressMode && left.VerticalAddressMode == right.VerticalAddressMode;
}

auto Pargon::operator!=(const SampleOptions& left, const SampleOptions& right) -> bool
{
	return left.FilterMethod != right.FilterMethod || left.HorizontalAddressMode != right.HorizontalAddressMode || left.VerticalAddressMode != right.VerticalAddressMode;
}

namespace
{
	auto WriteSampleOptions(const SampleOptions& value, StringWriter& writer, StringView name, SampleOptions set) -> bool
	{
		if (value == set)
		{
			writer.Write(name, {});
			return true;
		}

		return false;
	}

	auto ReadSampleOptions(SampleOptions& value, StringReader& reader, StringView name, SampleOptions set)
	{
		if (StartsWith(reader.ViewRemaining(), name, true))
		{
			value = set;
			reader.Advance(name.Length());
			return true;
		}

		return false;
	}
}

void Pargon::ToString(SampleOptions options, StringWriter& writer)
{
	if (WriteSampleOptions(options, writer, "Nearest", NearestSampling)
		|| WriteSampleOptions(options, writer, "Bilinear", BilinearSampling)
		|| WriteSampleOptions(options, writer, "Trilinear", TrilinearSampling)
		|| WriteSampleOptions(options, writer, "Anisotropic", AnisotropicSampling))
	{
		return;
	}

	//writer.Format("{} {} {}", options.FilterMethod, options.HorizontalAddressMode, options.VerticalAddressMode);
}

auto Pargon::FromString(SampleOptions& options, StringReader& reader) -> bool
{
	if (ReadSampleOptions(options, reader, "Nearest", NearestSampling)
		|| ReadSampleOptions(options, reader, "Bilinear", BilinearSampling)
		|| ReadSampleOptions(options, reader, "Trilinear", TrilinearSampling)
		|| ReadSampleOptions(options, reader, "Anisotropic", AnisotropicSampling))
	{
		return true;
	}

	return false;//reader.Parse("{} {} {}", options.FilterMethod, options.HorizontalAddressMode, options.VerticalAddressMode);
}

const BlendOptions Pargon::DisabledBlending = { BlendFactor::One, BlendFactor::One, BlendOperation::None };
const BlendOptions Pargon::StandardBlending = { BlendFactor::SourceAlpha, BlendFactor::InverseSourceAlpha, BlendOperation::Add };
const BlendOptions Pargon::AdditiveBlending = { BlendFactor::SourceAlpha, BlendFactor::One, BlendOperation::Add };
const BlendOptions Pargon::PremultipliedBlending = { BlendFactor::One, BlendFactor::InverseSourceAlpha, BlendOperation::Add };
const DepthOptions Pargon::DisabledDepthTesting = { false, false, DepthStencilComparison::Less };
const DepthOptions Pargon::StandardDepthTesting = { true, true, DepthStencilComparison::Less };
const DepthOptions Pargon::ReadOnlyDepthTesting = { true, false, DepthStencilComparison::Less };
const DepthOptions Pargon::WriteOnlyDepthTesting = { true, true, DepthStencilComparison::Always };
const StencilOptions Pargon::DisabledStenciling = { false, 0, DepthStencilComparison::Always, DepthStencilOperation::Keep, DepthStencilOperation::Keep, DepthStencilOperation::Keep };
const SampleOptions Pargon::NearestSampling = { SampleFilterMethod::Nearest, SampleAddressMode::Clamp, SampleAddressMode::Clamp };
const SampleOptions Pargon::BilinearSampling = { SampleFilterMethod::Bilinear, SampleAddressMode::Clamp, SampleAddressMode::Clamp };
const SampleOptions Pargon::TrilinearSampling = { SampleFilterMethod::Trilinear, SampleAddressMode::Clamp, SampleAddressMode::Clamp };
const SampleOptions Pargon::AnisotropicSampling = { SampleFilterMethod::Anisotropic, SampleAddressMode::Clamp, SampleAddressMode::Clamp };

void ShaderCompilationResult::WriteResult(Log& log)
{
	if (Success)
	{
		log.Write("successfully compiled {}", Identifier);
	}
	else
	{
		log.Write("failed to compile {}", Identifier);

		for (auto& error : Errors)
			log.Write(" - {}", error);
	}
}

auto Material::GetVertexSize() const -> std::size_t
{
	auto size = std::size_t{ 0 };

	for (auto& element : VertexLayout)
		size += element.Size();

	return size;
}

auto Material::GetInstanceSize() const -> std::size_t
{
	auto size = std::size_t{ 0 };

	for (auto& element : InstanceLayout)
		size += element.Size();

	return size;
}

void Material::Reset()
{
	assert(IsLocked());

	VertexShader.Clear();
	FragmentShader.Clear();

	CullMode = MaterialCullMode::None;
	FillMode = MaterialFillMode::Fill;

	DepthOptions = DisabledDepthTesting;
	StencilOptions = DisabledStenciling;
	SampleOptions.Clear();

	VertexLayout.Clear();
	InstanceLayout.Clear();
	Output.Clear();
}

auto Material::Reset(StringView filename) -> ShaderCompilationResult
{
	ShaderCompilationResult result;

	auto file = ApplicationDirectory().GetFile(filename);
	auto contents = file.ReadText();

	return Reset(filename, contents.Text);
}

auto Material::Reset(StringView identifier, StringView shaderText) -> ShaderCompilationResult
{
	Reset();

	_identifier = identifier;
	return Graphics().Renderer()->CompileShader(shaderText, *this);
}

void Material::Clear()
{
	VertexShader.Clear();
	FragmentShader.Clear();
}