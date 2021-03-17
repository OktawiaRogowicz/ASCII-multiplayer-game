# ASCII-multiplayer-game

My first bigger project, one of three made for one of my subjects, written only with C. You can look up the requirements here: x

It took few months of work at the end of 2019, during my third semester studying Computer Science. As it was just my third semester, so just my third semester of coding ever, the code might be pretty chaotic - and I left it as it was, only planning adding comments in the future in which I will explain what would I do differently this time. However, I had fun and I think it's pretty interesting, as it explores C language greatly! 

## Looks and gameplay
![screen](https://user-images.githubusercontent.com/72928120/111546307-c4f23e00-8777-11eb-8a67-bf15196cf6f7.png)
It was made used only ASCII and **ncurses** library, also featuring **semaphores**, **mutexes** and **threads**.

### Server
Server hoards all the information about the game and displays it. It shows the actual state of the map, all the layers, players and enemies. It also shows info about players: the number of deaths or coins gathered. 

You can spawn enemies clicking "B" on keyboard and add coins clicking "C". There are three types of coins to collect: c = 1 (80% of spawning), t = 10 (15%), T = 50 (5%).

### Players and bots 
Firstly, players map is empty. While moving, player slowly remembers the parts of the maze they have already visited, but only the walls - coins, bushes and other objects show only within 2 grids of vision. To move, use WSAD.

If players collide, they both die, leaving their coins at their deathpoint, possible to collect. If they want them secured, they need to visit a campsite - the green point!
