package mutations

import "github.com/google/uuid"

type MutationData struct {
	ActorID uuid.UUID
	D       int64
	StateID uuid.UUID
	Type    string
}

