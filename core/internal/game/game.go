package game

type GameConfig {
	TileWidth int64
	TileHeight int64
}

type GameRules struct {
	Mutations map[string]any
	Objects map[string]any
	States map[string]any
	Units map[string]any
}

type GameState struct {
   ObjectsIDs []uuid.UUID
   UnitIDs []uuid.UUID
   Objects map[uuid.UUID]*objects.Object
   Units map[uuid.UUID]*objects.Unit
   mu sync.Mutex
}
type GameCore struct {
	Players []players.Player
	Map tilemap.TileMap
	State GameState
	Ticks int64
	MutationsQueue []mutations.Mutation
	Rules GameRules

}

func (c *GameCore) New(cfg GameConfig) {
	c.Map = generators.GenerateMap(cfg.TileWidth, cfg.TileHeight)
	c.State.Units = generator.GenerateUnits(c.Map)

}

func (c *GameCore) Save() {
}

func (c *GameCore) Load() {
}

func (c *GameCore) Tick() {
	c.State.mu.Lock()
	defer c.State.mu.Unlock()
	for _, mut := range c.MutationsQueue {
		mutation := c.Rules.Mutations[mut.Type]
		mutation.Perform(c.State, mut)
	}

	c.MutationsQueue = []
}

func (c *GameCore) EnqueueMutation(mut mutations.Mutation) {
  c.MutationsQueue = append(c.MutationsQueue, mut)
}