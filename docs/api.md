# Draft API

create_game() game_id

start_game(game_id)

join_game(game_id, player_id) player_token

fetch_metadata(player_token) game_metadata

fetch_rulebook(game_id) game_rules

fetch_map(game_id, player_token) game_data

fetch_state(game_id, player_token) game_state

submit_command(game_id, player_token, command_data)
