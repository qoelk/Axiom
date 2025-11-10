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

# Map layout (16x16)
MAP_LAYOUT = """
0000000000000000
0000001111000000
0000111111110000
0001111111111000
0011111111111100
0011111111111100
0111111111111110
0111111111111110
0111111111111110
0011111111111100
0011111111111100
0001111111111000
0000111111110000
0000001111000000
0000000000000000
0000000000000000
"""


# ----------------------------
# Texture Generation
# ----------------------------


def create_land_texture(size=16):
    surf = pygame.Surface((size, size))
    base = (34, 139, 34)
    highlight = (50, 205, 50)
    shadow = (0, 100, 0)
    surf.fill(base)
    for _ in range(size * 2):
        x, y = random.randint(0, size - 1), random.randint(0, size - 1)
        surf.set_at((x, y), highlight if random.random() < 0.5 else shadow)
    return surf


def create_water_texture(size=16):
    surf = pygame.Surface((size, size))
    base = (0, 80, 160)
    highlight = (100, 180, 255)
    shadow = (0, 40, 100)
    surf.fill(base)
    for _ in range(size * 3):
        x, y = random.randint(0, size - 1), random.randint(0, size - 1)
        if random.random() < 0.7:
            surf.set_at((x, y), highlight)
        else:
            surf.set_at((x, y), shadow)
    return surf


# ----------------------------
# Map Class (now holds textures)
# ----------------------------


class Map:
    TEXTURE_SIZE = 16

    def __init__(self, layout_str=MAP_LAYOUT, width=16, height=16):
        self.width = width
        self.height = height
        clean_layout = "".join(layout_str.split())
        self.tiles = [int(ch) for ch in clean_layout]
        assert len(self.tiles) == width * height, "Map layout size mismatch"

        # Generate textures once
        self.land_tex = create_land_texture(self.TEXTURE_SIZE)
        self.water_tex = create_water_texture(self.TEXTURE_SIZE)

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
    def __init__(self, map: Map, entities):
        self.map = map
        self.entities = entities


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


def main():
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("RTS - 8-bit Style")
    clock = pygame.time.Clock()

    game_map = Map()
    camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
    camera.tile_x = game_map.width / 2
    camera.tile_y = game_map.height / 2

    # Pure game logic entities
    entities = create_random_entities(game_map, count=15)

    # Game state holds only data
    game_state = GameState(game_map, entities)

    # Renderers are separate; created once
    renderers = [EntityRenderer(entity, screen, camera) for entity in entities]

    dragging = False
    running = True

    while running:
        # --- Input Handling (same as before) ---
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEWHEEL:
                if event.y > 0:
                    camera.zoom_in()
                else:
                    camera.zoom_out()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    dragging = True
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    dragging = False

        keys = pygame.key.get_pressed()
        speed = 0.5
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            camera.move(-speed, 0)
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            camera.move(speed, 0)
        if keys[pygame.K_UP] or keys[pygame.K_w]:
            camera.move(0, -speed)
        if keys[pygame.K_DOWN] or keys[pygame.K_s]:
            camera.move(0, speed)

        if dragging:
            rel = pygame.mouse.get_rel()
            camera.move(-rel[0] / camera.tile_size_px, -rel[1] / camera.tile_size_px)
        else:
            pygame.mouse.get_rel()

        camera.clamp_to_map(game_state.map)

        # --- Rendering ---
        screen.fill(BLACK)

        # Render tiles
        start_x = int(camera.tile_x - camera.width_tiles / 2 - 1)
        end_x = int(camera.tile_x + camera.width_tiles / 2 + 2)
        start_y = int(camera.tile_y - camera.height_tiles / 2 - 1)
        end_y = int(camera.tile_y + camera.height_tiles / 2 + 2)

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile = game_state.map.get_tile(x, y)
                if tile is None:
                    continue

                base_tex = (
                    game_state.map.land_tex if tile == 1 else game_state.map.water_tex
                )
                screen_x, screen_y = camera.tile_to_screen(x, y)
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

        # Render entities using pre-created renderers
        for renderer in renderers:
            renderer.draw()

        # UI
        font = pygame.font.SysFont(None, 24)
        zoom = camera.tile_size_px / 32.0
        info = [
            f"Entities: {len(game_state.entities)}",
            f"Zoom: {zoom:.2f}x",
            "WASD/arrows, mouse wheel, or drag to navigate",
        ]
        for i, text in enumerate(info):
            screen.blit(font.render(text, True, UI_COLOR), (10, 10 + i * 25))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == "__main__":
    main()
