#include<stdio.h>
#include"pico/stdlib.h"
#include"hardware/watchdog.h"
#include<string>

using namespace std;
//last modified 10 march 2023

#define h 1
#define l 0

#define resolutionB 1
#define resolutionU 3

#define uart_id uart1
#define baud_rate 921600
#define uart_tx 4
#define uart_rx 5


int rightDelay=3000;
int leftsDelay =1800;
int writeDelay=40;
  
int width=0;
int height=0;

const int a=7,b=9,c=6,d=8; //for feeding paper
const int p=10,q=11,r=12,s=13;   //for moving writer head
const int writer=14;    // for write head
const int led=25;       // led for visual response

int leftStepNo=1;//, rightStepNo=1;       //keep track of step number in head motor
int stepU=0;

void pulseLed(int ,int ,int );// for visual resposne
void handshake();             // handshaking with the android app before further data transfer
void setParm();               // sets various parameters like height, width, write delay etc
void draw();                  // controlls the whole printing process
void dw(int,bool);            // just renaming gpio_put(int,bool) to dw 
void moveHeadRight(int);      // moves head RIGHT
void moveHeadLeft(int);       // moves head LEFT
void feedPaper(int);          // feeds the paper using unipolar stepper motor
string byteToPixels(int);     // converts BYTES to pixles
void writeHead();             // the printhead


