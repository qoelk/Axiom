package main

import (
	"encoding/json"
	"log"
	"net/http"

	"axiom/internal/game"

	"github.com/google/uuid"
	"github.com/samber/lo"
)

var app game.Game

type TileMap struct {
	Width  int   `json:"width"`
	Height int   `json:"height"`
	Tiles  []int `json:"tiles"`
}

type Object struct {
	ID   uuid.UUID `json:"id"`
	X    float64   `json:"x"`
	Y    float64   `json:"y"`
	Size float64   `json:"size"`
	Key  string    `json:"key"`
}

type Unit struct {
	Facing   float64 `json:"facing"`
	Velocity float64 `json:"velocity"`
	Owner    int     `json:"owner"`
	Object
}

type GameState struct {
	Map         TileMap              `json:"map"`
	Objects     map[uuid.UUID]Object `json:"objects"`
	Projectiles map[uuid.UUID]Object `json:"projectiles"`
	Units       map[uuid.UUID]Unit   `json:"units"`
	Ticks       int                  `json:"tick"`
}

func init() {
	app = *game.NewGame()
}

func getStateHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	// Helper to convert game.ObjectKey to string
	keyToString := func(key game.ObjectKey) string {
		switch key {
		case game.Tree:
			return "tree"
		case game.Decoration:
			return "decoration"
		default:
			return "unknown"
		}
	}

	gameState := GameState{
		Map: TileMap{
			Width:  app.State.Map.Width,
			Height: app.State.Map.Height,
			Tiles: lo.Map(app.State.Map.Tiles, func(item game.TileKey, _ int) int {
				switch item {
				case game.Land:
					return 1
				case game.Dirt:
					return 2
				case game.Rock:
					return 3
				default:
					return 0
				}
			}),
		},
		Objects:     make(map[uuid.UUID]Object),
		Projectiles: make(map[uuid.UUID]Object),
		Units:       make(map[uuid.UUID]Unit),
	}

	// Convert Objects
	for _, o := range app.State.Objects {
		gameState.Objects[o.ID] = Object{
			ID:   o.ID,
			X:    o.X,
			Y:    o.Y,
			Size: o.Size,
			Key:  keyToString(o.Key),
		}
	}

	// Convert Units
	for _, u := range app.State.Units {
		gameState.Units[u.ID] = Unit{
			Facing:   u.Facing,
			Velocity: u.Velocity,
			Owner:    u.Owner,
			Object: Object{
				ID:   u.ID,
				X:    u.X,
				Y:    u.Y,
				Size: u.Size,
			},
		}
	}

	if err := json.NewEncoder(w).Encode(gameState); err != nil {
		log.Printf("Error encoding state: %v", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func main() {
	http.HandleFunc("/", getStateHandler)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
