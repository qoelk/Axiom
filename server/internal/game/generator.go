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

	treeChance := 0.08       // 8%
	decorationChance := 0.04 // 4%

	tileMap := &state.Map
	objects := make(map[uuid.UUID]*Object)

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

			r := rng.Float64()
			if r < treeChance {
				obj := &Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: 0.5,
					Name: "Tree",
					Key:  Tree,
				}
				objects[obj.ID] = obj
			} else if r < treeChance+decorationChance {
				obj := &Object{
					ID:   uuid.New(),
					X:    worldX,
					Y:    worldY,
					Size: 0.3, // 0.5 * 0.6
					Name: "Decoration",
					Key:  Decoration,
				}
				objects[obj.ID] = obj
			}
		}
	}

	return &GameState{
		Map:     state.Map,
		Objects: objects,
		Units:   state.Units, // preserve existing units (or pass empty)
	}
}

// GenerateUnits generates military units + one building per owner, clustered
func GenerateUnits(state *GameState) *GameState {
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))
	tileMap := &state.Map

	units := make(map[uuid.UUID]*Unit)

	canPlace := func(x, y int) bool {
		if x < 0 || y < 0 || x >= tileMap.Width || y >= tileMap.Height {
			return false
		}
		tile := tileMap.Tiles[y*tileMap.Width+x]
		return tile == Land || tile == Dirt
	}

	// Helper: find a valid cluster center
	findValidCenter := func(attemptX, attemptY int, radius int) (int, int, bool) {
		for dy := -radius; dy <= radius; dy++ {
			for dx := -radius; dx <= radius; dx++ {
				x, y := attemptX+dx, attemptY+dy
				if canPlace(x, y) {
					return x, y, true
				}
			}
		}
		return 0, 0, false
	}

	// Define base positions (you can randomize or use corners)
	centers := []struct{ x, y int }{
		{tileMap.Width / 4, tileMap.Height / 4},           // Owner 1: top-left quadrant
		{3 * tileMap.Width / 4, 3 * tileMap.Height / 4},   // Owner 2: bottom-right quadrant
	}

	numRegularUnits := 5 // per owner

	for ownerIdx, center := range centers {
		ownerID := ownerIdx + 1 // 1 or 2

		// Find a valid cluster center near desired location
		cx, cy, ok := findValidCenter(center.x, center.y, 10)
		if !ok {
			continue // skip if no valid spot
		}

		// Place the building (size 3, isBuilding = true)
		building := &Unit{
			Entity: Entity{
				Object: Object{
					ID:   uuid.New(),
					X:    float64(cx) + 0.5,
					Y:    float64(cy) + 0.5,
					Size: 3.0,
					Name: "Headquarters",
				},
				Facing: 0,
			},
			HP:         100,
			Owner:      ownerID,
			IsBuilding: true,
		}
		units[building.ID] = building

		// Place regular units around the building (within radius ~3 tiles)
		for i := 0; i < numRegularUnits; i++ {
			attempts := 0
			for attempts < 50 {
				dx := rng.Intn(7) - 3 // -3 to +3
				dy := rng.Intn(7) - 3
				x, y := cx+dx, cy+dy

				if canPlace(x, y) {
					unit := &Unit{
						Entity: Entity{
							Object: Object{
								ID:   uuid.New(),
								X:    float64(x) + 0.5,
								Y:    float64(y) + 0.5,
								Size: 0.5,
								Name: "Soldier",
							},
							Facing: rng.Float64() * 2 * math.Pi,
						},
						HP:         10,
						Owner:      ownerID,
						IsBuilding: false,
					}
					units[unit.ID] = unit
					break
				}
				attempts++
			}
		}
	}

	return &GameState{
		Map:     state.Map,
		Objects: state.Objects,
		Units:   units,
	}
}