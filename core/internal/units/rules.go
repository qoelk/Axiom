package units

import "github.com/google/uuid"

type (
	UnitKey  string
	UnitType struct {
		Key           UnitKey
		IsBuilding    bool
		Width         int64
		Height        int64
		MaxHP         int64
		MovementSpeed float64
	}
)

type Unit struct {
	ID       uuid.UUID
	ObjectID uuid.UUID
	Type     UnitKey
	Owner    uuid.UUID
	HP       int64
	State    UnitState
}

const (
	PEASANT  UnitKey = "PEASANT"
	FOOTMAN  UnitKey = "FOOTMAN"
	ARCHER   UnitKey = "ARCHER"
	MACEMAN  UnitKey = "MACEMAN"
	CASTLE   UnitKey = "CASTLE"
	BARRACKS UnitKey = "BARRACKS"
)

var UnitsBook = map[UnitKey]UnitType{
	PEASANT: {
		Key:           PEASANT,
		IsBuilding:    false,
		Width:         1,
		Height:        1,
		MaxHP:         50,
		MovementSpeed: 2.5,
	},
	FOOTMAN: {
		Key:           FOOTMAN,
		IsBuilding:    false,
		Width:         1,
		Height:        1,
		MaxHP:         100,
		MovementSpeed: 2.0,
	},
	ARCHER: {
		Key:           ARCHER,
		IsBuilding:    false,
		Width:         1,
		Height:        1,
		MaxHP:         80,
		MovementSpeed: 2.2,
	},
	MACEMAN: {
		Key:           MACEMAN,
		IsBuilding:    false,
		Width:         1,
		Height:        1,
		MaxHP:         110,
		MovementSpeed: 1.8,
	},
	CASTLE: {
		Key:           CASTLE,
		IsBuilding:    true,
		Width:         4,
		Height:        4,
		MaxHP:         2000,
		MovementSpeed: 0.0,
	},
	BARRACKS: {
		Key:           BARRACKS,
		IsBuilding:    true,
		Width:         2,
		Height:        2,
		MaxHP:         800,
		MovementSpeed: 0.0,
	},
}
