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
BLUE = (0, 100, 200)  # Water
GREEN = (0, 200, 100)  # Land
BLACK = (20, 20, 20)
WHITE = (240, 240, 240)
RED = (220, 60, 60)  # Entity color
GRID_LINE = (40, 40, 40)
UI_COLOR = (220, 220, 220)

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
# Map Class
# ----------------------------


class Map:
    def __init__(self, layout_str=MAP_LAYOUT, width=16, height=16):
        self.width = width
        self.height = height
        clean_layout = "".join(layout_str.split())
        self.tiles = [int(ch) for ch in clean_layout]
        assert len(self.tiles) == width * height, "Map layout size mismatch"

    def get_tile(self, x, y):
        """Return tile value (0=water, 1=land) or None if out of bounds."""
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.tiles[y * self.width + x]
        return None

    def get_land_positions(self):
        """Return list of (x, y) tile coordinates where tile == 1 (land)."""
        return [
            (x, y)
            for y in range(self.height)
            for x in range(self.width)
            if self.get_tile(x, y) == 1
        ]


# ----------------------------
# Entity Class
# ----------------------------


class Entity:
    def __init__(self, x, y, facing=0.0):
        self.x = x  # float (sub-tile precision)
        self.y = y
        self.facing = facing  # radians

    def draw(self, screen, camera):
        screen_x, screen_y = camera.tile_to_screen(self.x, self.y)
        size = max(6, min(20, camera.tile_size_px * 0.35))

        # Triangle: points relative to (0,0), pointing right
        base_points = [(size, 0), (-size * 0.4, -size * 0.6), (-size * 0.4, size * 0.6)]

        cos_a = math.cos(self.facing)
        sin_a = math.sin(self.facing)
        rotated_points = []
        for px, py in base_points:
            rx = px * cos_a - py * sin_a
            ry = px * sin_a + py * cos_a
            rotated_points.append((screen_x + rx, screen_y + ry))

        pygame.draw.polygon(screen, RED, rotated_points)
        pygame.draw.polygon(screen, (255, 220, 220), rotated_points, width=1)


# ----------------------------
# Camera Class
# ----------------------------


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
    """Create `count` entities on random land tiles with random offsets and directions."""
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
    pygame.display.set_caption("RTS - Directional Entities")
    clock = pygame.time.Clock()

    game_map = Map()
    camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
    camera.tile_x = game_map.width / 2
    camera.tile_y = game_map.height / 2

    entities = create_random_entities(game_map, count=15)

    dragging = False
    running = True

    while running:
        # Handle events
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

        # Keyboard panning
        keys = pygame.key.get_pressed()
        pan_speed = 0.5
        if keys[pygame.K_LEFT] or keys[pygame.K_a]:
            camera.move(-pan_speed, 0)
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
            camera.move(pan_speed, 0)
        if keys[pygame.K_UP] or keys[pygame.K_w]:
            camera.move(0, -pan_speed)
        if keys[pygame.K_DOWN] or keys[pygame.K_s]:
            camera.move(0, pan_speed)

        # Mouse dragging
        if dragging:
            rel_x, rel_y = pygame.mouse.get_rel()
            camera.move(-rel_x / camera.tile_size_px, -rel_y / camera.tile_size_px)
        else:
            pygame.mouse.get_rel()  # Reset relative motion

        # Keep camera within map bounds
        camera.clamp_to_map(game_map)

        # Render
        screen.fill(BLACK)

        # Render tiles
        start_x = int(camera.tile_x - camera.width_tiles / 2 - 1)
        end_x = int(camera.tile_x + camera.width_tiles / 2 + 2)
        start_y = int(camera.tile_y - camera.height_tiles / 2 - 1)
        end_y = int(camera.tile_y + camera.height_tiles / 2 + 2)

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile = game_map.get_tile(x, y)
                if tile is None:
                    continue
                color = GREEN if tile == 1 else BLUE
                screen_x, screen_y = camera.tile_to_screen(x, y)
                rect = pygame.Rect(
                    screen_x, screen_y, camera.tile_size_px, camera.tile_size_px
                )
                pygame.draw.rect(screen, color, rect)
                pygame.draw.rect(screen, GRID_LINE, rect, width=1)

        # Render entities
        for entity in entities:
            entity.draw(screen, camera)

        # UI overlay
        font = pygame.font.SysFont(None, 24)
        zoom_level = camera.tile_size_px / 32.0
        info_lines = [
            f"Entities: {len(entities)}",
            f"Zoom: {zoom_level:.2f}x",
            "Use WASD/arrow keys, mouse wheel, or drag to navigate",
        ]
        for i, line in enumerate(info_lines):
            text_surf = font.render(line, True, UI_COLOR)
            screen.blit(text_surf, (10, 10 + i * 25))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


# ----------------------------
# Entry Point
# ----------------------------

if __name__ == "__main__":
    main()
