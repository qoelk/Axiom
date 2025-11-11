package game

type Game struct {
	State  GameState
	config ConfigProperty
}

func NewGame() *Game {
	var game Game
	config := Config
	game.State.Map = *GenerateMap(config)
	game.State = *GenerateObjects(&game.State)
	game.config = config
	return &game
}
