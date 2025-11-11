import requests
import pygame
import math
import random

# ----------------------------
# Constants
# ----------------------------

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Colors
RED = (220, 60, 60)  # Entity
BLACK = (20, 20, 20)
UI_COLOR = (220, 220, 220)
GRID_LINE = (40, 40, 40)


class Map:
    def __init__(self, width, height, tiles):
        self.width = width
        self.height = height
        self.tiles = tiles

    def get_tile(self, x, y):
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.tiles[y * self.width + x]
        return None

    def get_land_positions(self):
        return [
            (x, y)
            for y in range(self.height)
            for x in range(self.width)
            if self.get_tile(x, y) == 1
        ]


class GameState:
    def __init__(self):
        # Fetch full state once at startup
        resp = requests.get("http://localhost:8080/")
        data = resp.json()
        self.map = Map(
            data["map"]["width"], data["map"]["height"], data["map"]["tiles"]
        )
        self._update_entities_from_data(data)

    def update_entities(self):
        """Fetch only entities from server and update the list."""
        try:
            resp = requests.get("http://localhost:8080/")
            data = resp.json()
            self._update_entities_from_data(data)
        except Exception as e:
            print(f"[WARNING] Failed to update entities: {e}")

    def _update_entities_from_data(self, data):
        # Update objects
        self.objects = [
            S_Object(
                e["x"],
                e["y"],
                e["size"],
            )
            for e in data["objects"].values()
        ]

        # Update units
        self.units = [
            S_Unit(
                x=e["x"],
                y=e["y"],
                size=e["size"],
                facing=e["facing"],
                velocity=e["velocity"],
                owner=e["owner"],
            )
            for e in data["units"].values()
        ]


class S_ObjectRenderer:
    def __init__(self, s_object, screen, camera):
        self.s_object = s_object
        self.screen = screen
        self.camera = camera

    def draw(self):
        BROWN = (139, 69, 19)  # Brown color

        # Use the object's Size (e.g., 1.0 = full tile, 0.5 = half tile, etc.)
        side_length = int(self.camera.tile_size_px * self.s_object.size)

        # Get screen position (center of the object)
        screen_x, screen_y = self.camera.tile_to_screen(
            self.s_object.x, self.s_object.y
        )

        # Compute top-left corner so the rect is centered on (screen_x, screen_y)
        rect_x = screen_x - side_length // 2
        rect_y = screen_y - side_length // 2

        # Draw the square
        pygame.draw.rect(self.screen, BROWN, (rect_x, rect_y, side_length, side_length))


class S_UnitRenderer:
    def __init__(self, s_unit, screen, camera):
        self.s_unit = s_unit
        self.screen = screen
        self.camera = camera

    def draw(self):
        # Determine color by owner
        if self.s_unit.owner == 1:
            UNIT_COLOR = (160, 20, 20)  # Bright red
        elif self.s_unit.owner == 2:
            UNIT_COLOR = (20, 20, 160)  # Bright blue
        else:
            UNIT_COLOR = (100, 100, 100)  # Neutral gray (fallback)

        px_size = int(self.camera.tile_size_px * self.s_unit.size)
        screen_x, screen_y = self.camera.tile_to_screen(self.s_unit.x, self.s_unit.y)

        # --- Draw the base square (body of the unit) ---
        rect_x = screen_x - px_size // 2
        rect_y = screen_y - px_size // 2
        pygame.draw.rect(self.screen, UNIT_COLOR, (rect_x, rect_y, px_size, px_size))

        # --- Draw a small triangle indicating facing direction ---
        # Triangle size: 60% of unit size, but at least 4px
        tri_size = max(4, int(px_size * 0.6))
        half_tri = tri_size // 2

        # Direction vector from facing angle (radians)
        dx = math.cos(self.s_unit.facing) * half_tri
        dy = math.sin(self.s_unit.facing) * half_tri

        # Tip of the triangle (points in facing direction)
        tip = (screen_x + dx, screen_y + dy)

        # Base corners (perpendicular to facing)
        # Rotate (-dy, dx) and (dy, -dx) to get left/right of direction
        left_base = (screen_x - dy, screen_y + dx)
        right_base = (screen_x + dy, screen_y - dx)

        # Draw filled triangle
        pygame.draw.polygon(
            self.screen,
            (0, 0, 0),  # Black outline for visibility
            [tip, left_base, right_base],
        )


