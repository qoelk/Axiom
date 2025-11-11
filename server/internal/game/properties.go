package game

var Config = ConfigProperty{
	Width:     256,
	Height:    256,
	TickDelay: 500 * 1000,
}

const (
	None  TileKey = -1
	Water TileKey = 0
	Land  TileKey = 1
	Dirt  TileKey = 2
	Rock  TileKey = 3
)

const (
	Tree       ObjectKey = 1
	Decoration ObjectKey = 2
)

const (
	Worker  UnitKey = "worker"
	Warrior UnitKey = "warrior"
	Ranger  UnitKey = "ranger"
	Spawner UnitKey = "spawner"
)

const (
	Idle StateKey = "idle"
)

var TileProperties = TilePropertyMap{
	Water: TileProperty{
		IsLand:     false,
		IsWater:    true,
		IsPassable: true,
	},
	Land: TileProperty{
		IsLand:     true,
		IsWater:    false,
		IsPassable: true,
	},
	Dirt: TileProperty{
		IsLand:     true,
		IsWater:    false,
		IsPassable: true,
	},
	Rock: TileProperty{
		IsLand:     true,
		IsWater:    false,
		IsPassable: false,
	},
}

var ObjectProperties = ObjectPropertyMap{
	Tree: ObjectProperty{
		IsSelectable: true,
		IsTargetable: true,
	},
	Decoration: ObjectProperty{
		IsSelectable: false,
		IsTargetable: false,
	},
}

var UnitProperties = UnitPropertyMap{
	Worker: UnitProperty{
		IsBuilding:    false,
		ProdutionTime: 0,
		MaxHP:         0,
		Speed:         0,
	},
	Warrior: UnitProperty{
		IsBuilding:    false,
		ProdutionTime: 0,
		MaxHP:         0,
		Speed:         0,
	},
	Ranger: UnitProperty{
		IsBuilding:    false,
		ProdutionTime: 0,
		MaxHP:         0,
		Speed:         0,
	},
	Spawner: UnitProperty{
		IsBuilding:    true,
		ProdutionTime: 0,
		MaxHP:         0,
		Speed:         0,
	},
}

var StateProperties = StatePropertyMap{
	Idle: StateProperty{
		IsInterruptable: true,
		IsFinite:        false,
		Duration:        5,
	},
}
