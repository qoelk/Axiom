package simulation

import (
	"core.axiom/internal/game/simulation/units"
	"github.com/google/uuid"
)

type MutationType string

const (
	MoveMutation         MutationType = "m"
	StateMutation        MutationType = "s"
	TransitionMutation   MutationType = "t"
	DeleteObjectMutation MutationType = "do"
	DeleteUnitMutation   MutationType = "du"
	HPMutation           MutationType = "h"
)

type MutationData struct {
	ActorID uuid.UUID
	D       int64
	State   units.StateKey
	Type    MutationType
}

func (gs *Simulation) resolveMutation(mu MutationData) {
	switch mu.Type {
	case MoveMutation:
		gs.moveMutation(mu.ActorID)
	case StateMutation:
		gs.stateMutation(mu.ActorID)
	case TransitionMutation:
		gs.nextStateMutation(mu.ActorID, mu.State, mu.D)
	case DeleteObjectMutation:
		gs.deleteObject(mu.ActorID)
	case DeleteUnitMutation:
		gs.deleteUnit(mu.ActorID)
	case HPMutation:
		gs.hpMutation(mu.ActorID, mu.D)
	default:
		// Optionally log or handle unknown mutation types
	}
}

func (gs *Simulation) moveMutation(actorID uuid.UUID) {
	actor := gs.state.Objects[actorID]
	actor.X0 += actor.DX
	actor.X1 += actor.DX
	actor.Y0 += actor.DY
	actor.Y1 += actor.DY
}

func (gs *Simulation) stateMutation(actorID uuid.UUID) {
	actor := gs.state.Units[actorID]
	if actor.State.TicksLeft == 0 {
		actor.State.Current = actor.State.Next
	} else {
		actor.State.TicksLeft--
	}
}

func (gs *Simulation) nextStateMutation(actorID uuid.UUID, state units.StateKey, ticksLeft int64) {
	actor := gs.state.Units[actorID]
	actor.State.Next = state
	actor.State.TicksLeft = ticksLeft
}

func (gs *Simulation) deleteObject(objectID uuid.UUID) {
	delete(gs.state.Objects, objectID)
}

func (gs *Simulation) deleteUnit(unitID uuid.UUID) {
	unit := gs.state.Units[unitID]
	gs.deleteObject(unit.ObjectID)
	delete(gs.state.Units, unitID)
}

func (gs *Simulation) hpMutation(actorID uuid.UUID, d int64) {
	actor := gs.state.Units[actorID]
	actor.HP += d
	if actor.HP <= 0 {
		delete(gs.state.Units, actorID)
	}
}