class S_Object:
    def __init__(self, x, y, size):
        self.x = x
        self.y = y
        self.size = size


class S_Unit:
    def __init__(self, x, y, size, facing, velocity, owner):
        # Inherit object-like properties
        self.x = x
        self.y = y
        self.size = size

        # Unit-specific properties
        self.facing = facing  # direction in radians
        self.velocity = velocity  # speed (may be 0 if stationary)
        self.owner = owner  # 1 or 2 (or other for neutral)


class Camera:
    def __init__(self, screen_width, screen_height):
        self.screen_width = screen_width
        self.screen_height = screen_height
        self.tile_size_px = 32
        self.min_tile_size = 8
        self.max_tile_size = 128
        self.tile_x = 0.0
        self.tile_y = 0.0

    @property
    def width_tiles(self):
        return self.screen_width / self.tile_size_px

    @property
    def height_tiles(self):
        return self.screen_height / self.tile_size_px

    def move(self, dx, dy):
        self.tile_x += dx
        self.tile_y += dy

    def zoom_in(self):
        self.tile_size_px = min(self.max_tile_size, self.tile_size_px * 1.2)

    def zoom_out(self):
        self.tile_size_px = max(self.min_tile_size, self.tile_size_px / 1.2)

    def tile_to_screen(self, tile_x, tile_y):
        offset_x = tile_x - (self.tile_x - self.width_tiles / 2)
        offset_y = tile_y - (self.tile_y - self.height_tiles / 2)
        return offset_x * self.tile_size_px, offset_y * self.tile_size_px

    def clamp_to_map(self, game_map):
        half_w = self.width_tiles / 2
        half_h = self.height_tiles / 2
        self.tile_x = max(half_w, min(game_map.width - half_w, self.tile_x))
        self.tile_y = max(half_h, min(game_map.height - half_h, self.tile_y))


# ----------------------------
# Entity Factory
# ----------------------------


# ----------------------------
# Main Game Loop
# ----------------------------
class MapRenderer:
    TEXTURE_SIZE = 16  # Still used for base surfaces, though we’ll scale them

    def __init__(self, game_map, screen, camera):
        self.map = game_map
        self.screen = screen
        self.camera = camera

        # Create simple solid-color textures
        self.land_tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE))
        self.land_tex.fill((34, 177, 76))  # Green for land

        self.water_tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE))
        self.water_tex.fill((63, 72, 204))  # Blue for water

        self.dirt_tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE))
        self.dirt_tex.fill((172, 200, 12))

        self.rock_tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE))
        self.rock_tex.fill((11, 11, 11))

    def draw(self):
        screen = self.screen
        camera = self.camera
        game_map = self.map

        start_x = int(camera.tile_x - camera.width_tiles / 2 - 1)
        end_x = int(camera.tile_x + camera.width_tiles / 2 + 2)
        start_y = int(camera.tile_y - camera.height_tiles / 2 - 1)
        end_y = int(camera.tile_y + camera.height_tiles / 2 + 2)

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile = game_map.get_tile(x, y)
                if tile is None:
                    continue

                # Choose texture based on tile type
                if tile == 1:
                    base_tex = self.land_tex
                elif tile == 2:
                    base_tex = self.dirt_tex
                elif tile == 3:
                    base_tex = self.rock_tex
                else:
                    base_tex = self.water_tex

                screen_x, screen_y = camera.tile_to_screen(x, y)

                # Scale to current tile size (supports zooming)
                scaled_tex = pygame.transform.scale(
                    base_tex, (camera.tile_size_px, camera.tile_size_px)
                )
                screen.blit(scaled_tex, (screen_x, screen_y))

                pygame.draw.rect(
                    screen,
                    GRID_LINE,
                    (screen_x, screen_y, camera.tile_size_px, camera.tile_size_px),
                    1,
                )


