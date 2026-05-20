package tilemap

import (
	"encoding/json"
	"errors"
	"os"
	"slices"
)

type TileMap struct {
	Width   uint64  `json:"width"`
	Height  uint64  `json:"height"`
	Tiles   []int64 `json:"tiles"`
	Players uint64  `json:"players"`
}

func (m *TileMap) Validate() error {
	if slices.Contains([]uint64{m.Width, m.Height, m.Players}, 0) {
		return errors.New("invalid map")
	}
	if len(m.Tiles) != int(m.Width*m.Height) {
		return errors.New("invalid map")
	}
	return nil
}

func FromFile(path string) (*TileMap, error) {
	var gm TileMap
	rawMap, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	err = json.Unmarshal(rawMap, &gm)
	if err != nil {
		return nil, err
	}
	return &gm, nil
}
