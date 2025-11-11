package game

import (
	"github.com/google/uuid"
)

type ConfigProperty struct {
	Width     int
	Height    int
	TickDelay int
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
	Map     TileMap
	Objects map[uuid.UUID]Object
	Units   map[uuid.UUID]Unit
}
