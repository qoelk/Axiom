package game

import (
	"math"

	"github.com/google/uuid"
)

type GameSimulation struct {
	state *GameState
}

func (gs *GameSimulation) Tick() {
	gs.state.mu.Lock()
	defer gs.state.mu.Unlock()

	// Phase 2: Move units with full collision
	for id, u := range gs.state.Units {
		if u.Velocity == 0 {
			continue
		}

		nextX, nextY := u.NextTickCoordinates()
		s := u.Size

		// Check collisions in order (adjust priority if needed)
		if gs.checkTileCollision(nextX, nextY, s) ||
			gs.checkObjectCollision(nextX, nextY, s) ||
			gs.checkUnitCollision(nextX, nextY, s, id) {
			u.Stop()
		} else {
			u.X = nextX
			u.Y = nextY
		}
	}
}

func (gs *GameSimulation) checkTileCollision(x, y, size float64) bool {
	minX := int(math.Floor(x))
	maxX := int(math.Floor(x + size))
	minY := int(math.Floor(y))
	maxY := int(math.Floor(y + size))

	for ty := minY; ty <= maxY; ty++ {
		for tx := minX; tx <= maxX; tx++ {
			tile := gs.state.Map.GetTile(float64(tx), float64(ty))
			if !TileProperties[tile].IsPassable {
				return true
			}
		}
	}
	return false
}

func (gs *GameSimulation) checkObjectCollision(x, y, size float64) bool {
	unitLeft, unitRight := x, x+size
	unitBottom, unitTop := y, y+size

	for _, obj := range gs.state.Objects {

		objLeft, objRight := obj.X, obj.X+obj.Size
		objBottom, objTop := obj.Y, obj.Y+obj.Size

		// AABB vs AABB collision
		if unitRight > objLeft && unitLeft < objRight &&
			unitTop > objBottom && unitBottom < objTop {
			return true
		}
	}
	return false
}

func (gs *GameSimulation) checkUnitCollision(x, y, size float64, selfID uuid.UUID) bool {
	unitLeft, unitRight := x, x+size
	unitBottom, unitTop := y, y+size

	for id, other := range gs.state.Units {
		if id == selfID {
			continue // skip self
		}

		otherLeft, otherRight := other.X, other.X+other.Size
		otherBottom, otherTop := other.Y, other.Y+other.Size

		// AABB overlap test
		if unitRight > otherLeft && unitLeft < otherRight &&
			unitTop > otherBottom && unitBottom < otherTop {
			return true
		}
	}
	return false
}
