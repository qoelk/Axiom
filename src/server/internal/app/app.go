package app

import (
	"net/http"

	"axiom/internal/game"
	"axiom/internal/game/config"
	"axiom/internal/game/tilemap"

	"github.com/gin-gonic/gin"
)

type App struct {
	game   *game.Game
	server *gin.Engine
}

func NewApp(mapPath string, cfgPath string) (*App, error) {
	m, err := tilemap.FromFile(mapPath)
	if err != nil {
		return nil, err
	}

	cfg, err := config.FromFile(cfgPath)
	if err != nil {
		return nil, err
	}
	g, err := game.NewGame(m, cfg)
	if err != nil {
		return nil, err
	}

	server := gin.Default()
	app := App{
		game:   g,
		server: server,
	}

	server.GET("/ping", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"message": "pong",
		})
	})

	server.GET("/map", func(c *gin.Context) {
		c.JSON(http.StatusOK, app.game.Map())
	})
	return &app, nil
}

func (a *App) Run(addr string) error {
	return a.server.Run(addr)
}
