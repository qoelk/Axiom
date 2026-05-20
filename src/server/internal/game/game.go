package game

import (
	"errors"

	"axiom/internal/game/config"
	"axiom/internal/game/tilemap"
)

type Game struct {
	m *tilemap.TileMap
}

func (g *Game) Map() *tilemap.TileMap {
	return g.m
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
