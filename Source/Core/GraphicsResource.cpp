#include "Pargon/Graphics/GraphicsDevice.h"
#include "Pargon/Graphics/GraphicsResource.h"
#include "Pargon/Serialization/StringReader.h"
#include "Pargon/Serialization/StringWriter.h"

using namespace Pargon;

void GraphicsResource_::Lock()
{
	assert(!_isLocked);

	std::lock_guard<std::mutex> lock(_graphics._resourceGuard);
	_isLocked = true;
}

void GraphicsResource_::Unlock()
{
	assert(_isLocked);

	std::lock_guard<std::mutex> lock(_graphics._resourceGuard);
	_isLocked = false;

	if (!_isChanged)
	{
		_isChanged = true;
		_graphics._pendingUpdates.Add(this);
	}
}

GraphicsResource_::GraphicsResource_(GraphicsDevice& graphics, GraphicsStorage storage, std::unique_ptr<GraphicsHandle_>&& handle) :
	_graphics(graphics),
	_storage(storage),
	_handle(std::move(handle)),
	_isLocked(true),
	_isChanged(false)
{
}

void GraphicsResource_::UpdateComplete()
{
	if (_storage == GraphicsStorage::TransferredToGpu)
		Clear();

	_isChanged = false;
}
