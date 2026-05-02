#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

typedef struct {
  int number;
  char data[256];
  int ready; // 0 = empty, 1 = frame waiting
} Frame;

Frame buffer;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile int running = 1;

void sig_handler(){
  running = 0; // changes the flag to 0 for the loop to stop
  pthread_cond_broadcast(&cond); // wakes up the thread so they checks the running and exit
}
void *producer(void *arg){
  int frame_num = 0;
  while(running){
    pthread_mutex_lock(&mutex);
    buffer.number = ++frame_num;
    snprintf(buffer.data,256,"JPEG DATA %d",frame_num);
    buffer.ready=1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    usleep(100000);
  }
  return NULL;
}
void *consumer(void *args){
  while(running) {
    pthread_mutex_lock(&mutex);
    while(buffer.ready == 0 && running)
      pthread_cond_wait(&cond, &mutex);
    printf("Frame %d: %s\n", buffer.number, buffer.data);
    buffer.ready = 0;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main(void){
    pthread_t t1,t2;
    
    int s;

    s= pthread_create(&t1,NULL,producer,NULL);
    if(s!=0){
      printf("Error: cant create thread producer\n");
      goto exit;
    }
    s = pthread_create(&t2,NULL,consumer,NULL);
    if(s!=0){
      printf("Error: cant create thread consumer\n");
      goto exit;
    }

    signal(SIGINT,sig_handler);

    s = pthread_join(t1, NULL);
    if (s != 0)
      printf("Error: some error\n");

    s = pthread_join(t2, NULL);
    if (s != 0)
      printf("Error: some error\n");
    printf("program safely exited!!");
    return 0;

exit:
    return -1;
}