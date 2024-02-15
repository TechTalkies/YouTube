#define SELECT 2
#define START_SPEED 5
#define PADDLE_START 2
#define PADDLE_SIZE 3
#define BRICKS 24

enum {START, WON, LOST, PLAY, SETUP} state = SETUP;
enum {LEFT, RIGHT, NONE} controls = NONE;
enum {SEL, NO} actions = NO;