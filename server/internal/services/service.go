package services

import (
	"errors"
	"math"

	"axiom/internal/game"

	"github.com/google/uuid"
	"github.com/samber/lo"
)

type TileMap struct {
	Width  int   `json:"width"`
	Height int   `json:"height"`
	Tiles  []int `json:"tiles"`
}

type Object struct {
	ID   uuid.UUID `json:"id"`
	X    float64   `json:"x"`
	Y    float64   `json:"y"`
	Size float64   `json:"size"`
	Key  string    `json:"key"`
}

type Unit struct {
	Facing   float64 `json:"facing"`
	Velocity float64 `json:"velocity"`
	Owner    int     `json:"owner"`
	Object
}

type GameState struct {
	Map         TileMap              `json:"map"`
	Objects     map[uuid.UUID]Object `json:"objects"`
	Projectiles map[uuid.UUID]Object `json:"projectiles"`
	Units       map[uuid.UUID]Unit   `json:"units"`
	Ticks       int                  `json:"tick"`
}

func ConvertObjectKey(key game.ObjectKey) string {
	switch key {
	case game.Tree:
		return "tree"
	case game.Decoration:
		return "decoration"
	default:
		return "unknown"
	}
}

func ConvertTileKey(key game.TileKey) int {
	switch key {
	case game.Land:
		return 1
	case game.Dirt:
		return 2
	case game.Rock:
		return 3
	default:
		return 0
	}
}

func BuildFullGameState(g *game.Game, owner int) GameState {
	state := GameState{
		Objects:     make(map[uuid.UUID]Object),
		Projectiles: make(map[uuid.UUID]Object),
		Units:       make(map[uuid.UUID]Unit),
	}

	for _, o := range g.State.Objects {
		state.Objects[o.ID] = Object{
			ID:   o.ID,
			X:    o.X,
			Y:    o.Y,
			Size: o.Size,
			Key:  ConvertObjectKey(o.Key),
		}
	}

	for _, u := range g.State.Units {
		if owner != 0 && u.Owner != owner {
			continue
		}
		state.Units[u.ID] = Unit{
			Facing:   u.Facing,
			Velocity: u.Velocity,
			Owner:    u.Owner,
			Object: Object{
				ID:   u.ID,
				X:    u.X,
				Y:    u.Y,
				Size: u.Size,
			},
		}
	}

	return state
}

func BuildMapOnly(g *game.Game) GameState {
	return GameState{
		Map: TileMap{
			Width:  g.State.Map.Width,
			Height: g.State.Map.Height,
			Tiles: lo.Map(g.State.Map.Tiles, func(item game.TileKey, _ int) int {
				return ConvertTileKey(item)
			}),
		},
	}
}

func UpdateUnitFacing(g *game.Game, unitID uuid.UUID, facing float64) error {
	g.State.Lock()
	defer g.State.Unlock()

	unit, exists := g.State.Units[unitID]
	if !exists {
		return errors.New("unit not found")
	}

	unit.Facing = facing
	g.State.Units[unitID] = unit
	return nil
}

func MoveToPoint(g *game.Game, unitID uuid.UUID, x float64, y float64) error {
	g.State.Lock()
	defer g.State.Unlock()

	unit, exists := g.State.Units[unitID]
	if !exists {
		return errors.New("unit not found")
	}
	dx := x - unit.X
	dy := y - unit.Y

	// Compute facing angle in radians (angle from positive X-axis)
	facing := math.Atan2(dy, dx)

	unit.Facing = facing


	unit.Facing = facing
	unit.Velocity = 0.01
	g.State.Units[unitID] = unit
	return nil
}

func Damage(g *game.Game, srcID, targetID uuid.UUID) error {
	g.State.Lock()
	defer g.State.Unlock()

	src, srcExists := g.State.Units[srcID]
	if !srcExists {
		return errors.New("source unit not found")
	}

	target, targetExists := g.State.Units[targetID]
	if !targetExists {
		return errors.New("target unit not found")
	}

	// Apply lifesteal: source gains 1 HP, target takes 2 damage
	src.HP += 1
	target.HP -= 2

	// Save updated units back to state
	g.State.Units[srcID] = src
	g.State.Units[targetID] = target

	return nil
}