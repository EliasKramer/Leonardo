# Leonardo

Leonardo is currently at about 1900elo in Lichess https://lichess.org/@/Leonardo-Bot (It may not be online, since I don't have the means to host it 24/7

## Features
* Opening Book
* Iterative Deepening
* NegaMax
* Transposition Table
* Quiescence Search
* Killer Move Heuristic
* Butterfly History Heuristic
* NMP (Null Move Pruning)
* SSE (Static Exchange Evaluation)
* MVV LVA (Most Valuable Victim, Least Valuable Attacker)
* Selfmade NNUE (Efficiently Updatable Neural Network)
## Planned Features
* Principal Variation Search
* LMR (Late Move Reduction)
* etc

### Some Details about the NNUE
Another Project of mine is a neural network (https://github.com/EliasKramer/NeuroFox), which I incorperated into this project. 
The most important features of a NNUE (e.g. partial forward feeding) were implemented. 
The difference to other NNUEs is that it only takes pawn structure as input. This makes it a lot faster than a common NNUE, but it doesnt boost the elo that much.

## Thoughts

This project has probably a serious bug, because a Bot with these features should be in the range of 2700 elo
