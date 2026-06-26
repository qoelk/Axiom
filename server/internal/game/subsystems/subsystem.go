package subsystems

import "axiom/internal/game/state"

type Subsystem interface {
	Tick(state *state.State)
}
