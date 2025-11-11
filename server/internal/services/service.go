package services

import (
	"errors"

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

func BuildFullGameState(g *game.Game) GameState {
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
