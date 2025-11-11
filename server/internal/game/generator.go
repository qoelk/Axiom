package game

import (
	"math/rand"
	"time"

	"github.com/google/uuid"
	"github.com/ojrac/opensimplex-go"
)

func GenerateMap(config ConfigProperty) *TileMap {
	width, height := config.Width, config.Height
	tiles := make([]TileKey, width*height)
	noise := opensimplex.New(time.Now().UnixNano())
	scale := 0.03

	// Define thresholds for terrain classification
	// Tune these values to control biome distribution
	waterThreshold := -0.2 // below this → Water
	dirtThreshold := 0.0   // between waterThreshold and dirtThreshold → Dirt
	rockThreshold := 0.4   // above rockThreshold → Rock
	// between dirtThreshold and rockThreshold → Land

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			nx := float64(x) * scale
			ny := float64(y) * scale
			value := noise.Eval2(nx, ny)

			idx := y*width + x
			switch {
			case value > rockThreshold:
				tiles[idx] = Rock
			case value > dirtThreshold:
				tiles[idx] = Land
			case value > waterThreshold:
				tiles[idx] = Dirt
			default:
				tiles[idx] = Water
			}
		}
	}

	return &TileMap{
		Width:  width,
		Height: height,
		Tiles:  tiles,
	}
}

func GenerateObjects(state *GameState) *GameState {
	// Seed the RNG once per call (or better: pass in a seeded rand.Rand for determinism/testability)
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))

	// Configuration
	treeChance := 0.02       // 2% per eligible tile
	decorationChance := 0.01 // 1% per eligible tile

	tileMap := &state.Map
	objects := make(map[uuid.UUID]Object)

	// Helper: check if object can be placed on a tile
	canPlace := func(tile TileKey) bool {
		return tile == Land || tile == Dirt // e.g., no trees in water or on rock
	}

	for y := 0; y < tileMap.Height; y++ {
		for x := 0; x < tileMap.Width; x++ {
			tile := tileMap.Tiles[y*tileMap.Width+x]
			if !canPlace(tile) {
				continue
			}

			// Use float64 coordinates centered in the tile
			worldX := float64(x) + 0.5
			worldY := float64(y) + 0.5
			size := 0.5

			if rng.Float64() < treeChance {
				obj := Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: size,
					Name: "Tree",
					Key:  Tree,
				}
				objects[obj.ID] = obj
			} else if rng.Float64() < decorationChance {
				obj := Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: size * 0.6, // smaller
					Name: "Decoration",
					Key:  Decoration,
				}
				objects[obj.ID] = obj
			}
		}
	}

	// Return a new or updated GameState
	return &GameState{
		Map:     state.Map,
		Objects: objects,
	}
}
