# group-2-project
Flappy bird.

Executable expects the resource folder to be located in the same directory as
executable. So when you build, copy and paste the rsc folder into your build
directory.

## Online scoreboard
After playing a round, the game asks the player if they would like to submit
their score to a central scoreboard server.

This server is currently hosted at flappy.serwe.dev

The server's source lives in server/ and
can be built on its own with `cmake -S server -B build-server`.

```
./scoreboard_server [port (default 8080)] [score-file (default scores.txt)]
```
