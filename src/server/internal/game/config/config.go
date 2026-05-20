package config

import (
	"encoding/json"
	"os"
)

type Config struct {
	Players []any `json:"players"`
}

func FromFile(path string) (*Config, error) {
	var cfg Config
	rawCfg, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	err = json.Unmarshal(rawCfg, &cfg)
	if err != nil {
		return nil, err
	}
	return &cfg, nil
}
