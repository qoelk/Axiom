package game

type GameTickHistory struct {
	Tick int64
}

func (c *GameCore) Tick() {
	c.State.mu.Lock()
	defer c.State.mu.Unlock()
	c.Ticks++
}
