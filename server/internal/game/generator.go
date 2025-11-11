package game

import (
	"math"
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
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))

	// Increased chances
	treeChance := 0.08          // up from 0.02 → 8%
	decorationChance := 0.04    // up from 0.01 → 4%
	unitPromotionChance := 0.25 // 25% of eligible objects become units

	tileMap := &state.Map
	objects := make(map[uuid.UUID]*Object)
	units := make(map[uuid.UUID]*Unit)

	canPlace := func(tile TileKey) bool {
		return tile == Land || tile == Dirt
	}

	for y := 0; y < tileMap.Height; y++ {
		for x := 0; x < tileMap.Width; x++ {
			tile := tileMap.Tiles[y*tileMap.Width+x]
			if !canPlace(tile) {
				continue
			}

			worldX := float64(x) + 0.5
			worldY := float64(y) + 0.5

			// Decide what to place
			r := rng.Float64()
			var obj Object
			var isUnit bool

			if r < treeChance {
				obj = Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: 0.5,
					Name: "Tree",
					Key:  Tree,
				}
			} else if r < treeChance+decorationChance {
				obj = Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: 0.5 * 0.6,
					Name: "Decoration",
					Key:  Decoration,
				}
			} else {
				continue // nothing placed
			}

			// Possibly promote to Unit (only certain keys make sense as units)
			if obj.Key == Tree || obj.Key == Decoration {
				if rng.Float64() < unitPromotionChance {
					isUnit = true
				}
			}

			if isUnit {
				// Convert to Unit: note that Unit embeds Entity, which embeds Object
				unit := Unit{
					Entity: Entity{
						Object:   obj,
						Facing:   rng.Float64() * 2 * math.Pi, // random direction in radians
						Velocity: 0.01,                        // stationary by default
					},
					HP:    100,             // default HP
					Owner: 1 + rng.Intn(2), // 1 or 2
				}
				units[unit.ID] = &unit
				// Do NOT add to objects map
			} else {
				objects[obj.ID] = &obj
			}
		}
	}

	return &GameState{
		Map:     state.Map,
		Objects: objects,
		Units:   units,
	}
}
