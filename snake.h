#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h> // for system clear
#include <map>
#include <deque>
#include <algorithm>
using namespace std;
using std::chrono::system_clock;
using namespace std::this_thread;
char direction='r';


void input_handler(){
    // change terminal settings
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // turn off canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    map<char, char> keymap = {{'d', 'r'}, {'a', 'l'}, {'w', 'u'}, {'s', 'd'}, {'q', 'q'}};
    while (true) {
        char input = getchar();
        if (keymap.find(input) != keymap.end()) {
            // This now correctly modifies the single, shared 'direction' variable
            direction = keymap[input];
        }else if (input == 'q'){
            exit(0);
        }
        // You could add an exit condition here, e.g., if (input == 'q') break;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


void render_game(int size, deque<pair<int, int>> &snake, pair<int, int> food, pair<int,int> poison){
    for(size_t i=0;i<size;i++){
        for(size_t j=0;j<size;j++){
            if (i == food.first && j == food.second){
                cout << "🍎";
            }else if (i == poison.first && j == poison.second){
                cout << "☠️";  // poisonous food
            }else if (find(snake.begin(), snake.end(), make_pair(int(i), int(j))) != snake.end()) {
                cout << "🐍";
            }else{
                cout << "⬜";
            }
    }
    cout << endl;
}
}

pair<int,int> get_next_head(pair<int,int> current, char direction){
    pair<int, int> next; 
    if(direction =='r'){
        next = make_pair(current.first,(current.second+1) % 10);
    }else if (direction=='l')
    {
        next = make_pair(current.first, current.second==0?9:current.second-1);
    }else if(direction =='d'){
            next = make_pair((current.first+1)%10,current.second);
        }else if (direction=='u'){
            next = make_pair(current.first==0?9:current.first-1, current.second);
        }
    return next;
    
}

// #1 With these edits:
// Score increases by 10 each time food is eaten.
// Every 50 points, the snake moves faster (speed reduces by 50ms).
// Minimum speed is capped at 100ms.

//#2 Now food will never spawn inside the snake.

pair<int,int> generate_food(const deque<pair<int,int>> &snake, int size) {
    pair<int,int> food;
    while (true) {
        food = make_pair(rand() % size, rand() % size);
        // if food is NOT inside snake, return it
        if (find(snake.begin(), snake.end(), food) == snake.end()) {
            return food;
        }
    }
}

// #4
// Introduce a poisonous food (e.g., "☠️").
// It should not spawn inside the snake or overlap with normal food.
// If the snake eats poisonous food → Game Over (or alternatively deduct score, but usually it ends the game).
pair<int,int> generate_poison(const deque<pair<int,int>> &snake, int size, pair<int,int> food) {
    pair<int,int> poison;
    while (true) {
        poison = make_pair(rand() % size, rand() % size);
        // ensure it doesn't overlap with snake or normal food
        if (find(snake.begin(), snake.end(), poison) == snake.end() && poison != food) {
            return poison;
        }
    }
}

void game_play(){
    system("clear");
    deque<pair<int, int>> snake;
    snake.push_back(make_pair(0,0));

    // pair<int, int> food = make_pair(rand() % 10, rand() % 10);
    pair<int, int> food = generate_food(snake, 10);
    pair<int, int> poison = generate_poison(snake, 10, food);

    int score = 0;
    int speed = 500; // in milliseconds
    for(pair<int, int> head=make_pair(0,1);; head = get_next_head(head, direction)){
        // send the cursor to the top
        cout << "\033[H";
        // check self collision
        if (find(snake.begin(), snake.end(), head) != snake.end()) {
            system("clear");
            cout << "Game Over! Final Score: " << score << endl;
            exit(0);
        }else if (head.first == food.first && head.second == food.second) {
            // grow snake
            //food = make_pair(rand() % 10, rand() % 10);
            food = generate_food(snake, 10);
            poison = generate_poison(snake, 10, food); // spawn new poison too
            snake.push_back(head);          
            // update score and speed
            score += 10;  
            if (speed > 100 && score % 50 == 0) {  
                speed -= 50; // every 50 points, snake gets faster
            }  
        }else if (head.first == poison.first && head.second == poison.second) {
            // ate poison → game over
            system("clear");
            cout << "Game Over! You ate poison ☠️" << endl;
            cout << "Final Score: " << score << endl;
            exit(0);
        }else{
            // move snake
            snake.push_back(head);
            snake.pop_front();
        }
        render_game(10, snake, food, poison);
        cout << "length of snake: " << snake.size() << endl;
        cout << "Score: " << score << endl;  //fixed issue here
        sleep_for(chrono::milliseconds(speed)); // use dynamic speed
    }
}