class UIRenderer:
    def __init__(self, screen, camera, font_size=24):
        self.screen = screen
        self.camera = camera
        self.font = pygame.font.SysFont(None, font_size)

    def draw(self):
        zoom = self.camera.tile_size_px / 32.0
        info = [
            f"Zoom: {zoom:.2f}x",
            "WASD/arrows, mouse wheel, or drag to navigate",
        ]
        for i, text in enumerate(info):
            rendered = self.font.render(text, True, UI_COLOR)
            self.screen.blit(rendered, (10, 10 + i * 25))


class Game:
    def __init__(self):
        self.game_state = GameState()
        self.game_map = self.game_state.map
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("Axiom")
        self.clock = pygame.time.Clock()

        # Camera setup
        self.camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
        self.camera.tile_x = self.game_map.width / 2
        self.camera.tile_y = self.game_map.height / 2

        # Renderers
        self.map_renderer = MapRenderer(self.game_map, self.screen, self.camera)
        self.object_renderers = [
            S_ObjectRenderer(e, self.screen, self.camera)
            for e in self.game_state.objects
        ]
        self.unit_renderers = [
            S_UnitRenderer(e, self.screen, self.camera) for e in self.game_state.units
        ]
        self.ui_renderer = UIRenderer(
            self.screen,
            self.camera,
        )

        # Timing
        self.last_entity_update = pygame.time.get_ticks()
        self.entity_update_interval = 200  # milliseconds (e.g., update 5x/sec)

        # Input state
        self.dragging = False

    def handle_input(self):
        """Process input and return whether to quit."""
        keys = pygame.key.get_pressed()
        speed = 0.5
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            self.camera.move(-speed, 0)
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            self.camera.move(speed, 0)
        if keys[pygame.K_UP] or keys[pygame.K_w]:
            self.camera.move(0, -speed)
        if keys[pygame.K_DOWN] or keys[pygame.K_s]:
            self.camera.move(0, speed)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return True
            elif event.type == pygame.MOUSEWHEEL:
                if event.y > 0:
                    self.camera.zoom_in()
                else:
                    self.camera.zoom_out()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    self.dragging = True
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    self.dragging = False

        if self.dragging:
            rel = pygame.mouse.get_rel()
            self.camera.move(
                -rel[0] / self.camera.tile_size_px, -rel[1] / self.camera.tile_size_px
            )
        else:
            pygame.mouse.get_rel()  # Reset relative motion

        self.camera.clamp_to_map(self.game_state.map)
        return False

    def _rebuild_entity_renderers(self):
        """Recreate renderers based on current game state."""
        self.object_renderers = [
            S_ObjectRenderer(e, self.screen, self.camera)
            for e in self.game_state.objects
        ]
        self.unit_renderers = [
            S_UnitRenderer(e, self.screen, self.camera) for e in self.game_state.units
        ]
        # Optionally update UI renderer counts if needed
        self.ui_renderer = UIRenderer(
            self.screen,
            self.camera,
        )

    def update_entities_if_needed(self):
        now = pygame.time.get_ticks()
        if now - self.last_entity_update >= self.entity_update_interval:
            self.game_state.update_entities()
            self._rebuild_entity_renderers()
            self.last_entity_update = now

    def render(self):
        self.screen.fill(BLACK)
        self.map_renderer.draw()

        # Render static objects (trees, decorations)
        for renderer in self.object_renderers:
            renderer.draw()

        # Render units (on top, usually)
        for renderer in self.unit_renderers:
            renderer.draw()

        self.ui_renderer.draw()
        pygame.display.flip()

    def run(self):
        running = True
        while running:
            if self.handle_input():
                running = False

            self.update_entities_if_needed()
            self.render()
            self.clock.tick(FPS)

    def quit(self):
        pygame.quit()


def main():
    game = Game()
    game.run()
    game.quit()


if __name__ == "__main__":
    main()
