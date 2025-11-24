package game

type GameTickHistory struct {
	Tick int64
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
