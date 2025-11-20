package game

type GameConfig struct {
	TileWidth int64
	TileHeight int64
}
type GameTickHistory struct {
	Tick int64
	Mut mutations.Mutation
}
type GameRules struct {
	Mutations map[string]rules.MutationEntry
	Objects map[string]rules.ObjectsEntry
	States map[string]rules.StatesEntry
	Units map[string]rules.UnitsEntry
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
	History []GameTickHistory
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
		c.History = append(c.History, GameTickHistory{
			Tick: c.Ticks,
			Mut: mut,
		})
	}

	c.MutationsQueue = c.MutationsQueue[:0]
	c.Ticks++
}

func (c *GameCore) EnqueueMutation(mut mutations.Mutation) {
  c.MutationsQueue = append(c.MutationsQueue, mut)
}