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
