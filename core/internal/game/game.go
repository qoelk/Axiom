package game

import (
	"core.axiom/internal/game/simulation"
	"core.axiom/internal/players"
	"core.axiom/internal/tilemap"
)

type GameCore struct {
	Players    []players.Player
	Map        tilemap.TileMap
	Simulation simulation.Simulation
}

func NewGame(cfg GameConfig) *GameCore {
	return nil
}

func (c *GameCore) Run()   {}
func (c *GameCore) Pause() {}
func (c *GameCore) Stop()  {}

func (c *GameCore) GetState() {}

func (c *GameCore) PerformAction() {}

func (c *GameCore) ToDump() {}

func (c *GameCore) DumpToReplay() {}
