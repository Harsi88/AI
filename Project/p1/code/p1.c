/**
* This script searches the solution for an 8-puzzle problem through AI search techniques
* The blank tile is represented by the tile with the number 9
* The current search technique is a naive breadth first search
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


#define N 3         // NxN puzzle
#define MAXVALIDMOVES 4  // maximum number valid moves (4 for the center tile)
#define SOLDEPTH 4   // actual depth of the solution
#define MAX_DEPTH 17   // Maximum depth of tree uptill which algorithm will search for solution
#define _ff(w,g,h) w*g+(1-w)*h
#define w 1

typedef struct      // coordinates of a single tile
{
    int i;
    int j;
} Location;

struct Node             // node of the search space
{
    int **layout;       // tile configuration
    int g_val;          // cost to reach this node
    float h_val;
    float f_val;
    int move;           // move from the parent to reach this node
    struct Node *parent;// pointer to the parent. We will use this for tracing back
};

// search queue elements
struct SearchQueueElement
{
    struct Node *nodeptr;               // pointer to the state in the search space
    struct SearchQueueElement *next;    // pointer to the next element in the search queue
};

void SetGoal(int **a);       // Set the goal state of the puzzle
void PrintPuzzle(int **a);      // print the puzzle to the standard output
void MoveTile(int **a, Location x, int direction);     // move tile at location x along the direction
void Scramble(int **a);      // scramble the initial pattern moves number of times
void FindBlankTile(int **a, Location *blank);       // find the location of the blank tile
int IsValidMove(Location blank, int move);      // determine if a move is valid
int HeuristicMisplacedTiles(int **goal, int **a);   // compute the heuristic - number of misplaced tiles
int GoalTest(int **goal, int **a);      // Test if the current state is a goal state
void PrintPath(int **a, int *path);     // print the path to the goal state
int * BFS(int **goal, int **a);      // breadth first search
int * DFS(int **goal, int **a);      // depth first search
int * GBEFS(int **goal, int **a);      // greedy best first search
int * AStar(int **goal, int **a);      // A star search
int * IDAStar(int **goal, int **a);      // IDA star search
// search traversal functions
struct Node * CreateNode(int **a);         // create a node with the reuired information
struct SearchQueueElement *CreateSearchQueueElement(struct Node *curnode);        // Create hte search queue element
void AppendSearchQueueElementToEnd(struct SearchQueueElement* cursqelement);   // append a search queue elment to the end of the queue
void AppendSearchQueueElementToFront(struct SearchQueueElement* cursqelement);   // append a search queue elment to the Front of the queue
void InsertSearchQueueElementPriorityh(struct SearchQueueElement* cursqelement);   // append a search queue elment According to hueristic value
void InsertSearchQueueElementPriorityf(struct SearchQueueElement* cursqelement);   // append a search queue elment According to hueristic value
void FreeSearchMemory();


// search variables
struct SearchQueueElement *head = NULL;
int **goal;

int main(int argc, char *argv[])
{
    int i, j;
    int **puzzle;       // puzzle variable
    int *path;

    // allocate memory to the variable that stores the puzzle.
    puzzle = (int **)malloc(sizeof(int *)*N);
    goal = (int **)malloc(sizeof(int *)*N);
    for (i=0; i<N; i++)
    {
        puzzle[i] = (int *)calloc(N, sizeof(int));
        goal[i] = (int *)calloc(N, sizeof(int));
    }

    // set the goal state the puzzle
    SetGoal(puzzle);

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            goal[i][j] = puzzle[i][j];

    printf("Goal state tile configuration:\n");
    // print the goal tile configuration
    PrintPuzzle(goal);

    Scramble(puzzle);

    printf("Start state tile configuration:\n");
    // print the start state tile configuration
    PrintPuzzle(puzzle);

    // perform breadth first search
    //path = BFS(goal, puzzle);
    //path = DFS(goal, puzzle);
    //path = GBEFS(goal, puzzle);
    path = AStar(goal, puzzle);
    //path = IDAStar(goal, puzzle);
    //print path
//    PrintPath(puzzle, path);

    // free memory
    FreeSearchMemory();
    for (i=0; i<N; i++)
    {
        free(puzzle[i]);
        free(goal[i]);
    }
    free(puzzle);
    free(goal);
    free(path);

    return(1);
}

// This function sets the goal configuration of the tiles in the puzzle. Feel free to replace
// the code within the function to any create an goal layout of your liking.
// This will the goal state of the puzzle
void SetGoal(int **a)
{
    int i, j;
    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            a[i][j] = (i*N)+j+1;
    return;
}

// This function prints the 8 puzzle problem to the standard output. The blank tile
// is represented by the tile with the number 9
void PrintPuzzle(int **a)
{
    int i, j;

    for (i=0; i<N; i++)
    {
        for (j=0; j<N; j++)
            if (a[i][j] != 9)
                printf("%d ", a[i][j]);
            else
                printf("  ");

        printf("\n");
    }
    return;
}


// This function moves the tile at location x, along the 4 possible directions
// 0 - left
// 1 - right
// 2 - top
// 3 - bottom
void MoveTile(int **a, Location x, int direction)
{
    int temp;

    // value of the tile at the source location
    temp = a[x.i][x.j];

    switch (direction)
    {
        // move tile right
    case 0:
        a[x.i][x.j] = a[x.i][x.j-1];
        a[x.i][x.j-1] = temp;
        break;
        // move tile left
    case 1:
        a[x.i][x.j] = a[x.i][x.j+1];
        a[x.i][x.j+1] = temp;
        break;
        // move tile top
    case 2:
        a[x.i][x.j] = a[x.i-1][x.j];
        a[x.i-1][x.j] = temp;
        break;
        // move tile bottom
    case 3:
        a[x.i][x.j] = a[x.i+1][x.j];
        a[x.i+1][x.j] = temp;
        break;
    }
    return;
}

// This function scrambles the initial tile configuration using valid moves.
// It keeps track of the location of the blank tile
// It moves the blank tile along any of the valid moves a fixed number of times
// specified by moves.
void Scramble(int **a)
{
    int i, j;
    Location blank;
    int nvalidmoves=0, *validmoves;
    int move=0;     // stores the move in the previous iteration


    for (i=0; i<SOLDEPTH; i++)
    {
        // determine the initial location of the blank tile
        FindBlankTile(a, &blank);

        // list all valid moves
        // ignore the moves that cancel the previous move
        validmoves = (int *)calloc(MAXVALIDMOVES, sizeof(int));
        for (j=0; j<MAXVALIDMOVES; j++)
        {
            // ignore the moves that cancel previous move
            if ((j == 0 && move != 1) || (j == 1 && move != 0) ||
                    (j == 2 && move != 3) || (j == 3 && move != 2))
            {
                // consider only valid mvoes
                if (IsValidMove(blank, j)==1)
                {
                    validmoves[nvalidmoves] = j;
                    nvalidmoves++;
                }
            }
        }

        // select a move at random
        //srand((unsigned)time(NULL));
        move = validmoves[rand() % nvalidmoves];
        // perform the move
        MoveTile(a, blank, move);

        nvalidmoves = 0;
        free(validmoves);
    }
}

// This function determines the location of the blank tile in the puzzle
void FindBlankTile(int **a, Location *blank)
{
    int i, j;

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if (a[i][j] == 9)
            {
                blank->i = i;
                blank->j = j;
                return;
            }
}

// This function determines if a move is valid
// returns 1 if move is valid else 0
int IsValidMove(Location blank, int move)
{
    switch (move)
    {
        // left
    case 0:
        if (blank.j-1 >= 0)
            return(1);
        break;
        // right
    case 1:
        if (blank.j+1 <= N-1)
            return(1);
        break;
        // top
    case 2:
        if (blank.i-1 >= 0)
            return(1);
        break;
        // bottom
    case 3:
        if (blank.i+1 <= N-1)
            return(1);
        break;
    }
    return(0);
}

// This function computes the number of misplaced tiles
int HeuristicMisplacedTiles(int **goal, int **a)
{
    int i, j;
    int h=0;

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if (goal[i][j] != a[i][j])
                h++;
    return(h);
}

// This function checks if the current state is the goal state
int GoalTest(int **goal, int **a)
{
    int i, j;

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if (goal[i][j] != a[i][j])
                return(0);
    return(1);
}

// This function computes sum of Manhattan distance heuristic
int HeuristicManhattanDistance(int **goal, int **a)
{
    // Fill in the code
    int i, j, m, n;
    int h=0;
    int aij;
    
    for (i=0; i<N; i++){
        for (j=0; j<N; j++){
            for(m=0;m<N;m++){
                for(n=0;n<N;n++){
                    if(goal[m][n]!=9 && goal[m][n]==a[i][j])
                        h+=abs(i-m)+abs(j-n);
                }
            }
        }
    }
    return(h);
}

void PrintPath(int **a, int *path)
{
    int i;
    Location blank;

    printf("path to the goal node: \n");
    PrintPuzzle(a);
    for (i=path[0]; i>0; i--)
    {
        FindBlankTile(a, &blank);
        MoveTile(a, blank, path[i]);
        printf("move blank tile: ");
        switch (path[i])
        {
            case 0: printf("left");
                    break;
            case 1: printf("right");
                    break;
            case 2: printf("top");
                    break;
            case 3: printf("bottom");
                break;
        }
        printf("\n");
        PrintPuzzle(a);
    }
}

// This function performs breadth first search
int *BFS(int **goal, int **start)
{
/*    start[0][0]=2;
    start[0][1]=9;
    start[0][2]=4;
    start[1][0]=3;
    start[1][1]=1;
    start[1][2]=6;
    start[2][0]=7;
    start[2][1]=5;
    start[2][2]=8;
    
    start[0][0]=4;
    start[0][1]=1;
    start[0][2]=2;
    start[1][0]=9;
    start[1][1]=8;
    start[1][2]=7;
    start[2][0]=6;
    start[2][1]=3;
    start[2][2]=5;
*/    

    //////////////////////////////////////////////////////////////////// Parameters
    int nodes_expanded=0,nodes_generated=1,max_depth=0,memory_consumed=0;
    float computation_time,start_time,end_time;
    ////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////Computing start time
    start_time = clock();
    ////////////////////////////////////////////////////////////////////
    
    int i;
    struct Node *curnode;
    struct SearchQueueElement *cursqelement, *temphead;
    Location blank;
    int *path = NULL;

    // create the root node of the search tree
    curnode = CreateNode(start);

    // create the first element of the search queue
    if (head == NULL)
    {
        cursqelement = CreateSearchQueueElement(curnode);
        head = cursqelement;
    }

    temphead = head;

    

    while(temphead != NULL)
    {
        /////////////////////////////////////////// Computing parameters
        nodes_expanded++;            
        /////////////////////////////////////////////////////////////////        

        // check for goal
        if (GoalTest(goal, temphead->nodeptr->layout) == 1)
        {
            // we have found a goal state!
            printf("goal state found at depth: %d\n", temphead->nodeptr->g_val);
            // path[0] - length of the path
            // path[1:path[0]] - the moves in the path
            path = (int *)malloc(sizeof(int));
            path[0] = 0;
            // traverse the path in the reverse order - from goal to root
            // while traversing, store the blank tile moves for each step
            curnode = temphead->nodeptr;
            while (curnode->parent != NULL)
            {
                path = (int *)realloc(path, sizeof(int)*(path[0]+2));
                path[path[0]+1] = curnode->move;
                path[0]++;
                curnode = curnode->parent;
            }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////

            return(path);
        }
        FindBlankTile(temphead->nodeptr->layout, &blank);
        // compute the children of the current node
        for (i=0; i<MAXVALIDMOVES; i++)
        {
            if (IsValidMove(blank, i) == 1)
            {
                int lastmove = temphead->nodeptr->move;
                if( (i==0 && lastmove==1) || (i==1 && lastmove==0) || (i==2 && lastmove==3) || (i==3 && lastmove==2) ) continue;
                /////////////////////////////////////////////////////Computing nodes generated
                nodes_generated++;
                /////////////////////////////////////////////////////
                curnode = CreateNode(temphead->nodeptr->layout);
                MoveTile(curnode->layout, blank, i);
                curnode->move = i;
                curnode->g_val = temphead->nodeptr->g_val+1;
                ////////////////////////////////////////////////////Computing max depth reached
                if(max_depth < curnode->g_val){
                    max_depth = curnode->g_val;
                }
                ////////////////////////////////////////////////////
                curnode->parent = temphead->nodeptr;
                cursqelement = CreateSearchQueueElement(curnode);
                AppendSearchQueueElementToEnd(cursqelement);
            }
        }
        
        /////////////////////////////////////////////// Computing Memory consumed
        if(memory_consumed < nodes_generated-nodes_expanded){
            memory_consumed = nodes_generated-nodes_expanded;
        }
        ///////////////////////////////////////////////
        temphead = temphead->next;
    }
    
    /////////////////////////////////////////// Printing parameters
    end_time = clock();
    computation_time = end_time - start_time;
    printf("Nodes Expanded : %d\n",nodes_expanded);
    printf("Nodes Generated : %d\n",nodes_generated); 
    printf("Max Depth Reached : %d\n", max_depth);
    printf("Memory Consumed : %d\n", memory_consumed);
    printf("Computation Time : %f\n",computation_time);
    ////////////////////////////////////////////////////////////////////
    
    return(path);
}

