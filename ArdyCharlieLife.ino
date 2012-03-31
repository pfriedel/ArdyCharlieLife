/*
  USE_GITHUB_USERNAME=pfriedel
*/

/*
  TinyChuckArdy:
  
  A (light) conversion of http://b2ben.blogspot.com/2011/02/diving-into-microcontrollers-my-tiny.html for Arduino.

*/

// http://www.hobbytronics.co.uk/arduino-atmega328-pinout

#define LINE_A 2 //Pin 2 (PD2) on ATmega328
#define LINE_B 3 //Pin 3 (PD3) on ATmega328
#define LINE_C 4 //Pin 4 (PD4) on ATmega328
#define LINE_D 5 //Pin 5 (PD5) on ATmega328
#define LINE_E 6 //Pin 6 (PD6) on ATmega328

// how many times should it redraw the screen before moving on to the next generation?
#define FRAME_DELAY 500
#define SETUP_FRAME_DELAY 50

#include <avr/io.h>

/*
  Theory of operation:

  1) Write changes to led_grid_next[].
  2) call fade_to_next_frame to smoothly transition between the current frame and the next frame
  3) Purge changes in led_grid_next unless if you want them to persist.  fade_to_next_frame doesn't swap framebuffers.

*/
char led_grid[20] = {
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000
};

char led_grid_next[20] = {
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000 ,
  000 , 000 , 000 , 000 , 000
};

void setup() {
  randomSeed(analogRead(0));
// Just a few simple LED testing sweeps...
  positive_h_test();
  negative_h_test();
  positive_v_test();
  negative_v_test();
  positive_h_line_test();
  negative_h_line_test();
}

void loop() {
  char led;
  
  while(1){
    int f, frame_number;
    frame_number = 0;
    
    initialize_frame_log();
    
    //fade to all-on LED grid
    for ( led=0; led<=19; led++ ) { led_grid_next[led] = 100; }
    fade_to_next_frame();
    for( f=0 ; f<FRAME_DELAY ; f++ ){ draw_frame(); } //display this frame for awhile
    
    //fade to random start frame
    set_random_next_frame();
    fade_to_next_frame();
    
    while(1){
      
      if( frame_number==0 ){ log_current_frame(); } //log every 20th frame, compare future frames against it to check for oscillators
      
      for( f=0 ; f<FRAME_DELAY ; f++ ){ draw_frame(); }
      
      generate_next_generation();			
      
      if( current_equals_next() == 1 ){
	//next generated frame is the same as the current frame... "still life" achieved
	for( f=0 ; f<1500 ; f++ ){ 
	  draw_frame(); 
	} //stay on frame for awhile
	break; //exit this loop & start over
      }
      
      //next frame is not the same as the last
      if( next_equals_logged_frame() == 1 ){
	//next frame matches one we've had before, we have a repeating pattern
	//show the next frame
	fade_to_next_frame();
	for( f=0 ; f<FRAME_DELAY ; f++ ){ 
	  draw_frame(); 
	}
	break; //exit this loop & start over
      }
      
      fade_to_next_frame();
      frame_number++;
      if( frame_number==20 ){ 
	frame_number = 0; 
      }
      
      if( current_living_cells() == 0 ){
	for( f=0 ; f<1500 ; f++ ){ 
	  draw_frame(); 
	} //stay on dead frame for awhile
	break; //all cells dead, exit this loop & start over
      }
    }
  }
}

