#include "Pargon/Application/Log.h"
#include "Pargon/Graphics/Renderer.h"

using namespace Pargon;

void RendererInformation::WriteInformation(Log& log)
{
	StringWriter writer;

	auto information = R"(Renderer Information
 - Type: {})"_sv;

	writer.Format(information, Type);

	for (auto& message : Messages)
		writer.Format(" - {}", message);

	log.Write(writer.GetString());
}

void RendererInformation::WriteCapabilities(Log& log)
{
	auto capabilities = R"(Renderer Capabilities
 - Sample Counts: {|-})"_sv;

	log.Write(capabilities, SupportedSampleCounts);
}
