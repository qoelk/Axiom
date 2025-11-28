package generators

import (
	"core.axiom/internal/players"
	"core.axiom/internal/tilemap"
	"core.axiom/internal/units"
	"github.com/google/uuid"
)

var defaultUnits []units.UnitKey = []units.UnitKey{
	units.CASTLE,
	units.PEASANT,
	units.PEASANT,
	units.PEASANT,
}

func GenerateUnits(tileMap tilemap.TileMap, players []players.Player) map[uuid.UUID]*units.Unit {
	res := make(map[uuid.UUID]*units.Unit)
	for _, p := range players {
		us := generatePlayerUnits(p, 0, 0)
		for _, u := range us {
			res[u.ID] = u
		}
	}
	return res
}

func generatePlayerUnits(player players.Player, x0 int64, y0 int64) []*units.Unit {
	res := make([]*units.Unit, 0)

	for _, uk := range defaultUnits {
		defaultUnit := units.UnitsBook[uk]
		u := units.Unit{
			ID:       uuid.New(),
			ObjectID: uuid.New(),
			Type:     uk,
			Owner:    player.ID,
			HP:       defaultUnit.MaxHP,
		}
		res = append(res, &u)
	}
	return res
}
