package game

import (
	"math"

	"core.axiom/internal/objects"
	"github.com/google/uuid"
)

type GameTickHistory struct {
	Tick int64
}

func (c *GameCore) ResolveMovement() []uuid.UUID {
	var moved []uuid.UUID

	// Compute movement vectors
	for _, id := range c.State.ObjectsIDs {
		obj := c.State.Objects[id]
		if obj.Velocity == 0 {
			continue
		}

		obj.DX, obj.DY = c.computeDisplacement(obj.Facing, obj.Velocity)
		c.State.Objects[id] = obj
	}

	// Resolve movement with collision
	for _, id := range c.State.ObjectsIDs {
		obj := c.State.Objects[id]
		if obj.Velocity == 0 {
			continue
		}

		if !c.wouldCollide(id, obj) {
			moved = append(moved, id)
		} else {
			// Halt on collision
			obj.DX = 0
			obj.DY = 0
			obj.Velocity = 0
		}
		c.State.Objects[id] = obj
	}

	return moved
}

// computeDisplacement returns integer movement deltas from facing (degrees) and speed.
func (c *GameCore) computeDisplacement(facing int64, speed float64) (dx, dy int64) {
	angleRad := float64(facing) * math.Pi / 180.0
	return int64(math.Cos(angleRad) * speed), int64(math.Sin(angleRad) * speed)
}

// wouldCollide checks if moving obj (by its current DX/DY) would collide with others.
func (c *GameCore) wouldCollide(id uuid.UUID, obj *objects.Object) bool {
	futureX0 := obj.X0 + obj.DX
	futureY0 := obj.Y0 + obj.DY
	futureX1 := obj.X1 + obj.DX
	futureY1 := obj.Y1 + obj.DY

	centerX := (futureX0 + futureX1) / 2
	centerY := (futureY0 + futureY1) / 2

	for _, otherID := range c.CollisionsMesh.ObjectsByPosition(centerX, centerY) {
		if otherID == id {
			continue
		}
		other := c.State.Objects[otherID]
		if futureX0 < other.X1 && futureX1 > other.X0 &&
			futureY0 < other.Y1 && futureY1 > other.Y0 {
			return true
		}
	}
	return false
}

func (c *GameCore) Tick() {
	c.State.mu.Lock()
	defer c.State.mu.Unlock()
	for _, mu := range c.PendingMutations {
		c.State.ResolveMutation(mu)
	}
	c.PendingMutations = c.PendingMutations[:0]
	c.Ticks++
}

func (c *GameCore) AppendMutations(mutations []MutationData) {
	c.State.mu.Lock()
	defer c.State.mu.Unlock()
	c.PendingMutations = append(c.PendingMutations, mutations...)
}
