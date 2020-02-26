#pragma once

#include "Pargon/Containers/Buffer.h"
#include "Pargon/Containers/List.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Graphics/GraphicsResource.h"
#include "Pargon/Serialization/Serializer.h"

namespace Pargon
{
	class Log;
	class GraphicsDevice;
	class StringReader;
	class StringWriter;
	class Material;

	using MaterialId = GraphicsId<Material>;
	using MaterialHandle = GraphicsHandle<Material>;

	enum class ShaderElementType
	{
		Float,
		Vector2,
		Vector3,
		Vector4,
		Int,
		Point2,
		Point3,
		Point4,
		Matrix3x3,
		Matrix4x4,
		Color,
		Texture,
		Sampler
	};

	template<> auto EnumNames<ShaderElementType> = SetEnumNames
	(
		"Float",
		"Vector2",
		"Vector3",
		"Vector4",
		"Int",
		"Point2",
		"Point3",
		"Point4",
		"Matrix3x3",
		"Matrix4x4",
		"Color",
		"Texture",
		"Sampler"
	);

	enum class ShaderElementUsage
	{
		Position,
		Coordinate,
		Color,
		Other
	};

	template<> auto EnumNames<ShaderElementUsage> = SetEnumNames
	(
		"Position",
		"Coordinate",
		"Color",
		"Other"
	);

	struct ShaderElement
	{
		ShaderElementType Type;
		ShaderElementUsage Usage;

		auto Size() const -> size_t;
	};

	void ToString(ShaderElement element, StringWriter& writer);
	auto FromString(ShaderElement& element, StringReader& reader) -> bool;

	enum class MaterialCullMode
	{
		None,
		Clockwise,
		CounterClockwise
	};

	enum class MaterialFillMode
	{
		Fill,
		Wireframe,
		None
	};

	enum class BlendFactor
	{
		Zero,
		One,
		SourceColor,
		InverseSourceColor,
		SourceAlpha,
		InverseSourceAlpha,
		DestinationColor,
		InverseDestinationColor,
		DestinationAlpha,
		InverseDestinationAlpha,
		Saturate
	};

	enum class BlendOperation
	{
		None,
		Add,
		Subtract,
		Invert,
		Minimum,
		Maximum
	};

	struct BlendOptions
	{
		BlendFactor SourceFactor;
		BlendFactor DestinationFactor;
		BlendOperation Operation;
	};

	auto operator==(const BlendOptions& left, const BlendOptions& right) -> bool;
	auto operator!=(const BlendOptions& left, const BlendOptions& right) -> bool;
	void ToString(BlendOptions options, StringWriter& writer);
	auto FromString(BlendOptions& options, StringReader& reader) -> bool;

	enum class DepthStencilComparison
	{
		Never,
		Always,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual
	};

	enum class DepthStencilOperation
	{
		Keep,
		Zero,
		Replace,
		Increase,
		IncreaseWrap,
		Decrease,
		DecreaseWrap,
		Invert
	};

	struct DepthOptions
	{
		bool TestingEnabled;
		bool WritingEnabled;
		DepthStencilComparison Comparison;
	};

	auto operator==(const DepthOptions& left, const DepthOptions& right) -> bool;
	auto operator!=(const DepthOptions& left, const DepthOptions& right) -> bool;
	void ToString(DepthOptions options, StringWriter& writer);
	auto FromString(DepthOptions& options, StringReader& reader) -> bool;

	struct StencilOptions
	{
		bool Enabled;
		int ReferenceValue;
		DepthStencilComparison Comparison;
		DepthStencilOperation PassOperation;
		DepthStencilOperation FailureOperation;
		DepthStencilOperation DepthFailureOperation;
	};

	auto operator==(const StencilOptions& left, const StencilOptions& right) -> bool;
	auto operator!=(const StencilOptions& left, const StencilOptions& right) -> bool;
	void ToString(StencilOptions options, StringWriter& writer);
	auto FromString(StencilOptions& options, StringReader& reader) -> bool;

	enum class SampleFilterMethod
	{
		Nearest,
		Bilinear,
		Trilinear,
		Anisotropic
	};

	enum class SampleAddressMode
	{
		Clamp,
		Repeat,
		Mirror
	};

	struct SampleOptions
	{
		SampleFilterMethod FilterMethod;
		SampleAddressMode HorizontalAddressMode;
		SampleAddressMode VerticalAddressMode;
	};

	auto operator==(const SampleOptions& left, const SampleOptions& right) -> bool;
	auto operator!=(const SampleOptions& left, const SampleOptions& right) -> bool;
	void ToString(SampleOptions options, StringWriter& writer);
	auto FromString(SampleOptions& options, StringReader& reader) -> bool;

	struct MaterialOutput
	{
		BlendOptions ColorBlending;
		BlendOptions AlphaBlending;
	};

	extern const BlendOptions DisabledBlending;
	extern const BlendOptions StandardBlending;
	extern const BlendOptions AdditiveBlending;
	extern const BlendOptions PremultipliedBlending;

	extern const DepthOptions DisabledDepthTesting;
	extern const DepthOptions StandardDepthTesting;
	extern const DepthOptions ReadOnlyDepthTesting;
	extern const DepthOptions WriteOnlyDepthTesting;

	extern const StencilOptions DisabledStenciling;

	extern const SampleOptions NearestSampling;
	extern const SampleOptions BilinearSampling;
	extern const SampleOptions TrilinearSampling;
	extern const SampleOptions AnisotropicSampling;

	struct ShaderCompilationResult
	{
		bool Success;
		String Identifier;
		List<String> Errors;

		void WriteResult(Log& log);
	};

	class Material : public GraphicsResource<Material>
	{
	public:
		Buffer VertexShader;
		Buffer FragmentShader;

		MaterialCullMode CullMode = MaterialCullMode::None;
		MaterialFillMode FillMode = MaterialFillMode::Fill;

		DepthOptions DepthOptions = DisabledDepthTesting;
		StencilOptions StencilOptions = DisabledStenciling;
		List<SampleOptions> SampleOptions;

		List<ShaderElement> VertexLayout;
		List<ShaderElement> InstanceLayout;
		List<MaterialOutput> Output;

		auto Identifier() const -> StringView;
		auto GetVertexSize() const -> std::size_t;
		auto GetInstanceSize() const -> std::size_t;

		void Reset();
		auto Reset(StringView filename) -> ShaderCompilationResult;
		auto Reset(StringView identifier, StringView shaderText) -> ShaderCompilationResult;

	protected:
		void Clear() override;

	private:
		friend class GraphicsDevice;
		using GraphicsResource<Material>::GraphicsResource;

		String _identifier;
	};
}

inline
auto Pargon::Material::Identifier() const -> StringView
{
	return _identifier;
}