// This function performs Depth first search
int *DFS(int **goal, int **start)
{
/*    start[0][0]=2;
    start[0][1]=9;
    start[0][2]=4;
    start[1][0]=3;
    start[1][1]=1;
    start[1][2]=6;
    start[2][0]=7;
    start[2][1]=5;
    start[2][2]=8;
    
    start[0][0]=4;
    start[0][1]=1;
    start[0][2]=2;
    start[1][0]=9;
    start[1][1]=8;
    start[1][2]=7;
    start[2][0]=6;
    start[2][1]=3;
    start[2][2]=5;
*/    
    //////////////////////////////////////////////////////////////////// Parameters
    int nodes_expanded=0,nodes_generated=1,max_depth=0,memory_consumed=0;
    float computation_time,start_time,end_time;
    ////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////Computing start time
    start_time = clock();
    ////////////////////////////////////////////////////////////////////

    int i;
    struct Node *curnode;
    struct SearchQueueElement *cursqelement, *temphead;
    Location blank;
    int *path = NULL;

    // create the root node of the search tree
    curnode = CreateNode(start);

    // create the first element of the search queue
    if (head == NULL)
    {
        cursqelement = CreateSearchQueueElement(curnode);
        head = cursqelement;
    }

    temphead = head;

    

    while(temphead != NULL)
    {
        /////////////////////////////////////////// Computing parameters
        nodes_expanded++;            
        /////////////////////////////////////////////////////////////////        
        head = head->next;
        // check for goal
        if (GoalTest(goal, temphead->nodeptr->layout) == 1)
        {
            // we have found a goal state!
            printf("goal state found at depth: %d\n", temphead->nodeptr->g_val);
            // path[0] - length of the path
            // path[1:path[0]] - the moves in the path
            path = (int *)malloc(sizeof(int));
            path[0] = 0;
            // traverse the path in the reverse order - from goal to root
            // while traversing, store the blank tile moves for each step
            curnode = temphead->nodeptr;
            while (curnode->parent != NULL)
            {
                path = (int *)realloc(path, sizeof(int)*(path[0]+2));
                path[path[0]+1] = curnode->move;
                path[0]++;
                curnode = curnode->parent;
            }
            
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////

            return(path);
        }
        FindBlankTile(temphead->nodeptr->layout, &blank);
        // compute the children of the current node
        if (temphead->nodeptr->g_val > MAX_DEPTH){
            struct SearchQueueElement* temp;
            temp = temphead;
            temphead = head;
            free(temp);
            continue;
        }
        for (i=0; i<MAXVALIDMOVES; i++)
        {
            if (IsValidMove(blank, i) == 1)
            {
                int lastmove = temphead->nodeptr->move;
                if( (i==0 && lastmove==1) || (i==1 && lastmove==0) || (i==2 && lastmove==3) || (i==3 && lastmove==2) ) continue;
                /////////////////////////////////////////////////////Computing nodes generated
                nodes_generated++;
                /////////////////////////////////////////////////////
                curnode = CreateNode(temphead->nodeptr->layout);
                MoveTile(curnode->layout, blank, i);
                curnode->move = i;
                curnode->g_val = temphead->nodeptr->g_val+1;
                ////////////////////////////////////////////////////Computing max depth reached
                if(max_depth < curnode->g_val){
                    max_depth = curnode->g_val;
                }
                ////////////////////////////////////////////////////
                curnode->parent = temphead->nodeptr;
                cursqelement = CreateSearchQueueElement(curnode);
                AppendSearchQueueElementToFront(cursqelement);
            }
        }
        /////////////////////////////////////////////// Computing Memory consumed
        if(memory_consumed < nodes_generated-nodes_expanded){
            memory_consumed = nodes_generated-nodes_expanded;
        }
        ///////////////////////////////////////////////
        struct SearchQueueElement* temp;
        temp = temphead;
        temphead = head;
        free(temp);        
    }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
    return(path);
}

