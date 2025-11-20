package game

import (
	"math"
	"sync"
	"time"

	"github.com/google/uuid"
)

type ConfigProperty struct {
	Width     int
	Height    int
	TickDelay time.Duration
}
type TileKey = int

type TileProperty struct {
	IsLand     bool
	IsWater    bool
	IsPassable bool
}

type TilePropertyMap map[TileKey]TileProperty

type TileMap struct {
	Width  int
	Height int
	Tiles  []TileKey
}

func (tm *TileMap) GetTile(x, y float64) TileKey {
	ix := int(math.Floor(x))
	iy := int(math.Floor(y))

	if ix < 0 || iy < 0 || ix >= tm.Width || iy >= tm.Height {
		return None
	}

	index := iy*tm.Width + ix
	return tm.Tiles[index]
}

type ObjectKey = int

type Object struct {
	ID   uuid.UUID
	X    float64
	Y    float64
	Size float64
	Name string
	Key  ObjectKey
}

type ObjectProperty struct {
	IsSelectable bool
	IsTargetable bool
}

type ObjectPropertyMap map[ObjectKey]ObjectProperty

type Entity struct {
	Object
	Facing   float64
	Velocity float64
}

func (e *Entity) Stop() {
	e.Velocity = 0
}

func (e *Entity) StartMoving() {
	e.Velocity = 0.01
}

func (e *Entity) NextTickCoordinates() (float64, float64) {
	newX := e.X + e.Velocity*math.Cos(e.Facing)
	newY := e.Y + e.Velocity*math.Sin(e.Facing)
	return newX, newY
}

type UnitKey = string

type UnitProperty struct {
	IsBuilding    bool
	ProdutionTime int
	MaxHP         int
	Speed         int
}

type UnitPropertyMap map[UnitKey]UnitProperty

type Unit struct {
	Entity
	HP    int
	Owner int
	IsBuilding bool
}

type Projectile struct {
	Entity
	IsVisible bool
}

type StateKey = string

type State struct {
	CurrentTick int
}

type StateProperty struct {
	IsInterruptable bool
	IsFinite        bool
	Duration        int
}

type StatePropertyMap map[StateKey]StateProperty

type GameState struct {
	mu      sync.Mutex
	Map     TileMap
	Objects map[uuid.UUID]*Object
	Units   map[uuid.UUID]*Unit
}

func (gs *GameState) Lock() {
	gs.mu.Lock()
}

func (gs *GameState) Unlock() {
	gs.mu.Unlock()
}
