#include "Pargon/Application/Log.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "DirectX11/DirectX11Material.h"
#include "DirectX11/DirectX11Renderer.h"

using namespace Pargon;

namespace
{
	auto GetFillMode(MaterialFillMode mode) -> D3D11_FILL_MODE
	{
		return mode == MaterialFillMode::Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	}

	auto GetCullMode(MaterialCullMode mode) -> D3D11_CULL_MODE
	{
		switch (mode)
		{
		case MaterialCullMode::None: return D3D11_CULL_NONE;
		case MaterialCullMode::Clockwise: return D3D11_CULL_FRONT;
		case MaterialCullMode::CounterClockwise: return D3D11_CULL_BACK;
		}

		return D3D11_CULL_NONE;
	}

	auto GetBlendFactor(BlendFactor factor) -> D3D11_BLEND
	{
		switch (factor)
		{
		case BlendFactor::Zero: return D3D11_BLEND_ZERO;
		case BlendFactor::One: return D3D11_BLEND_ONE;
		case BlendFactor::SourceColor: return D3D11_BLEND_SRC_COLOR;
		case BlendFactor::InverseSourceColor: return D3D11_BLEND_INV_SRC_COLOR;
		case BlendFactor::SourceAlpha: return D3D11_BLEND_SRC_ALPHA;
		case BlendFactor::InverseSourceAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
		case BlendFactor::DestinationColor: return D3D11_BLEND_DEST_COLOR;
		case BlendFactor::InverseDestinationColor: return D3D11_BLEND_INV_DEST_COLOR;
		case BlendFactor::DestinationAlpha: return D3D11_BLEND_DEST_ALPHA;
		case BlendFactor::InverseDestinationAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
		}

		return D3D11_BLEND_ZERO;
	}

	auto GetBlendOperation(BlendOperation operation) -> D3D11_BLEND_OP
	{
		switch (operation)
		{
		case BlendOperation::Add: return D3D11_BLEND_OP_ADD;
		case BlendOperation::Subtract: return D3D11_BLEND_OP_SUBTRACT;
		case BlendOperation::Invert: return D3D11_BLEND_OP_REV_SUBTRACT;
		case BlendOperation::Minimum: return D3D11_BLEND_OP_MIN;
		case BlendOperation::Maximum: return D3D11_BLEND_OP_MAX;
		};

		return D3D11_BLEND_OP_ADD;
	}

	auto GetComparison(DepthStencilComparison comparison) -> D3D11_COMPARISON_FUNC
	{
		switch (comparison)
		{
		case DepthStencilComparison::Never: return D3D11_COMPARISON_NEVER;
		case DepthStencilComparison::Always: return D3D11_COMPARISON_ALWAYS;
		case DepthStencilComparison::Equal: return D3D11_COMPARISON_EQUAL;
		case DepthStencilComparison::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
		case DepthStencilComparison::Less: return D3D11_COMPARISON_LESS;
		case DepthStencilComparison::LessOrEqual: return D3D11_COMPARISON_LESS_EQUAL;
		case DepthStencilComparison::Greater: return D3D11_COMPARISON_GREATER;
		case DepthStencilComparison::GreaterOrEqual: return D3D11_COMPARISON_GREATER_EQUAL;
		}

		return D3D11_COMPARISON_NEVER;
	}

	auto GetStencilOperation(DepthStencilOperation operation) -> D3D11_STENCIL_OP
	{
		switch (operation)
		{
		case DepthStencilOperation::Keep: return D3D11_STENCIL_OP_KEEP;
		case DepthStencilOperation::Zero: return D3D11_STENCIL_OP_ZERO;
		case DepthStencilOperation::Replace: return D3D11_STENCIL_OP_REPLACE;
		case DepthStencilOperation::Increase: return D3D11_STENCIL_OP_INCR_SAT;
		case DepthStencilOperation::IncreaseWrap: return D3D11_STENCIL_OP_INCR;
		case DepthStencilOperation::Decrease: return D3D11_STENCIL_OP_DECR_SAT;
		case DepthStencilOperation::DecreaseWrap: return D3D11_STENCIL_OP_DECR;
		case DepthStencilOperation::Invert: return D3D11_STENCIL_OP_INVERT;
		}

		return D3D11_STENCIL_OP_KEEP;
	}

