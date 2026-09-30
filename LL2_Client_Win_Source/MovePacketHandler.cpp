#include "MovePacketHandler.h"
#include "MovementPacketHandler.h"

// Legacy 0x0020 responses are diagnostic only; they must never move an entity.
void MovePacketHandler::Execute(const ParsedPacket& packet)
{
    MovementPacketHandler::HandleInputResponse(packet);
}
