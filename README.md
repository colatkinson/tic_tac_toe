Tic Tac Toe
===========

    Version: 1.0.0

A nifty little C program to generate playable Tic Tac Toe PDFs.

Description
-----------

Essentially, the program generates every possible game state, with each move representing a state transition. It's basically just a finite state machine.

It can operate in one- or two-player mode. In the former, it uses Alpha-Beta Minimax to determine the moves for the second player. In the latter, it allows both players to select their moves manually. Note that this results in significantly more possible states, and thus more pages and a larger document.

The program also allows for generating a document for electronic or print consumption. In electronic mode, the moves are clickable links, and the document opens in full screen mode automatically. In print mode, each empty square has the page number to turn to next, a&#768; la Choose Your Own Adventure books.

Don't want to generate documents on your own? I mean what kind of nerd doesn't love compiling and running C code? But fine, whatever floats your boat. Just check out the releases page.

Building
--------

Dependencies:

 * `libharu`

Make sure to recursively clone the repository, as it pulls in `uthash` as a submodule.

Then, in the directory you cloned it into, run `make`. The generated binary will be in the `build` subdirectory.

Usage
-----

    Usage: tic-tac-toe [-2pv] [-o FILE] [-h]
        2: 2-player mode. If set, will allow playing as both X and O
        p: Print mode. If set, will print line numbers
        o: Output to FILE. defaults to tic_tac_toe.pdf
        v: Verbose output
        h: Print this help message

Contributing
------------

Contributors are welcome! Just make a fork and shoot me a pull request. Preferably, document your code well and follow the current conventions.