// This function performs Greedy best first search
int *GBEFS(int **goal, int **start)
{
/*    start[0][0]=2;
    start[0][1]=9;
    start[0][2]=4;
    start[1][0]=3;
    start[1][1]=1;
    start[1][2]=6;
    start[2][0]=7;
    start[2][1]=5;
    start[2][2]=8;
    
    start[0][0]=4;
    start[0][1]=1;
    start[0][2]=2;
    start[1][0]=9;
    start[1][1]=8;
    start[1][2]=7;
    start[2][0]=6;
    start[2][1]=3;
    start[2][2]=5;
*/    
    
    //////////////////////////////////////////////////////////////////// Parameters
    int nodes_expanded=0,nodes_generated=1,max_depth=0,memory_consumed=0;
    float computation_time,start_time,end_time;
    ////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////Computing start time
    start_time = clock();
    ////////////////////////////////////////////////////////////////////
        
    int i;
    struct Node *curnode;
    struct SearchQueueElement *cursqelement, *temphead;
    Location blank;
    int *path = NULL;

    // create the root node of the search tree
    curnode = CreateNode(start);

    // create the first element of the search queue
    if (head == NULL)
    {
        cursqelement = CreateSearchQueueElement(curnode);
        head = cursqelement;
    }

    temphead = head;    
    while(temphead != NULL)
    {   
        //////////////////////////////////////////////////////////
        nodes_expanded++;
        //////////////////////////////////////////////////////////
        
        head = head->next;
//        printf("%p\n",head);
        // check for goal
        if (GoalTest(goal, temphead->nodeptr->layout) == 1)
        {
            // we have found a goal state!
            printf("goal state found at depth: %d\n", temphead->nodeptr->g_val);
            // path[0] - length of the path
            // path[1:path[0]] - the moves in the path
            path = (int *)malloc(sizeof(int));
            path[0] = 0;
            // traverse the path in the reverse order - from goal to root
            // while traversing, store the blank tile moves for each step
            curnode = temphead->nodeptr;
            while (curnode->parent != NULL)
            {
                path = (int *)realloc(path, sizeof(int)*(path[0]+2));
                path[path[0]+1] = curnode->move;
                path[0]++;
                curnode = curnode->parent;
            }

            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////

            return(path);
        }
        FindBlankTile(temphead->nodeptr->layout, &blank);
        // compute the children of the current node
        if (temphead->nodeptr->g_val > MAX_DEPTH){
            struct SearchQueueElement* temp;
            temp = temphead;
            temphead = head;
            free(temp);
            continue;
        }
        for (i=0; i<MAXVALIDMOVES; i++)
        {
            if (IsValidMove(blank, i) == 1)
            {
                int lastmove = temphead->nodeptr->move;
                if( (i==0 && lastmove==1) || (i==1 && lastmove==0) || (i==2 && lastmove==3) || (i==3 && lastmove==2) ) continue;
                ///////////////////////////////////////////////////////////
                nodes_generated++;
                ///////////////////////////////////////////////////////////
                curnode = CreateNode(temphead->nodeptr->layout);
                MoveTile(curnode->layout, blank, i);
                curnode->move = i;
                curnode->g_val = temphead->nodeptr->g_val+1;
                ////////////////////////////////////////////////////Computing max depth reached
                if(max_depth < curnode->g_val){
                    max_depth = curnode->g_val;
                }
                ////////////////////////////////////////////////////
                curnode->h_val = HeuristicMisplacedTiles(goal,curnode->layout);
                curnode->parent = temphead->nodeptr;
                cursqelement = CreateSearchQueueElement(curnode);
                cursqelement->next = NULL;
                InsertSearchQueueElementPriorityh(cursqelement);
            }
        }
        /////////////////////////////////////////////// Computing Memory consumed
        if(memory_consumed < nodes_generated-nodes_expanded){
            memory_consumed = nodes_generated-nodes_expanded;
        }
        ///////////////////////////////////////////////
        struct SearchQueueElement* temp;
        temp = temphead;
        temphead = head;
        free(temp);
    }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
    return(path);
}

