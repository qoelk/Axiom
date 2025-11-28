package players

import "github.com/google/uuid"

type Player struct {
	ID   uuid.UUID
	Team int64
}

func NewPlayer(team int64) Player {
	return Player{
		ID:   uuid.New(),
		Team: team,
	}
}

