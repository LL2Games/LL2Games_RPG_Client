# Monster projectile spawn contract

`PKT_PROJECTILE_MOVE (0x0044)` is a spawn batch, despite its name. Its payload is
`count`, followed by nine length-prefixed string fields per projectile:

`instanceId, projectileTypeId, ownerMonsterId, dirX, dirY, range, speed, xPos, yPos`

The first three fields are integers. Direction, range, speed and position are
floats. Direction is used as received, without normalization, integer conversion
or conversion to a facing sign. Negative counts, truncated batches and extra
fields are rejected before spawning any projectile from that batch.

Position uses the same world coordinates as monsters and players: x increases
rightward and y increases downward. Rendering subtracts the camera distance;
there is no additional axis flip or world-unit scaling.

Each frame applies `position += direction * speed * max(deltaTime, 0)` and adds
the Euclidean length of that displacement to travelled distance. A projectile
expires when travelled distance reaches range. There is no receive timeout or
position correction, because no subsequent position/despawn packet is expected.
Non-finite fields, negative IDs/ranges, nonpositive speeds and zero directions
are rejected as invalid moving-projectile spawn information.

Duplicate instance IDs are ignored, including IDs already retired in this map.
Map/scene exit and disconnect clear both visuals and remembered IDs. The wire
packet has no map ID, so it cannot distinguish a delayed old-map packet from a
new-map spawn; the server must preserve map-transition packet ordering.

Debug logs use the existing Logger (debugger output and `client_<characterId>.log`
when initialized). Search for:

- `[Projectile receive]`: ID, type/owner, original direction/position, speed, range.
- `[Projectile move]`: ID, before/after position, direction, speed, range, travelled distance, dt.
- `[Projectile remove]`: ID, removal reason and final movement values.
- `[Projectile duplicate]`, `[Projectile reject]`, `[Projectile clear]`: lifecycle decisions.

The regression test exercises the real decimal-string packet receiver,
diagonal movement, survival beyond two seconds, range expiry, duplicate spawn
handling, ID reuse after map exit, and matching trajectories for two simulated
clients with the same spawn and elapsed time. It does not prove server broadcast
visibility or live collision timing.

For live verification, place two players in the same map and compare receive
logs for the same instance ID and spawn fields. Confirm matching world paths,
range removal, and absence of lingering visuals after leaving/re-entering.
Compare the server hit timestamp/position with the visible projectile at impact.
Receiving an already-advanced server position with the full range causes the
client to travel slightly farther than the server; exact expiry requires remaining
range or explicit expiry information from the server. Receive latency also limits
visual/hit synchronization without a server timestamp.
