Chess Engine Documentation

Credits:
This project was made possible through the following resources and contributions:

Bluefever Software: For tutorial series and in-depth explanations of chess engine mechanics.
YouTube Channel: [BlueFeverSoft](https://www.youtube.com/@BlueFeverSoft)

Chess Programming Wiki: Comprehensive information about chess engine concepts.
Website: [ChessProgramming.org](https://www.chessprogramming.org/Main_Page)

ChatGPT: Assistance with C language-specific details and documentation writing.

Description and Components:

It is a computer program designed to play chess at a high level. It evaluates board positions and calculates the best moves using Alpha-Beta pruning, alongside the use of Move Ordering, Transposition Tables and Null Move Forward Pruning. This engine can operate in the console and can also communicate with a graphical user interface (GUI) or other software using either the UCI or the XBoard protocols. The engine is composed of the following components:

1. Board Representation: (defs.h)

    The board is represented as an array containing the 64 cells and will actually be of size 120 which includes off board square to
    stop them from being accounted into move generation. The values of the cells denote if it has a piece or is empty or is off board.
    There are 2 rows of extra cells each on the top and bottom and 1 column extra on the sides.

    This is needed by the engine to maintain chess positions for its search, evaluation and game-play. Besides mainting piece positions,
    it also stores additional information and accounts for the following:

        50 Move Rule: Something to track whether 50 moves have passed without a pawn move or a capture. That'll lead to a draw.

        3x Repetition: Something to track whether the same board position has occurred 3 times. A key assigning a unique value to the board
        position will helps with that. That'll lead to a draw.

        En Passant: Something to track the squares for En Passant.

        Side: Something to track which side is to play.

        Ply: Something to track amount of half moves played. (If W and B play, ply = 2 and if only W plays, ply = 1) 

2. Bitboards: (bitboards.c)

    These are very efficient implementation of a representation of the pawn structure for W, B and Both sides. These boards are implemented
    inside the board structure as 64 bit integers where the bits represent the board squares and whether a pawn is situated there or 
    not (1 or 0). There are many useful functions alongside it to aid working with the pawn structure of either side.

3. FEN Notation (io.c)

    Forsyth-Edwards Notation (FEN) describes a Chess Position. It is an one-line ASCII-string.The information is encoded as follows:
    <FEN> ::
        <Piece Placement> ' ' <Side to move> ' ' <Castling ability> ' ' <En passant target square> ' ' <Halfmove clock> ' ' <Fullmove counter>

4. Board Structure: (board.c)

    The structure for the board is defined in defs.h and many functionalities are implemented for setting the board up from the FEN representation 
    and updating the material of the board alongside checking the validity of the board.

5. Move format: 

    The structure for a move contains an integer that houses all the information within 31 bits of the integer.
    1. The first 7 bits are used to indicate the "From" square from where the piece moved.
    2. The next 7 bits are used to indicate the "To" square to where the piece moved.
    3. The next 4 bits are used to indicate the piece that was "Captured".
    4. The next 1 bit is used to indicate whether a capture was an "En Passant" capture.
    5. The next 1 bit is used to indicate whether the move was a "Pawn Start".
    6. The next 4 bits are used to indicate the piece a pawn promoted to, if the move led to that.
    7. The next 1 bit is used to indicate whether the move was a "Castling" move.

6. Move generation: (movegen.c, attack.c)

    It is handled in 3 steps:
    1. Move generation for pawns
    2. Move generation for sliding pieces (rooks, queens, bishops)
    3. Move generation for non-sliding pieces (nights, kings)

    The moves are record into one of three categories: Quiet Move (Normal), Capture Move, EnPassant Move.

6. Make Move/Undo Move: (makemove.c)

    For making the move, the following methodology/algorithm is implemented in code:
    1. Make "move".
    2. Get the "from" (sq), "to" (sq), and "cap" (captured piece) information from the move.
    3. Store the current position in the pos->history array.
    4. Move the current piece (from -> to).
    5. If a capture was made, remove the captured piece from the piece list.
    6. Update fifty move rule, and see if a pawn was moved.
    7. Check for promotions.
    8. Check for enPassant captures.
    9. If pawnstart, set the enPassant squares.
    10. For all the pieces added or removed, udpate the position counters and piece lists.
    11. Maintain Hashkey (posKey).
    12. Account for the castle permissions.
    13. Change the side and increment ply and hisPly.

7. Perft Testing: (perft.c)

    Perft Test is used to debug the move generation system of the engine. The principle behind this is to generate (all) moves upto a certain number of turns/ply (depth) and count
    all the possible positions that can result from the possible number of moves. This corresponds to searching a tree of all possible board positions where the edge
    denote taking a move and going to another position, and in the end returning the total number of leaf nodes of that tree.

8. Evaluation: (evaluate.c)

    The board is evaluated considering real world chess rules and norms. Many lookup tables with piece score on different squares of the board are used alongside 
    many other factors such as material advantage, king safety, passed pawns, isolated pawns, bishop pair and open/semi-open files in the evaluation function for the engine.
    
    The functions implemented in relation to this are:

        evalPosition(const S_BOARD *pos): evaluates the position of the board.

        materialDraw(const S_BOARD *pos): checks for a material draw for correct evaluation of a draw position.

9. Searching: (search.c, pvtable.c)

    Searching is the most important aspect of an engine as this is how an engine decides what move to make in a position. There are many algorithms for this task such as
    MiniMax and its slight modification Negamax. The Alpha-Beta Search is a more refined version of the previous algorithms (that search the whole search space) that improves by
    pruning the search tree using bounds Alpha and Beta (this is an example of the Branch-and-Bound problem solving paradigm). In the moce tree, white is always trying to maximize the score 
    and black is always trying to minimize the score. Alpha is the best value for the maximizer along the path to the root and Beta is the best value for the minimizer along the path to the 
    root. The algorithm for the Alpha-Beta Search is as follows:

        Algorithm:

        int AB(int depth, int alpha, int beta) {
            moveScore = -INF;

            depth < 1 ? return scoreFromSidePOV()

            for (each move in position) {
                makeMove;
                moveScore = -AB(depth - 1, - beta, -alpha);
                takeMove;
                if (moveScore >= beta)
                    return beta; // Beta-Cutoff
                if (moveScore > alpha)
                    alpha = moveScore; // Improve Alpha
            }

            return alpha;
        }

        Move Ordering:
        This is done to increase the search efficiency of the search by ordering the moves that need to searched. It is done in the following manner:
        1. Principal Variation Moves
        2. Capture Moves (Done according to the MVVLVA: Most Valuable Victim Least Valuable Attacker scheme)
        3. Killer Moves
        4. Quite Moves/Non Captures (according to the historyScore)

        Null Move Pruning:
        The idea behind this is simple: If the position for the engine is still good even after a null/free move, then no need for searching any line, simply return beta. This allows for searching 
        to greater depths in a givent time (around 1-2 depths more).

        The search is made even faster using the Transposition Table structure that allows for faster recall of evalaution scores if the position has already been searched to a deeper 
        depth in some other search. This allows for searching to even greater depths in a given time.

10. GUI Communication: (uci.c, xboard.c)

    The communication between the GUI and the engine is established using various protocols. The protocols used here are:

    UCI Protocol: A stateless, text-based protocol where the GUI manages the game state (move history, time control, etc.), and the engine focuses solely on move generation and evaluation.
    It allows for detailed configuration of engine parameters.

    XBoard Protocol: A simpler, text-based protocol where the GUI sends commands, and the engine handles the game state (including move generation and legal moves).
    The functionality to play in the console is also added.
    

The other files perform the following functions for the program:

    init.c: initializes all of the lookup tables and masks.
    engine.c: containing the main function for the program and is where the call to the game loop is given, according to the gamemode.
    hashkeys.c: generates the position key for the board position.
    misc.c: contains some non-chess specific utility functions for the program's functionality.
    utility.c: contains chess specific data and functions for the program to use.
    validate.c: contains functions used for checking and validation.
