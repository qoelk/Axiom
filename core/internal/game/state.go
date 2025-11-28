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

func (gs *GameState) ResolveMutation(mu MutationData) {
	switch mu.Type {
	case MoveMutation:
		gs.MoveMutation(mu.ActorID)
	case StateMutation:
		gs.StateMutation(mu.ActorID)
	case TransitionMutation:
		gs.NextStateMutation(mu.ActorID, mu.State, mu.D)
	case DeleteObjectMutation:
		gs.DeleteObject(mu.ActorID)
	case DeleteUnitMutation:
		gs.DeleteUnit(mu.ActorID)
	case HPMutation:
		gs.HPMutation(mu.ActorID, mu.D)
	default:
		// Optionally log or handle unknown mutation types
	}
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
