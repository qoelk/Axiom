import pygame
import math
import random

# Initialize Pygame
pygame.init()

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
FPS = 60

BLUE = (0, 100, 200)  # water
GREEN = (0, 200, 100)  # land
BLACK = (20, 20, 20)
WHITE = (240, 240, 240)
RED = (220, 60, 60)  # entity color

# Realistic map (16x16)
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


class Map:
    def __init__(self, layout_str=MAP_LAYOUT, width=16, height=16):
        self.width = width
        self.height = height
        clean = "".join(layout_str.split())
        self.tiles = [int(ch) for ch in clean]
        assert len(self.tiles) == width * height

    def get_tile(self, x, y):
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.tiles[y * self.width + x]
        return None

    def get_land_positions(self):
        """Return list of (x, y) tile coordinates where tile == 1 (land)"""
        land_tiles = []
        for y in range(self.height):
            for x in range(self.width):
                if self.get_tile(x, y) == 1:
                    land_tiles.append((x, y))
        return land_tiles


class Entity:
    def __init__(self, x, y, facing=0.0):
        self.x = x  # tile coordinate (float, e.g., 7.3)
        self.y = y  # tile coordinate (float)
        self.facing = facing  # radians

    def draw(self, screen, camera):
        # Center of the entity in screen space
        screen_x, screen_y = camera.tile_to_screen(self.x, self.y)

        size = max(6, min(20, camera.tile_size_px * 0.35))  # adaptive size

        # Triangle pointing right by default
        points = [
            (size, 0),  # tip
            (-size * 0.4, -size * 0.6),  # back left
            (-size * 0.4, size * 0.6),  # back right
        ]

        # Rotate points
        cos_a = math.cos(self.facing)
        sin_a = math.sin(self.facing)
        screen_points = []
        for px, py in points:
            rx = px * cos_a - py * sin_a
            ry = px * sin_a + py * cos_a
            screen_points.append((screen_x + rx, screen_y + ry))

        pygame.draw.polygon(screen, RED, screen_points)
        pygame.draw.polygon(screen, (255, 220, 220), screen_points, 1)


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
        screen_x = (tile_x - (self.tile_x - self.width_tiles / 2)) * self.tile_size_px
        screen_y = (tile_y - (self.tile_y - self.height_tiles / 2)) * self.tile_size_px
        return screen_x, screen_y


def create_random_entities(game_map, count=12):
    """Create `count` entities on random land tiles with random offsets and directions."""
    land_tiles = game_map.get_land_positions()
    if not land_tiles:
        return []

    entities = []
    for _ in range(count):
        # Pick a random land tile
        tx, ty = random.choice(land_tiles)
        # Add random sub-tile offset (so they're not all aligned to corners)
        x = tx + random.uniform(0.2, 0.8)
        y = ty + random.uniform(0.2, 0.8)
        # Random direction (0 to 2π)
        facing = random.uniform(0, 2 * math.pi)
        entities.append(Entity(x, y, facing))
    return entities


def main():
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("RTS - Multiple Directional Entities")
    clock = pygame.time.Clock()

    game_map = Map()
    camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT)
    camera.tile_x = game_map.width / 2
    camera.tile_y = game_map.height / 2

    # Create multiple entities
    entities = create_random_entities(game_map, count=15)

    dragging = False
    running = True

    while running:
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

        # Camera movement
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
            dx = rel[0] / camera.tile_size_px
            dy = rel[1] / camera.tile_size_px
            camera.move(-dx, -dy)
        else:
            pygame.mouse.get_rel()

        # Keep camera in map bounds
        half_w = camera.width_tiles / 2
        half_h = camera.height_tiles / 2
        camera.tile_x = max(half_w, min(game_map.width - half_w, camera.tile_x))
        camera.tile_y = max(half_h, min(game_map.height - half_h, camera.tile_y))

        # Render
        screen.fill(BLACK)

        # Render tiles
        start_x = int(camera.tile_x - camera.width_tiles / 2 - 1)
        end_x = int(camera.tile_x + camera.width_tiles / 2 + 2)
        start_y = int(camera.tile_y - camera.height_tiles / 2 - 1)
        end_y = int(camera.tile_y + camera.height_tiles / 2 + 2)

        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                tile_val = game_map.get_tile(x, y)
                if tile_val is None:
                    continue
                color = GREEN if tile_val == 1 else BLUE
                sx, sy = camera.tile_to_screen(x, y)
                rect = pygame.Rect(sx, sy, camera.tile_size_px, camera.tile_size_px)
                pygame.draw.rect(screen, color, rect)
                pygame.draw.rect(screen, (40, 40, 40), rect, 1)

        # Render all entities
        for entity in entities:
            entity.draw(screen, camera)

        # UI
        font = pygame.font.SysFont(None, 24)
        zoom = camera.tile_size_px / 32.0
        info = [
            f"Entities: {len(entities)}",
            f"Zoom: {zoom:.2f}x",
            f"Use WASD/arrows, mouse wheel, or drag to navigate",
        ]
        for i, text in enumerate(info):
            screen.blit(font.render(text, True, (220, 220, 220)), (10, 10 + i * 25))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == "__main__":
    main()