// This function performs A* search
int *AStar(int **goal, int **start)
{
/*    start[0][0]=2;
    start[0][1]=9;
    start[0][2]=4;
    start[1][0]=3;
    start[1][1]=1;
    start[1][2]=6;
    start[2][0]=7;
    start[2][1]=5;
    start[2][2]=8;
    
    start[0][0]=4;
    start[0][1]=1;
    start[0][2]=2;
    start[1][0]=9;
    start[1][1]=8;
    start[1][2]=7;
    start[2][0]=6;
    start[2][1]=3;
    start[2][2]=5;
*/
    //////////////////////////////////////////////////////////////////// Parameters
    int nodes_expanded=0,nodes_generated=1,max_depth=0,memory_consumed=0;
    float computation_time,start_time,end_time;
    ////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////Computing start time
    start_time = clock();
    ////////////////////////////////////////////////////////////////////

    int i;
    struct Node *curnode;
    struct SearchQueueElement *cursqelement, *temphead;
    Location blank;
    int *path = NULL;

    // create the root node of the search tree
    curnode = CreateNode(start);

    // create the first element of the search queue
    if (head == NULL)
    {
        cursqelement = CreateSearchQueueElement(curnode);
        head = cursqelement;
    }

    temphead = head;    
    while(temphead != NULL)
    {
        ///////////////////////////////////////////////////////////////////////////////
        nodes_expanded++;
        ///////////////////////////////////////////////////////////////////////////////
        
        head = head->next;
//        printf("%p\n",head);
        // check for goal
        if (GoalTest(goal, temphead->nodeptr->layout) == 1)
        {
            // we have found a goal state!
            printf("goal state found at depth: %d\n", temphead->nodeptr->g_val);
            // path[0] - length of the path
            // path[1:path[0]] - the moves in the path
            path = (int *)malloc(sizeof(int));
            path[0] = 0;
            // traverse the path in the reverse order - from goal to root
            // while traversing, store the blank tile moves for each step
            curnode = temphead->nodeptr;
            while (curnode->parent != NULL)
            {
                path = (int *)realloc(path, sizeof(int)*(path[0]+2));
                path[path[0]+1] = curnode->move;
                path[0]++;
                curnode = curnode->parent;
            }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
            return(path);
        }
        FindBlankTile(temphead->nodeptr->layout, &blank);
        // compute the children of the current node
        if (temphead->nodeptr->g_val > MAX_DEPTH){
            struct SearchQueueElement* temp;
            temp = temphead;
            temphead = head;
            free(temp);
            continue;
        }
        for (i=0; i<MAXVALIDMOVES; i++)
        {
            if (IsValidMove(blank, i) == 1)
            {
                int lastmove = temphead->nodeptr->move;
                if( (i==0 && lastmove==1) || (i==1 && lastmove==0) || (i==2 && lastmove==3) || (i==3 && lastmove==2) ) continue;
                ///////////////////////////////////////////////////////////////
                nodes_generated++;
                ///////////////////////////////////////////////////////////////
                curnode = CreateNode(temphead->nodeptr->layout);
                MoveTile(curnode->layout, blank, i);
                curnode->move = i;
                curnode->g_val = temphead->nodeptr->g_val+1;
                ////////////////////////////////////////////////////Computing max depth reached
                if(max_depth < curnode->g_val){
                    max_depth = curnode->g_val;
                }
                ////////////////////////////////////////////////////
                //curnode->h_val = HeuristicMisplacedTiles(goal,curnode->layout);
                curnode->h_val = HeuristicManhattanDistance(goal,curnode->layout);
                //curnode->f_val = curnode->g_val + curnode->h_val;
                curnode->f_val = _ff(w,curnode->g_val,curnode->h_val);
                curnode->parent = temphead->nodeptr;
                cursqelement = CreateSearchQueueElement(curnode);
                cursqelement->next = NULL;
                InsertSearchQueueElementPriorityf(cursqelement);
            }
        }
        /////////////////////////////////////////////// Computing Memory consumed
        if(memory_consumed < nodes_generated-nodes_expanded){
            memory_consumed = nodes_generated-nodes_expanded;
        }
        ///////////////////////////////////////////////
        struct SearchQueueElement* temp;
        temp = temphead;
        temphead = head;
        free(temp);
    }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
    return(path);
}

