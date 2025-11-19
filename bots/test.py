import requests
import time
import random
import sys

BASE_URL = "http://localhost:8080"

def main():
    # 1. Get map dimensions
    try:
        map_resp = requests.get(f"{BASE_URL}/map")
        map_resp.raise_for_status()
        map_data = map_resp.json()["map"]
        width = map_data.get("width")
        height = map_data.get("height")
        if width is None or height is None:
            raise ValueError("Map response missing 'width' or 'height'")
        print(f"Map size: {width} x {height}")
    except Exception as e:
        print(f"Failed to get map: {e}")
        sys.exit(1)

    while True:
        try:
            # 2. Get current state
            state_resp = requests.get(f"{BASE_URL}/state")
            state_resp.raise_for_status()
            state_data = state_resp.json()
            units = state_data.get("units", {})

            if not units:
                print("No units found in state.")
                time.sleep(1)
                continue

            # 3. For each unit, send move command to random point
            for unit_id in units.keys():
                x = random.uniform(0, width)
                y = random.uniform(0, height)
                move_payload = {
                    "unit_id": unit_id,
                    "x": x,
                    "y": y
                }
                try:
                    move_resp = requests.post(f"{BASE_URL}/unit/move", json=move_payload)
                    if move_resp.status_code == 204:
                        print(f"Moved unit {unit_id} to ({x:.2f}, {y:.2f})")
                    else:
                        print(f"Failed to move unit {unit_id}: {move_resp.status_code} {move_resp.text}")
                except Exception as move_err:
                    print(f"Error moving unit {unit_id}: {move_err}")

            # Wait before next tick
            time.sleep(1)

        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error during loop: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()