///////////////////////////////////////// main() 
int main(){
  stdio_init_all(); 
  uart_init(uart_id,baud_rate);
  gpio_set_function(uart_tx,GPIO_FUNC_UART);
  gpio_set_function(uart_rx,GPIO_FUNC_UART);
  // uart_set_hw_flow(uart_id, false, false); 
  // uart_set_fifo_enabled(uart_id, false);
  gpio_init(led);
  gpio_set_dir(led,GPIO_OUT); 
  
  for(int i=6;i<=14;i++){
    gpio_init(i);
    gpio_set_dir(i,GPIO_OUT);
    dw(i,0);
 
  }

  pulseLed(2000,400,1);
  handshake();
  setParm();
  draw(); 
 
  return 0;
}
///////////////////////////////// for visual resposne for various steps we are using led pulses
void pulseLed(int highDuration,int lowDuration,int cycles){
  for(int i=0;i<cycles;i++){
    gpio_put(led,1);
    sleep_ms(highDuration);
    gpio_put(led,0);
    sleep_ms(lowDuration);
  }
}
////////////////////////////////////////// handshaking with the android app before further data transfer
void handshake(){
  while(1){
      pulseLed(50,300,1);
      if(uart_is_readable(uart_id)&&uart_getc(uart_id)=='a'){
        uart_putc(uart_id,'a');
        break;
      }
  }
}
////////////////////////////////////////// sets various parameters like height, width, write delay etc
void setParm(){

  while(1){
    if(uart_is_readable(uart_id)){
      width= uart_getc(uart_id);
      break;
    }
  }
  while(1){
    if(uart_is_readable(uart_id)){
      height= uart_getc(uart_id)*8;
      break;
   }
  }
  
  while(1){
    if(uart_is_readable(uart_id)){
      height+= uart_getc(uart_id);
      break;
   }
  }
  
  while(1){
    if(uart_is_readable(uart_id)){
      rightDelay= uart_getc(uart_id)*100;
      break;
   }
  }

  while(1){
    if(uart_is_readable(uart_id)){
      leftsDelay = uart_getc(uart_id)*100;
      break;
   }
  }

  while(1){
    if(uart_is_readable(uart_id)){
      writeDelay= uart_getc(uart_id)*10;
      break;
    }
  }
  pulseLed(50,200,10);
}
///////////////////////////////////////// controlls the whole printing process
void draw(){
  string pxl;
  int index=0;
  char *line=new char(width*height);
  uart_putc(uart_id,'k'); //sending ASCII value of k as positive acknowledgement
  sleep_ms(1);
 
  while(index<width*height){
    if(uart_is_readable(uart_id))
      line[index++]=uart_getc(uart_id);  //read the whole image data sent from phone into byte[]
  }
  feedPaper(100);                        // to load the paper sheet into the dotter
  moveHeadRight(resolutionB*width*8);
  sleep_ms(10);
  moveHeadLeft(resolutionB*width*8);
 
  for(int p=0;p<height;p++){
    feedPaper(resolutionU);
    if(uart_is_readable(uart_id) && uart_getc(uart_id)==10)
      break;
    sleep_ms(8);

    for(int i=0;i<width;i++){
      pxl=byteToPixels(line[i+width*p]);

      for(int k=0;k<8;k++){
        if(pxl[k]=='1')
          writeHead();
        moveHeadRight(resolutionB);
       }
    }
    
  moveHeadLeft(resolutionB*width*8);
  sleep_ms(10);
  }

dw(p,l);
dw(q,l);
dw(r,l);
dw(s,l);

pulseLed(3000,1,1);
sleep_ms(1000);
watchdog_enable(1,1); /////////////////// reset pico
}
///////////////////////////////////////// just renaming gpio_put(int,bool) to dw 
void dw(int pin,bool level){
  gpio_put(pin,level);
}
///////////////////////////////////////// moves head RIGHT
void moveHeadRight(int n){
  while(n>0){
    n--;
    switch(leftStepNo){
    case 1:
        dw(p,h);//#1
        dw(q,l);
        dw(r,l);
        dw(s,l);
        break;
        
      case 2:
        dw(p,h);//#2
        dw(q,l);
        dw(r,h);
        dw(s,l);
        break;

      case 3:
        dw(p,l);//#3
        dw(q,l);
        dw(r,h);
        dw(s,l);
        break;
        
      case 4:
        dw(p,l);//#4
        dw(q,h);
        dw(r,h);
        dw(s,l);
        break;
        
      case 5:
        dw(p,l);//#5
        dw(q,h);
        dw(r,l);
        dw(s,l);
        break;
        
      case 6:
        dw(p,l);//#6
        dw(q,h);
        dw(r,l);
        dw(s,h);
        break;
        
      case 7:
        dw(p,l);//#7
        dw(q,l);
        dw(r,l);
        dw(s,h);
        break;
        
      case 8:
        dw(p,h);//#8
        dw(q,l);
        dw(r,l);
        dw(s,h);
        leftStepNo=0;
        break;
    }
    sleep_us(rightDelay);
    leftStepNo++;    
  }
 
}
///////////////////////////////////////// moves head LEFT
void moveHeadLeft(int n){
  sleep_ms(40);
  leftStepNo-=2;
  if(leftStepNo==0)
    leftStepNo=8;
  else if(leftStepNo==-1)
    leftStepNo=7;
    
  while(n>0){
    n--;
    switch(leftStepNo){
    case 1:
        dw(p,h);//#1
        dw(q,l);
        dw(r,l);
        dw(s,l);
        leftStepNo=9;
        break;
        
      case 2:
        dw(p,h);//#2
        dw(q,l);
        dw(r,h);
        dw(s,l);
        break;

      case 3:
        dw(p,l);//#3
        dw(q,l);
        dw(r,h);
        dw(s,l);
        break;
        
      case 4:
        dw(p,l);//#4
        dw(q,h);
        dw(r,h);
        dw(s,l);
        break;
        
      case 5:
        dw(p,l);//#5
        dw(q,h);
        dw(r,l);
        dw(s,l);
        break;
        
      case 6:
        dw(p,l);//#6
        dw(q,h);
        dw(r,l);
        dw(s,h);
        break;
        
      case 7:
        dw(p,l);//#7
        dw(q,l);
        dw(r,l);
        dw(s,h);
        break;
        
      case 8:
        dw(p,h);//#8
        dw(q,l);
        dw(r,l);
        dw(s,h);
        break;
    }
    sleep_us(leftsDelay);
    leftStepNo--;    
  }
  leftStepNo+=2;
  if(leftStepNo==10)
    leftStepNo=2;
  else if(leftStepNo==9)
    leftStepNo=1;
  dw(p,l);
  dw(q,l);
  dw(r,l);
  dw(s,l);
  sleep_ms(40);
}
///////////////////////////////////////// feeds the paper using unipolar stepper motor
void feedPaper(int n){
  int t=4;
  while(n>0){
    n--;
    switch(stepU){
      case 0:
        dw(a,1);
        dw(b,0);
        dw(c,0);
        dw(d,0);
        break;

      case 1:
        dw(a,0);
        dw(b,1);
        dw(c,0);
        dw(d,0);
        break;

      case 2:
        dw(a,0);
        dw(b,0);
        dw(c,1);
        dw(d,0);
        break;

      case 3:
        dw(a,0);
        dw(b,0);
        dw(c,0);
        dw(d,1);
        stepU=-1;
        break;
    }
    stepU++;
    sleep_ms(t);
  }

  dw(a,0);
  dw(b,0);
  dw(c,0);
  dw(d,0);
  pulseLed(10,20,2);
  //sleep_ms(t);  
}
///////////////////////////////////////// GET PIXELS FROM RECEIVED BYTES
string byteToPixels(int input){
  int m=128;
  string pxl="00000000";
  for(int i=0;i<8;i++){
    
    if(input>=m){
      pxl[i]='1';
      input-=m;
    }
    m/=2;
  }
  return pxl;
}
///////////////////////////////////////// the printhead
void writeHead(){ //to print dot on sheet NOTE: writer is also active low
    sleep_ms(3);  
    dw(p,0);
    dw(q,0);
    dw(r,0);
    dw(s,0);

    dw(writer,1);
    sleep_us(writeDelay);   
    dw(writer,0);
    sleep_ms(2);
}
