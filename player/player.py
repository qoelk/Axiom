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
        entities_dict = data["entities"]
        self.entities = [
            Entity(e["x"], e["y"], e["facing"]) for e in entities_dict.values()
        ]


class EntityRenderer:
    def __init__(self, entity, screen, camera):
        self.entity = entity
        self.screen = screen
        self.camera = camera

    def draw(self):
        screen_x, screen_y = self.camera.tile_to_screen(self.entity.x, self.entity.y)
        size = max(6, min(20, self.camera.tile_size_px * 0.35))
        base_points = [(size, 0), (-size * 0.4, -size * 0.6), (-size * 0.4, size * 0.6)]
        cos_a, sin_a = math.cos(self.entity.facing), math.sin(self.entity.facing)
        rotated = []
        for px, py in base_points:
            rx = px * cos_a - py * sin_a
            ry = px * sin_a + py * cos_a
            rotated.append((screen_x + rx, screen_y + ry))
        pygame.draw.polygon(self.screen, RED, rotated)
        pygame.draw.polygon(self.screen, (255, 220, 220), rotated, width=1)


# ----------------------------
# Entity & Camera (unchanged from previous refactored version)
# ----------------------------


class Entity:
    def __init__(self, x, y, facing=0.0):
        self.x = x
        self.y = y
        self.facing = facing


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


def create_random_entities(game_map, count=15):
    land_tiles = game_map.get_land_positions()
    if not land_tiles:
        return []
    entities = []
    for _ in range(count):
        tx, ty = random.choice(land_tiles)
        x = tx + random.uniform(0.2, 0.8)
        y = ty + random.uniform(0.2, 0.8)
        facing = random.uniform(0, 2 * math.pi)
        entities.append(Entity(x, y, facing))
    return entities


# ----------------------------
# Main Game Loop
# ----------------------------
class MapRenderer:
    TEXTURE_SIZE = 16
    WATER_FRAMES = 4  # Number of animation frames for water

    def __init__(self, game_map, screen, camera):
        self.map = game_map
        self.screen = screen
        self.camera = camera

        # Generate base textures once
        self.land_tex = self._create_land_texture()
        self.water_tex_frames = [
            self._create_water_texture(frame=i) for i in range(self.WATER_FRAMES)
        ]
        self.frame_counter = 0

    def _create_land_texture(self):
        tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE))
        # 8-bit dithered green
        base_color = (34, 177, 76)
        dark_color = (20, 120, 50)
        for y in range(self.TEXTURE_SIZE):
            for x in range(self.TEXTURE_SIZE):
                # Checkerboard dither pattern
                if (x + y) % 4 < 2:
                    tex.set_at((x, y), base_color)
                else:
                    tex.set_at((x, y), dark_color)
        return tex

    def _create_water_texture(self, frame=0):
        tex = pygame.Surface((self.TEXTURE_SIZE, self.TEXTURE_SIZE), pygame.SRCALPHA)
        base_color = (63, 72, 204)
        light_color = (100, 150, 255)

        # Create a subtle wave-like pattern that shifts with 'frame'
        for y in range(self.TEXTURE_SIZE):
            for x in range(self.TEXTURE_SIZE):
                # Offset based on frame to simulate motion
                offset = (x + frame * 2) % 4
                wave = (y + offset) % 4
                if wave < 2:
                    tex.set_at((x, y), base_color)
                else:
                    tex.set_at((x, y), light_color)
        return tex

    def update(self):
        """Call once per frame to advance water animation."""
        self.frame_counter = (self.frame_counter + 1) % (self.WATER_FRAMES * 8)

    def draw(self):
        screen = self.screen
        camera = self.camera
        game_map = self.map

        start_x = int(camera.tile_x - camera.width_tiles / 2 - 1)
        end_x = int(camera.tile_x + camera.width_tiles / 2 + 2)
        start_y = int(camera.tile_y - camera.height_tiles / 2 - 1)
        end_y = int(camera.tile_y + camera.height_tiles / 2 + 2)

        # Choose current water frame (cycle slowly)
        current_water_frame = (self.frame_counter // 8) % self.WATER_FRAMES

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile = game_map.get_tile(x, y)
                if tile is None:
                    continue

                if tile == 1:
                    base_tex = self.land_tex
                else:
                    base_tex = self.water_tex_frames[current_water_frame]

                screen_x, screen_y = camera.tile_to_screen(x, y)

                # Optionally: cache scaled textures if camera zoom is fixed
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
    def __init__(self, screen, camera, entity_count, font_size=24):
        self.screen = screen
        self.camera = camera
        self.entity_count = entity_count
        self.font = pygame.font.SysFont(None, font_size)

    def draw(self):
        zoom = self.camera.tile_size_px / 32.0
        info = [
            f"Entities: {self.entity_count}",
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
        entities = self.game_state.entities
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("RTS - 8-bit Style")
        self.clock = pygame.time.Clock()

        # Core game objects
        self.camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
        self.camera.tile_x = self.game_map.width / 2
        self.camera.tile_y = self.game_map.height / 2

        # Renderers
        self.map_renderer = MapRenderer(self.game_map, self.screen, self.camera)
        self.entity_renderers = [
            EntityRenderer(e, self.screen, self.camera) for e in entities
        ]
        self.ui_renderer = UIRenderer(self.screen, self.camera, len(entities))
        self.last_entity_update = pygame.time.get_ticks()
        self.entity_update_interval = 200  #

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
                return True  # quit
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

        # Pan with mouse
        if self.dragging:
            rel = pygame.mouse.get_rel()
            self.camera.move(
                -rel[0] / self.camera.tile_size_px, -rel[1] / self.camera.tile_size_px
            )
        else:
            pygame.mouse.get_rel()

        self.camera.clamp_to_map(self.game_state.map)
        return False

    def _rebuild_entity_renderers(self):
        self.entity_renderers = [
            EntityRenderer(e, self.screen, self.camera)
            for e in self.game_state.entities
        ]

    def update_entities_if_needed(self):
        now = pygame.time.get_ticks()
        if now - self.last_entity_update >= self.entity_update_interval:
            self.game_state.update_entities()
            self._rebuild_entity_renderers()
            self.last_entity_update = now

    def render(self):
        self.screen.fill(BLACK)

        self.map_renderer.update()
        self.map_renderer.draw()
        for renderer in self.entity_renderers:
            renderer.draw()
        self.ui_renderer.draw()

        pygame.display.flip()

    def run(self):
        running = True
        while running:
            if self.handle_input():
                running = False

            self.update_entities_if_needed()  # <-- Poll every 200ms
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
