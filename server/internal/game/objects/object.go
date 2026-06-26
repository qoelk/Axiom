package objects

import "github.com/google/uuid"

type Object struct {
	ID           uuid.UUID
	ObjectID     uuid.UUID
	X            int
	Y            int
	Width        int
	Height       int
	Velocity     int
	Acceleration int
	Facing       float64
}