	auto GetFilterMethod(SampleFilterMethod method) -> D3D11_FILTER
	{
		switch (method)
		{
		case SampleFilterMethod::Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case SampleFilterMethod::Bilinear: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case SampleFilterMethod::Trilinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case SampleFilterMethod::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
		}

		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	auto GetAddressMode(SampleAddressMode mode) -> D3D11_TEXTURE_ADDRESS_MODE
	{
		switch (mode)
		{
		case SampleAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
		case SampleAddressMode::Repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
		case SampleAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
		}

		return D3D11_TEXTURE_ADDRESS_CLAMP;
	}

	auto GetUsage(ShaderElementUsage usage) -> const char*
	{
		switch (usage)
		{
			case ShaderElementUsage::Position: return "POSITION";
			case ShaderElementUsage::Color: return "COLOR";
			case ShaderElementUsage::Coordinate: return "TEXCOORD";
			case ShaderElementUsage::Other: return "CUSTOM";
		}

		return "";
	}

	auto GetType(ShaderElementType type) -> DXGI_FORMAT
	{
		switch (type)
		{
		case ShaderElementType::Float: return DXGI_FORMAT_R32_FLOAT;
		case ShaderElementType::Vector2: return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderElementType::Vector3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderElementType::Vector4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ShaderElementType::Int: return DXGI_FORMAT_R32_SINT;
		case ShaderElementType::Point2: return DXGI_FORMAT_R32G32_SINT;
		case ShaderElementType::Point3: return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderElementType::Point4: return DXGI_FORMAT_R32G32B32A32_SINT;
		case ShaderElementType::Color: return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	auto GetLayoutDescription(SequenceView<ShaderElement> vertexElements, SequenceView<ShaderElement> instanceElements) -> List<D3D11_INPUT_ELEMENT_DESC>
	{
		auto layout = List<D3D11_INPUT_ELEMENT_DESC>();

		Array<int, EnumNames<ShaderElementUsage>.Count()> usageIndex = {{ 0 }};

		for (auto& element : vertexElements)
		{
			auto& item = layout.Increment();
			item.SemanticName = GetUsage(element.Usage);
			item.SemanticIndex = usageIndex.Item(static_cast<int>(element.Usage))++;
			item.Format = GetType(element.Type);
			item.InputSlot = 0;
			item.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			item.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			item.InstanceDataStepRate = 0;
		}

		for (auto& element : instanceElements)
		{
			auto& item = layout.Increment();
			item.SemanticName = GetUsage(element.Usage);
			item.SemanticIndex = usageIndex.Item(static_cast<int>(element.Usage))++;
			item.Format = GetType(element.Type);
			item.InputSlot = 1;
			item.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			item.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			item.InstanceDataStepRate = 1;
		}

		return layout;
	}
}

auto DirectX11MaterialHandle::Update(Material* material) -> bool
{
	auto renderer = static_cast<DirectX11Renderer*>(material->Graphics().Renderer());

	VertexShader.Reset();
	PixelShader.Reset();
	RasterizerState.Reset();
	BlendState.Reset();
	DepthStencilState.Reset();
	Layout.Reset();

	auto result = renderer->Device->CreateVertexShader(material->VertexShader.begin(), material->VertexShader.Size(), NULL, VertexShader.GetAddressOf());
	if (FAILED(result))
		return false;

	result = renderer->Device->CreatePixelShader(material->FragmentShader.begin(), material->FragmentShader.Size(), NULL, PixelShader.GetAddressOf());
	if (FAILED(result))
		return false;

	D3D11_RASTERIZER_DESC rasterizerDescription;
	rasterizerDescription.FillMode = GetFillMode(material->FillMode);
	rasterizerDescription.CullMode = GetCullMode(material->CullMode);
	rasterizerDescription.FrontCounterClockwise = false;
	rasterizerDescription.DepthBias = 0;
	rasterizerDescription.DepthBiasClamp = 0.0f;
	rasterizerDescription.SlopeScaledDepthBias = 0.0f;
	rasterizerDescription.DepthClipEnable = false;
	rasterizerDescription.ScissorEnable = true;
	rasterizerDescription.MultisampleEnable = true;
	rasterizerDescription.AntialiasedLineEnable = false;

	result = renderer->Device->CreateRasterizerState(&rasterizerDescription, RasterizerState.GetAddressOf());
	if (FAILED(result))
		return false;

	D3D11_BLEND_DESC blendDescription;
	blendDescription.AlphaToCoverageEnable = FALSE;
	blendDescription.IndependentBlendEnable = FALSE;

	for (auto i = 0; i < material->Output.Count(); i++)
	{
		auto& output = material->Output.Item(i);

		blendDescription.RenderTarget[i].BlendEnable = output.ColorBlending.Operation == BlendOperation::None ? FALSE : TRUE;
		blendDescription.RenderTarget[i].SrcBlend = GetBlendFactor(output.ColorBlending.SourceFactor);
		blendDescription.RenderTarget[i].DestBlend = GetBlendFactor(output.ColorBlending.DestinationFactor);
		blendDescription.RenderTarget[i].BlendOp = GetBlendOperation(output.ColorBlending.Operation);
		blendDescription.RenderTarget[i].SrcBlendAlpha = GetBlendFactor(output.AlphaBlending.SourceFactor);
		blendDescription.RenderTarget[i].DestBlendAlpha = GetBlendFactor(output.AlphaBlending.DestinationFactor);
		blendDescription.RenderTarget[i].BlendOpAlpha = GetBlendOperation(output.AlphaBlending.Operation);
		blendDescription.RenderTarget[i].RenderTargetWriteMask = material->FillMode == MaterialFillMode::None ? 0 : D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	result = renderer->Device->CreateBlendState(&blendDescription, BlendState.GetAddressOf());
	if (FAILED(result))
		return false;

	auto depth = material->DepthOptions;
	auto stencil = material->StencilOptions;

	D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
	depthStencilDescription.DepthEnable = depth.TestingEnabled ? TRUE : FALSE;
	depthStencilDescription.DepthWriteMask = depth.WritingEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDescription.DepthFunc = GetComparison(depth.Comparison);
	depthStencilDescription.StencilEnable = stencil.Enabled ? TRUE : FALSE;
	depthStencilDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDescription.FrontFace.StencilFailOp = GetStencilOperation(stencil.FailureOperation);
	depthStencilDescription.FrontFace.StencilDepthFailOp = GetStencilOperation(stencil.DepthFailureOperation);
	depthStencilDescription.FrontFace.StencilPassOp = GetStencilOperation(stencil.PassOperation);
	depthStencilDescription.FrontFace.StencilFunc = GetComparison(stencil.Comparison);
	depthStencilDescription.BackFace.StencilFailOp = GetStencilOperation(stencil.FailureOperation);
	depthStencilDescription.BackFace.StencilDepthFailOp = GetStencilOperation(stencil.DepthFailureOperation);
	depthStencilDescription.BackFace.StencilPassOp = GetStencilOperation(stencil.PassOperation);
	depthStencilDescription.BackFace.StencilFunc = GetComparison(stencil.Comparison);

	result = renderer->Device->CreateDepthStencilState(&depthStencilDescription, DepthStencilState.GetAddressOf());
	if (FAILED(result))
		return false;

	for (auto& sampler : material->SampleOptions)
	{
		D3D11_SAMPLER_DESC description;
		description.Filter = GetFilterMethod(sampler.FilterMethod);
		description.AddressU = GetAddressMode(sampler.HorizontalAddressMode);
		description.AddressV = GetAddressMode(sampler.VerticalAddressMode);
		description.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		description.MipLODBias = 0;
		description.MaxAnisotropy = 0;
		description.ComparisonFunc = D3D11_COMPARISON_NEVER;
		description.BorderColor[0] = 0;
		description.BorderColor[1] = 0;
		description.BorderColor[2] = 0;
		description.BorderColor[3] = 0;
		description.MinLOD = 0;
		description.MaxLOD = D3D11_FLOAT32_MAX;

		auto& state = SamplerStates.Increment();
		auto result = renderer->Device->CreateSamplerState(&description, state.GetAddressOf());
	}

	auto layout = GetLayoutDescription(material->VertexLayout, material->InstanceLayout);
	result = renderer->Device->CreateInputLayout(layout.begin(), layout.Count(), material->VertexShader.begin(), material->VertexShader.Size(), Layout.GetAddressOf());

	return true;
}