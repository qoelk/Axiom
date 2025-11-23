package game

import "core.axiom/internal/mutations"

type GameTickHistory struct {
	Tick int64
	Mut  mutations.MutationData
}

func (c *GameCore) Tick() {
	c.State.mu.Lock()
	defer c.State.mu.Unlock()
	c.Ticks++
}
