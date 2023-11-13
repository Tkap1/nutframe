import asyncio
import json
from websockets.server import serve

def str_to_int(text):
	try:
		return int(text)
	except ValueError:
		return None

async def echo(websocket):
	async for message in websocket:
		message = message.strip()
		words = message.split(":")
		if len(words) != 3: continue
		mode = str_to_int(words[0].strip())
		if mode == None: continue
		if mode < 0 or mode > 2: continue
		name = words[1].strip()
		score = str_to_int(words[2].strip())
		if not score: continue
		add_or_update_entry(mode, name, score)
		data = read_leaderboard()
		output = ""
		for key, value in data[mode].items():
			output += f"{key}:{value}\n"
		await websocket.send(output)


def read_leaderboard():
	data = {}
	try:
		with open("leaderboard.json", "r") as f:
			data = json.load(f)
	except FileNotFoundError:
		pass
	return data

def add_or_update_entry(mode, name, score):
	data = read_leaderboard()

	print(f"{name}: {score} for mode {mode}")
	data[mode][name] = max(data[mode].get(name, 1), score)

	with open("leaderboard.json", "w") as f:
		json.dump(data, f)


async def main():
	async with serve(echo, "0.0.0.0", 9416):
		await asyncio.Future()  # run forever


asyncio.run(main())