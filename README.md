# arduino

## Try compiling
`arduino-cli compile --fqbn arduino:avr:uno /workspaces/arduino`

## Upload to a board:

Plug in your Arduino, find its port with arduino-cli board list, then run:
`arduino-cli upload -p <your-port> --fqbn arduino:avr:uno /workspaces/arduino`

## Find your board port
1) run command `arduino-cli board list`
2) update the arg in "Arduino: Upload" in tasks.json with the correct port