// This function performs IDA* search
int *IDAStar(int **goal, int **start)
{
/*    start[0][0]=2;
    start[0][1]=9;
    start[0][2]=4;
    start[1][0]=3;
    start[1][1]=1;
    start[1][2]=6;
    start[2][0]=7;
    start[2][1]=5;
    start[2][2]=8;
    
    start[0][0]=4;
    start[0][1]=1;
    start[0][2]=2;
    start[1][0]=9;
    start[1][1]=8;
    start[1][2]=7;
    start[2][0]=6;
    start[2][1]=3;
    start[2][2]=5;
*/
    //////////////////////////////////////////////////////////////////// Parameters
    int nodes_expanded=0,totalnodes_expanded=0,nodes_generated=1,totalnodes_generated=1,max_depth=0,memory_consumed=0;
    float computation_time,start_time,end_time;
    ////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////Computing start time
    start_time = clock();
    ////////////////////////////////////////////////////////////////////

    int i;
    struct Node *curnode;
    struct SearchQueueElement *cursqelement, *temphead;
    Location blank;
    int *path = NULL;
    
    int fdepth = 0,nextmin_fdepth=999999;
    
    curnode = CreateNode(start);    
    fdepth = curnode->f_val;
    
    while(1)
    {
    nextmin_fdepth=999999;
    curnode = CreateNode(start);    
    // create the first element of the search queue
    if (head == NULL)
    {
        cursqelement = CreateSearchQueueElement(curnode);
        head = cursqelement;
    }

    temphead = head;    
    while(temphead != NULL)
    {
        ///////////////////////////////////////////////////////////////////////
        nodes_expanded++;
        ///////////////////////////////////////////////////////////////////////
        head = head->next;
//        printf("%p\n",head);
        // check for goal
        if (GoalTest(goal, temphead->nodeptr->layout) == 1)
        {
            // we have found a goal state!
            printf("goal state found at depth: %d\n", temphead->nodeptr->g_val);
            // path[0] - length of the path
            // path[1:path[0]] - the moves in the path
            path = (int *)malloc(sizeof(int));
            path[0] = 0;
            // traverse the path in the reverse order - from goal to root
            // while traversing, store the blank tile moves for each step
            curnode = temphead->nodeptr;
            while (curnode->parent != NULL)
            {
                path = (int *)realloc(path, sizeof(int)*(path[0]+2));
                path[path[0]+1] = curnode->move;
                path[0]++;
                curnode = curnode->parent;
            }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
            return(path);
        }
        FindBlankTile(temphead->nodeptr->layout, &blank);
        // compute the children of the current node
        for (i=0; i<MAXVALIDMOVES; i++)
        {
            if (IsValidMove(blank, i) == 1)
            {
                int lastmove = temphead->nodeptr->move;
                if( (i==0 && lastmove==1) || (i==1 && lastmove==0) || (i==2 && lastmove==3) || (i==3 && lastmove==2) ) continue;
                //////////////////////////////////////////////////////////////
                nodes_generated++;
                //////////////////////////////////////////////////////////////
                curnode = CreateNode(temphead->nodeptr->layout);
                MoveTile(curnode->layout, blank, i);
                curnode->move = i;
                curnode->g_val = temphead->nodeptr->g_val+1;
                ////////////////////////////////////////////////////Computing max depth reached
                if(max_depth < curnode->g_val){
                    max_depth = curnode->g_val;
                }
                ////////////////////////////////////////////////////
                curnode->h_val = HeuristicMisplacedTiles(goal,curnode->layout);
                curnode->f_val = curnode->g_val + curnode->h_val;
                if(curnode->f_val > fdepth){
                    if(curnode->f_val < nextmin_fdepth){
                        nextmin_fdepth = curnode->f_val;
                    }
                    continue;
                }
                curnode->parent = temphead->nodeptr;
                cursqelement = CreateSearchQueueElement(curnode);
                cursqelement->next = NULL;
                InsertSearchQueueElementPriorityf(cursqelement);
            }
        }
        /////////////////////////////////////////////// Computing Memory consumed
        if(memory_consumed < nodes_generated-nodes_expanded){
            memory_consumed = nodes_generated-nodes_expanded;
        }
        ///////////////////////////////////////////////
        struct SearchQueueElement* temp;
        temp = temphead;
        temphead = head;
        free(temp);
    }
    
    ////////////////////////////////////////////////////////////////////////
    totalnodes_expanded += nodes_expanded;
    totalnodes_generated += nodes_generated;
    nodes_expanded=nodes_generated=0;
    ////////////////////////////////////////////////////////////////////////
    
    fdepth = nextmin_fdepth;
        
    }
            /////////////////////////////////////////// Printing parameters
            end_time = clock();
            computation_time = end_time - start_time;
    
            printf("Nodes Expanded : %d\n",nodes_expanded);
            printf("Nodes Generated : %d\n",nodes_generated); 
            printf("Max Depth Reached : %d\n", max_depth);
            printf("Memory Consumed : %d\n", memory_consumed);
            printf("Computation Time : %f\n",computation_time);
            ////////////////////////////////////////////////////////////////////
    return(path);
}

