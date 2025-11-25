package game

import (
	"encoding/json"

	"core.axiom/internal/generators"
	"core.axiom/internal/physics"
	"core.axiom/internal/players"
	"core.axiom/internal/tilemap"
)

type GameCore struct {
	Players          []players.Player
	Map              tilemap.TileMap
	State            GameState
	Ticks            int64
	History          []GameTickHistory
	PendingMutations []MutationData
	CollisionsMesh   *physics.CollisionsMesh
}

func NewGame(cfg GameConfig) *GameCore {
	c := &GameCore{}
	c.Map = generators.GenerateMap(cfg.TileWidth, cfg.TileHeight)
	c.Players = make([]players.Player, 0, cfg.PlayersCount)
	for i := 0; i < int(cfg.PlayersCount); i++ {
		c.Players = append(c.Players, players.NewPlayer(int64(i)))
	}
	c.State.Units = generators.GenerateUnits(c.Map, c.Players)
	return c
}

func (c *GameCore) Serialize() []byte {
	data, _ := json.Marshal(c)
	return data
}
