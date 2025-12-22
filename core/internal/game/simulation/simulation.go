package simulation

import (
	"sync"

	"core.axiom/internal/game/simulation/objects"
	"core.axiom/internal/game/simulation/units"
	"core.axiom/internal/physics"
	"github.com/google/uuid"
)

type SimulationState struct {
	ObjectsIDs []uuid.UUID
	UnitIDs    []uuid.UUID
	Objects    map[uuid.UUID]objects.Object
	Units      map[uuid.UUID]units.Unit
}
type Simulation struct {
	state          SimulationState
	mutations      []MutationData
	history        []GameTickHistory
	Ticks          int64
	CollisionsMesh *physics.CollisionsMesh
	mu             sync.Mutex
}

type GameSimulation interface {
	Tick()
	AppendMutations(mutations []MutationData)
	State() *SimulationState
}
