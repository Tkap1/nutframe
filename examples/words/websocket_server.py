import asyncio
import json
from websockets.server import serve

async def echo(websocket):
	async for message in websocket:
		message = message.strip()
		colon_index = message.find(":")
		if colon_index != -1:
			name = message[0: colon_index].strip()
			if len(name) <= 16:
				score_str = message[colon_index + 1:].strip()
				try:
					score = int(score_str)
					if score > 0:
						add_or_update_entry(name, score)

						data = read_leaderboard()
						output = ""
						for key, value in data.items():
							output += f"{key}:{value}\n"
						await websocket.send(output)

				except ValueError:
					pass


def read_leaderboard():
	data = {}
	try:
		with open("leaderboard.json", "r") as f:
			data = json.load(f)
	except FileNotFoundError:
		pass
	return data

def add_or_update_entry(name, score):
	data = read_leaderboard()

	data[name] = max(data.get(name, 1), score)

	with open("leaderboard.json", "w") as f:
		json.dump(data, f)


async def main():
	async with serve(echo, "0.0.0.0", 9416):
		await asyncio.Future()  # run forever


asyncio.run(main())