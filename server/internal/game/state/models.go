package state

import (
	"axiom/internal/game/objects"
	"sync"

	"github.com/google/uuid"
)

type Player struct {
	ID uuid.UUID
}

type Effect struct {
	ID       uuid.UUID
	EffectID uuid.UUID
	Duration int
}

type Mutation struct {
	ID    uuid.UUID
	Type  string
	Value any
}

type State struct {
	mu      sync.Mutex
	objects objects.Object
}
