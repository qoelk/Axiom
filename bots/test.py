import requests
import time
import random
import sys

BASE_URL = "http://localhost:8080"

def main():
    # Optional: fetch map (not used for damage, but kept for consistency)
    try:
        map_resp = requests.get(f"{BASE_URL}/map")
        map_resp.raise_for_status()
        map_data = map_resp.json()["map"]
        width = map_data.get("width", 100)
        height = map_data.get("height", 100)
        print(f"Map size: {width} x {height}")
    except Exception as e:
        print(f"Warning: failed to get map ({e}). Using default 100x100.")
        width = height = 100

    while True:
        try:
            # Fetch latest state every iteration
            state_resp = requests.get(f"{BASE_URL}/state")
            state_resp.raise_for_status()
            state_data = state_resp.json()
            units = state_data.get("units", {})

            unit_ids = list(units.keys())

            if not unit_ids:
                print("No units found.")
                time.sleep(1)
                continue

            # 1. Move each unit to a random point
            for unit_id in unit_ids:
                x = random.uniform(0, width)
                y = random.uniform(0, height)
                move_payload = {"unit_id": unit_id, "x": x, "y": y}
                try:
                    resp = requests.post(f"{BASE_URL}/unit/move", json=move_payload, timeout=2)
                    if resp.status_code != 204:
                        print(f"Move failed for {unit_id}: {resp.status_code}")
                except Exception as e:
                    print(f"Move error for {unit_id}: {e}")

            # 2. Each unit damages a random *other* unit (if possible)
            if len(unit_ids) >= 2:
                for unit_id in unit_ids:
                    # Exclude self from targets
                    possible_targets = [uid for uid in unit_ids if uid != unit_id]
                    if not possible_targets:
                        continue
                    target_id = random.choice(possible_targets)
                    damage_payload = {"unit_id": unit_id, "target_id": target_id}
                    try:
                        resp = requests.post(f"{BASE_URL}/unit/damage", json=damage_payload, timeout=2)
                        if resp.status_code == 204:
                            print(f"Unit {unit_id} damaged {target_id}")
                        else:
                            print(f"Damage failed: {resp.status_code} - {resp.text}")
                    except Exception as e:
                        print(f"Damage error ({unit_id} → {target_id}): {e}")
            else:
                print("Not enough units to perform damage.")

            time.sleep(1)

        except KeyboardInterrupt:
            print("\nStopping script...")
            break
        except Exception as e:
            print(f"Unexpected error: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()