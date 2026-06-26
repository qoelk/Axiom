package game

import (
	"axiom/internal/game/config"
	"axiom/internal/game/state"
	"axiom/internal/game/subsystems"
	"axiom/internal/game/tilemap"
	"errors"
)

type Game struct {
	m       *tilemap.TileMap
	s       *state.State
	systems []subsystems.Subsystem
}

func NewGame(m *tilemap.TileMap, cfg *config.Config) (*Game, error) {
	err := m.Validate()
	if err != nil {
		return nil, err
	}

	if int(m.Players) != len(cfg.Players) {
		return nil, errors.New("incompatable game config")
	}
	g := Game{
		m: m,
	}
	return &g, nil
}

func (g *Game) Map() *tilemap.TileMap {
	return g.m
}

func (g *Game) Tick() {
}
