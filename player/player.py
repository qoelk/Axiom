import requests
import pygame
import math

# ----------------------------
# Constants
# ----------------------------

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

# Colors
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


class S_Object:
    __slots__ = ("x", "y", "size")

    def __init__(self, x, y, size):
        self.x = x
        self.y = y
        self.size = size


class S_Unit:
    __slots__ = ("x", "y", "size", "facing", "velocity", "owner")

    def __init__(self, x, y, size, facing, velocity, owner):
        self.x = x
        self.y = y
        self.size = size
        self.facing = facing
        self.velocity = velocity
        self.owner = owner


class GameState:
    def __init__(self):
        resp = requests.get("http://localhost:8080/")
        data = resp.json()
        self.map = Map(
            data["map"]["width"], data["map"]["height"], data["map"]["tiles"]
        )
        self.objects = []
        self.units = []
        self._update_entities_from_data(data)

    def update_entities(self):
        try:
            resp = requests.get("http://localhost:8080/")
            data = resp.json()
            self._update_entities_from_data(data)
        except Exception as e:
            print(f"[WARNING] Failed to update entities: {e}")

    def _update_entities_from_data(self, data):
        # Update objects
        new_objs = list(data["objects"].values())
        if len(new_objs) == len(self.objects):
            for obj, e in zip(self.objects, new_objs):
                obj.x, obj.y, obj.size = e["x"], e["y"], e["size"]
        else:
            self.objects = [S_Object(e["x"], e["y"], e["size"]) for e in new_objs]

        # Update units
        new_units = list(data["units"].values())
        if len(new_units) == len(self.units):
            for unit, e in zip(self.units, new_units):
                unit.x = e["x"]
                unit.y = e["y"]
                unit.size = e["size"]
                unit.facing = e["facing"]
                unit.velocity = e.get("velocity", 0)
                unit.owner = e["owner"]
        else:
            self.units = [
                S_Unit(
                    e["x"],
                    e["y"],
                    e["size"],
                    e["facing"],
                    e.get("velocity", 0),
                    e["owner"],
                )
                for e in new_units
            ]


# ----------------------------
# Rendering Functions (Stateless)
# ----------------------------