// This function creates a node variable. Copies the contents of the layout of the node,
// computes the heuristic values
struct Node *CreateNode(int **a)
{
    int i, j;
    struct Node *curnode;

    curnode = (struct Node *)malloc(sizeof(struct Node));
    curnode->layout = (int **)malloc(sizeof(int*)*N);
    for (i=0; i<N; i++)
    {
        curnode->layout[i] = (int *)malloc(sizeof(int)*N);
        for (j=0; j<N; j++)
            curnode->layout[i][j] = a[i][j];
    }
    curnode->parent = NULL;
    curnode->g_val = 0;
    curnode->h_val = HeuristicMisplacedTiles(goal,curnode->layout);
    curnode->f_val = curnode->g_val + curnode->h_val;
    curnode->move = -1;

    return(curnode);
}

// This function creates a search queue element
struct SearchQueueElement *CreateSearchQueueElement(struct Node *curnode)
{
    struct SearchQueueElement *cursqelement;

    cursqelement = (struct SearchQueueElement*)malloc(sizeof(struct SearchQueueElement));
    cursqelement->nodeptr = curnode;
    cursqelement->next = NULL;

    return(cursqelement);

}

// This function appends a search queue element to the end of the queue - for breadth first search
void AppendSearchQueueElementToEnd(struct SearchQueueElement* cursqelement)
{
    struct SearchQueueElement *tempsqelement;
    if (head != NULL)
    {
        tempsqelement = head;
        while (tempsqelement->next != NULL)
            tempsqelement = tempsqelement->next;
        tempsqelement->next = cursqelement;
    }
    else
        head = cursqelement;
    return;
}


