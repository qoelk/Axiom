package game

import (
	"sync"

	"core.axiom/internal/objects"
	"core.axiom/internal/units"
	"github.com/google/uuid"
)

type GameState struct {
	ObjectsIDs []uuid.UUID
	UnitIDs    []uuid.UUID
	Objects    map[uuid.UUID]*objects.Object
	Units      map[uuid.UUID]*units.Unit
	mu         sync.Mutex
}
type MutationData struct {
	ActorID uuid.UUID
}

func (gs *GameState) MoveMutation(actorID uuid.UUID) {
	actor := gs.Objects[actorID]
	actor.X0 += actor.DX
	actor.X1 += actor.DX
	actor.Y0 += actor.DY
	actor.Y1 += actor.DY
}

func (gs *GameState) StateMutation(actorID uuid.UUID) {
	actor := gs.Units[actorID]
	if actor.State.TicksLeft == 0 {
		actor.State.Current = actor.State.Next
	} else {
		actor.State.TicksLeft--
	}
}

func (gs *GameState) NextStateMutation(actorID uuid.UUID, state units.StateKey, ticksLeft int64) {
	actor := gs.Units[actorID]
	actor.State.Next = state
	actor.State.TicksLeft = ticksLeft
}

func (gs *GameState) DeleteObject(objectID uuid.UUID) {
	delete(gs.Objects, objectID)
}

func (gs *GameState) DeleteUnit(unitID uuid.UUID) {
	unit := gs.Units[unitID]
	gs.DeleteObject(unit.ObjectID)
	delete(gs.Units, unitID)
}

func (gs *GameState) HPMutation(actorID uuid.UUID, d int64) {
	actor := gs.Units[actorID]
	actor.HP += d
	if actor.HP <= 0 {
		delete(gs.Units, actorID)
	}
}