def draw_object(s_obj, screen, camera):
    BROWN = (139, 69, 19)
    side = int(camera.tile_size_px * s_obj.size)
    sx, sy = camera.tile_to_screen(s_obj.x, s_obj.y)
    rect = (sx - side // 2, sy - side // 2, side, side)
    pygame.draw.rect(screen, BROWN, rect)


def draw_unit(s_unit, screen, camera):
    if s_unit.owner == 1:
        color = (160, 20, 20)
    elif s_unit.owner == 2:
        color = (20, 20, 160)
    else:
        color = (100, 100, 100)

    px_size = int(camera.tile_size_px * s_unit.size)
    sx, sy = camera.tile_to_screen(s_unit.x, s_unit.y)
    rect = (sx - px_size // 2, sy - px_size // 2, px_size, px_size)
    pygame.draw.rect(screen, color, rect)

    # Facing triangle
    tri_size = max(4, int(px_size * 0.6))
    h = tri_size // 2
    dx = math.cos(s_unit.facing) * h
    dy = math.sin(s_unit.facing) * h
    tip = (sx + dx, sy + dy)
    left = (sx - dy, sy + dx)
    right = (sx + dy, sy - dx)
    pygame.draw.polygon(screen, (0, 0, 0), [tip, left, right])


class Camera:
    def __init__(self, screen_width, screen_height):
        self.screen_width = screen_width
        self.screen_height = screen_height
        self.tile_size_px = 32.0
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


class MapRenderer:
    TILE_COLORS = {
        1: (34, 177, 76),  # land
        2: (172, 200, 12),  # dirt
        3: (11, 11, 11),  # rock
        0: (63, 72, 204),  # water (default)
    }

    def __init__(self, game_map, screen, camera):
        self.map = game_map
        self.screen = screen
        self.camera = camera
        self._texture_cache = {}
        self._last_tile_size = -1

    def draw(self):
        tile_size = int(self.camera.tile_size_px)
        if tile_size != self._last_tile_size:
            self._texture_cache.clear()
            self._last_tile_size = tile_size

        start_x = int(self.camera.tile_x - self.camera.width_tiles / 2 - 1)
        end_x = int(self.camera.tile_x + self.camera.width_tiles / 2 + 2)
        start_y = int(self.camera.tile_y - self.camera.height_tiles / 2 - 1)
        end_y = int(self.camera.tile_y + self.camera.height_tiles / 2 + 2)

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile_val = self.map.get_tile(x, y)
                if tile_val is None:
                    continue

                color = self.TILE_COLORS.get(tile_val, (0, 0, 0))
                cache_key = (color, tile_size)
                if cache_key not in self._texture_cache:
                    surf = pygame.Surface((tile_size, tile_size))
                    surf.fill(color)
                    self._texture_cache[cache_key] = surf

                screen_x, screen_y = self.camera.tile_to_screen(x, y)
                self.screen.blit(self._texture_cache[cache_key], (screen_x, screen_y))
                pygame.draw.rect(
                    self.screen,
                    GRID_LINE,
                    (screen_x, screen_y, tile_size, tile_size),
                    1,
                )


class UIRenderer:
    def __init__(self, screen, camera, font_size=24):
        self.screen = screen
        self.camera = camera
        self.font = pygame.font.SysFont(None, font_size)

    def draw(self):
        zoom = self.camera.tile_size_px / 32.0
        texts = [
            f"Zoom: {zoom:.2f}x",
            "WASD/arrows, mouse wheel, or drag to navigate",
        ]
        for i, text in enumerate(texts):
            img = self.font.render(text, True, UI_COLOR)
            self.screen.blit(img, (10, 10 + i * 25))


class Game:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("Axiom")
        self.clock = pygame.time.Clock()

        self.game_state = GameState()
        self.camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
        self.camera.tile_x = self.game_state.map.width / 2
        self.camera.tile_y = self.game_state.map.height / 2

        self.map_renderer = MapRenderer(self.game_state.map, self.screen, self.camera)
        self.ui_renderer = UIRenderer(self.screen, self.camera)

        self.last_entity_update = pygame.time.get_ticks()
        self.entity_update_interval = 200  # ms
        self.dragging = False

    def handle_input(self, dt):
        speed = 10.0 * dt  # tiles per second
        keys = pygame.key.get_pressed()
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
                -rel[0] / self.camera.tile_size_px,
                -rel[1] / self.camera.tile_size_px,
            )
        else:
            pygame.mouse.get_rel()  # consume relative motion

        self.camera.clamp_to_map(self.game_state.map)
        return False

    def update_entities_if_needed(self):
        now = pygame.time.get_ticks()
        if now - self.last_entity_update >= self.entity_update_interval:
            self.game_state.update_entities()
            self.last_entity_update = now

    def render(self):
        self.screen.fill(BLACK)
        self.map_renderer.draw()

        for obj in self.game_state.objects:
            draw_object(obj, self.screen, self.camera)

        for unit in self.game_state.units:
            draw_unit(unit, self.screen, self.camera)

        self.ui_renderer.draw()
        pygame.display.flip()

    def run(self):
        running = True
        last_time = pygame.time.get_ticks()
        while running:
            current_time = pygame.time.get_ticks()
            dt = (current_time - last_time) / 1000.0
            last_time = current_time

            if self.handle_input(dt):
                running = False

            self.update_entities_if_needed()
            self.render()
            self.clock.tick(FPS)

    def quit(self):
        pygame.quit()


def main():
    try:
        game = Game()
        game.run()
    finally:
        pygame.quit()


if __name__ == "__main__":
    main()