// This function appends a search queue element to the front of the queue - for Depth first search
void AppendSearchQueueElementToFront(struct SearchQueueElement* cursqelement)
{
    if (head != NULL)
    {
        cursqelement->next = head;
        head = cursqelement;
    }
    else
        head = cursqelement;
    return;
}

// This function appends a search queue element to the front of the queue - for Greedy best first search
void InsertSearchQueueElementPriorityh(struct SearchQueueElement* cursqelement)
{
    int h_val = cursqelement->nodeptr->h_val;
    struct SearchQueueElement *tempsqelement;
    if (head != NULL)
    {
        if(head->nodeptr->h_val > h_val){
            cursqelement->next = head;
            head = cursqelement;
        }
        else{
            tempsqelement = head;
            if(head->next == NULL){
                head->next = cursqelement;
            }
            else{
                while (tempsqelement->next != NULL && tempsqelement->next->nodeptr->h_val < h_val)
                    tempsqelement = tempsqelement->next;
        
                cursqelement->next = tempsqelement->next;
                tempsqelement->next = cursqelement;
            }
        }
    }
    else
        head = cursqelement;
    return;
}

void InsertSearchQueueElementPriorityf(struct SearchQueueElement* cursqelement)
{
    int f_val = cursqelement->nodeptr->f_val;
    struct SearchQueueElement *tempsqelement;
    if (head != NULL)
    {
        if(head->nodeptr->f_val > f_val){
            cursqelement->next = head;
            head = cursqelement;
        }
        else{
            tempsqelement = head;
            if(head->next == NULL){
                head->next = cursqelement;
            }
            else{
                while (tempsqelement->next != NULL && tempsqelement->next->nodeptr->f_val < f_val)
                    tempsqelement = tempsqelement->next;
        
                cursqelement->next = tempsqelement->next;
                tempsqelement->next = cursqelement;
            }
        }
    }
    else
        head = cursqelement;
    return;
}


void FreeSearchMemory()
{
    int i;
    struct SearchQueueElement *cursqelement, *tempsqelement;
    struct Node *curnode;

    cursqelement = head;
    while (cursqelement != NULL)
    {
        tempsqelement = cursqelement;
        cursqelement = cursqelement->next;
        curnode = tempsqelement->nodeptr;
        free(tempsqelement);
        for (i=0; i<N; i++)
            free(curnode->layout[i]);
        free(curnode->layout);
        free(curnode);
    }
    head = NULL;
}