void positive_h_test() {
  // counts up from the lower right hand corner horizontally, individually lighting lamps
  for(int x = 0; x<=19; x++) {
    // set this LED to 100% brightness in the next frame
    led_grid_next[x] = 100;
    // Display the results of the changes to the next frame
    for(int f = 0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
    // remove this LED from the next frame.
    led_grid_next[x] = 0;
  }
}

void negative_h_line_test() {
  // horizontal lines descending the grid
  for(int repeat = 0; repeat<=1; repeat++) { 
    for(int y=4; y>=1; y--) {
      for(int x=1; x<=5; x++) {
	int k;
	k = ((5*y)+x)-6;
	led_grid_next[k] = 100; // light the LED in the current row (y) to 100%
	led_grid_next[k+5] = 0; // drop the LED in the prior row (y+5) to 0%
      }
      for(int f=0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
      for( int cell=0 ; cell<20 ; cell++ ){ led_grid_next[cell] = -1; }
    }
  }
}

void positive_h_line_test() {
  // horizontal lines ascending the grid
  for(int repeat = 0; repeat<=1; repeat++) { 
    for(int y=1; y<=4; y++) {
      for(int x=1; x<=5; x++) {
	int k;
	k = ((5*y)+x)-6;
	led_grid_next[k] = 100;
	led_grid_next[k-5] = 0;
      }
      for(int f=0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
      for( int cell=0 ; cell<20 ; cell++ ){ led_grid_next[cell] = -1; }
    }
  }
}

void negative_v_test() {
  // counts down from the upper left hand corner vertically, lighting each LED individually.
  for(int i = 4; i>=0; i--) {
    for(int j = 4; j>=0; j--) {
      int x;
      x = (5*j+i);
      if(x>20) { continue; } 
      if(x<0) { continue; }
      led_grid_next[x] = 100;
      for(int f = 0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
      led_grid_next[x] = 0;
    }
  }
}

void positive_v_test() {
  // counts up from the lower left corner vrtically, individually lighting lamps
  for(int i = 0; i<=4; i++) {
    for(int j = 0; j<=4; j++) {
      int x;
      x = (5*j+i);
      if(x>20) { continue; } 
      led_grid_next[x] = 100;
      for(int f = 0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
      led_grid_next[x] = 0;
    }
  }
}

void negative_h_test() {
  // Counts down from the upper left hand corner, individually lighting lamps.
  for(int x = 19; x>=0; x--) {
    led_grid_next[x] = 100;
    for(int f = 0; f<=SETUP_FRAME_DELAY; f++) { fade_to_next_frame(); }
    led_grid_next[x] = 0;
  }
}

char current_equals_next(){
  char led, diffs;
  diffs = 0;
  for(led=0;led<=19;led++){
    if( led_grid[led] != led_grid_next[led] ){ 
      return 0; 
    } //a difference was found, return 0 (for False)
  }
  return 1; //no differences found, return 1 (for True)
}

char current_living_cells(){
  char led, cells;
  for ( led=0; led<=19; led++ ) {
    if( led_grid[led] > 0 ){ 
      cells++; 
    }
  }
  return cells;
}


/*
 * pseudorandom - From Instructables: http://www.instructables.com/id/Flickering-LED-Candle/
 * return the next pseudo-random number (PRN) using a standard maximum
 * length xor-feedback 16-bit shift register.
 * This returns the number from 1 to 65535 in a fixed but apparently
 * random sequence.  No one number repeats.
 */

// this is all kind of ridiculous on an Arduino with 32k of ram - the original
// code is for a very small device. I left in the function in case anyone prefers
// their randomness to be pseudo.

uint16_t randreg = 222; //seed value
static uint16_t pseudorandom16 (void)
{
  /*    uint16_t newbit = 0;
   
   if (randreg == 0) {
   randreg = 1;
   }
   if (randreg & 0x8000) newbit = 1;
   if (randreg & 0x4000) newbit ^= 1;
   if (randreg & 0x1000) newbit ^= 1;
   if (randreg & 0x0008) newbit ^= 1;
   randreg = (randreg << 1) + newbit;
   */
  randreg = random(65535);
  return randreg;
} 


void set_random_next_frame(void){
  char i;

  //clear LED Grid
  for ( i=0; i<=19; i++ ) {
    led_grid_next[i] = 0;
  }

  char total, cell;
  total = ((pseudorandom16() % 16)+5); //yield random from 5 to 20

  //set random cells
  for( i=0 ; i<total ; i++ ) {
    cell = (pseudorandom16() % 20);
    led_grid_next[cell] = 100;
  }
}

char frame_log[20];
void initialize_frame_log(){
  char cell;
  for( cell=0 ; cell<20 ; cell++ ){
    frame_log[cell] = -1;
  }
}
void log_current_frame(){
  char cell;
  for( cell=0 ; cell<20 ; cell++ ){
    frame_log[cell] = led_grid[cell];
  }
}

int next_equals_logged_frame(){
  char cell, diffs;
  diffs = 0;
  for( cell=0 ; cell<=19 ; cell++ ){
    if( led_grid_next[cell] != frame_log[cell] ){ 
      return 0; 
    } //a difference was found, return 0 (for False)
  }
  return 1; //no differences found, return 1 (for True)
}
void set_led_next_xy( char col, char row, char value ){
  //5 columns, 4 rows, zero-indexed
  led_grid_next[((5*row)+col)] = value;
}

char get_led_xy( char col, char row ){
  //5 columns, 4 rows, zero-indexed
  if( col<0 | col>4 ){ return 0; }
  if( row<0 | row>3 ){ return 0; }
  return led_grid[((5*row)+col)];
}
void generate_next_generation(void){  //looks at current generation, writes to next generation array
  char row, col, neighbors;
  for ( row=0; row<=3; row++ ) {
    for ( col=0; col<=4; col++ ) {
      //look at cell (col,row) in current generation

      //count the number of current neighbors - currently planar.  I'd love to make it toroidal.
      neighbors = 0;
      if( get_led_xy((col-1),(row-1)) > 0 ) { neighbors++; } //NW
      if( get_led_xy(( col ),(row-1)) > 0 ) { neighbors++; } //N
      if( get_led_xy((col+1),(row-1)) > 0 ) { neighbors++; } //NE
      if( get_led_xy((col-1),( row )) > 0 ) { neighbors++; } //W
      if( get_led_xy((col+1),( row )) > 0 ) { neighbors++; } //E
      if( get_led_xy((col-1),(row+1)) > 0 ) { neighbors++; } //SW
      if( get_led_xy(( col ),(row+1)) > 0 ) { neighbors++; } //S
      if( get_led_xy((col+1),(row+1)) > 0 ) { neighbors++; } //SE

      if( get_led_xy(col,row) > 0 ){
	
        //current cell is alive
        if( neighbors < 2 ){
          //Any live cell with fewer than two live neighbours dies, as if caused by under-population.
          set_led_next_xy( col , row , 0 );
        }
        if( (neighbors == 2) || (neighbors == 3) ){
          //Any live cell with two or three live neighbours lives on to the next generation.
          set_led_next_xy( col , row , 100 );
        }
        if( neighbors > 3 ){
          //Any live cell with more than three live neighbours dies, as if by overcrowding.
          set_led_next_xy( col , row , 0 );
        }

      } 
      else {

        //current cell is dead
        if( neighbors == 3 ){
          // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
          set_led_next_xy( col , row , 100 );
        } 
        else {
          //stay dead for next generation
          set_led_next_xy( col , row , 0 );
        }

      }
    }
  }
}

const char led_dir[20] = {
  ( 1<<LINE_A | 1<<LINE_E ), //LED 0
  ( 1<<LINE_B | 1<<LINE_E ), //LED 1
  ( 1<<LINE_C | 1<<LINE_E ), //LED 2
  ( 1<<LINE_D | 1<<LINE_E ), //LED 3
  ( 1<<LINE_E | 1<<LINE_D ), //LED 4
  
  ( 1<<LINE_A | 1<<LINE_D ), //LED 5
  ( 1<<LINE_B | 1<<LINE_D ), //LED 6
  ( 1<<LINE_C | 1<<LINE_D ), //LED 7
  ( 1<<LINE_D | 1<<LINE_C ), //LED 8
  ( 1<<LINE_E | 1<<LINE_C ), //LED 9
  
  ( 1<<LINE_A | 1<<LINE_C ), //LED 10
  ( 1<<LINE_B | 1<<LINE_C ), //LED 11
  ( 1<<LINE_C | 1<<LINE_B ), //LED 12
  ( 1<<LINE_D | 1<<LINE_B ), //LED 13
  ( 1<<LINE_E | 1<<LINE_B ), //LED 14
  
  ( 1<<LINE_A | 1<<LINE_B ), //LED 15
  ( 1<<LINE_B | 1<<LINE_A ), //LED 16
  ( 1<<LINE_C | 1<<LINE_A ), //LED 17
  ( 1<<LINE_D | 1<<LINE_A ), //LED 18
  ( 1<<LINE_E | 1<<LINE_A ) //LED 19
};

//PORTD output config for each LED (1 = High, 0 = Low)
const char led_out[20] = {
  ( 1<<LINE_A ), //LED 0
  ( 1<<LINE_B ), //LED 1
  ( 1<<LINE_C ), //LED 2
  ( 1<<LINE_D ), //LED 3
  ( 1<<LINE_E ), //LED 4
  
  ( 1<<LINE_A ), //LED 5
  ( 1<<LINE_B ), //LED 6
  ( 1<<LINE_C ), //LED 7
  ( 1<<LINE_D ), //LED 8
  ( 1<<LINE_E ), //LED 9
  
  ( 1<<LINE_A ), //LED 10
  ( 1<<LINE_B ), //LED 11
  ( 1<<LINE_C ), //LED 12
  ( 1<<LINE_D ), //LED 13
  ( 1<<LINE_E ), //LED 14
  
  ( 1<<LINE_A ), //LED 15
  ( 1<<LINE_B ), //LED 16
  ( 1<<LINE_C ), //LED 17
  ( 1<<LINE_D ), //LED 18
  ( 1<<LINE_E ) //LED 19
};

void light_led(char led_num) { //led_num must be from 0 to 19
  //DDRD is the ports in use
  DDRD = led_dir[led_num];
  PORTD = led_out[led_num];
}

void leds_off() {
  DDRD = 0;
  PORTD = 0;	
}

void draw_frame(void){
  char led, bright_val, b;
  for ( led=0; led<=19; led++ ) {
    //software PWM
    bright_val = led_grid[led];
    // A little explanation here: If bright_val is 50, the LED will be lit 50% of the time by the following two lines.
    for( b=0 ; b < bright_val ; b+=1) { light_led(led); } //delay while on
    for( b=bright_val ; b<100 ; b+=1 ) { leds_off(); } //delay while off
  }
}

void fade_to_next_frame(void){
  char led, changes;	

  while(1){
    changes = 0;
    for ( led=0; led<=19; led++ ) {
      if( led_grid[led] < led_grid_next[led] ){ 
        led_grid[led] = (led_grid[led]+1); 
        changes++; 
      }
      if( led_grid[led] > led_grid_next[led] ){ 
        led_grid[led] = (led_grid[led]-1); 
        changes++; 
      }
    }
    draw_frame();
    if( changes == 0 ){
      break;
    }
  }
}

