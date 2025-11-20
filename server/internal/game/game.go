package game

import "time"

type Game struct {
	State      GameState
	config     ConfigProperty
	simulation *GameSimulation
}

func NewGame() *Game {
	var game Game
	config := Config
	game.State.Map = *GenerateMap(config)
	game.State = *GenerateObjects(&game.State)
	game.State = *GenerateUnits(&game.State)
	game.config = config
	game.simulation = &GameSimulation{state: &game.State}
	game.startTicking()
	return &game
}

func (g *Game) startTicking() {
	ticker := time.NewTicker(g.config.TickDelay)
	go func() {
		for range ticker.C {
			g.simulation.Tick()
		}
	}()
}
