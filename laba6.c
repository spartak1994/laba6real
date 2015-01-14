#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
struct thread_attrs{
pthread_mutex_t *mutex_ptr;
int shmID;
};
void getTime(struct tm *current_time)
{
time_t sec = 0;
time(&sec);
*current_time = *(localtime(&sec));
}
void *thread_func(void *attr)
{
struct thread_attrs *data;
data = (struct thread_attrs *)attr;
pthread_mutex_lock( data->mutex_ptr );
sleep(3);
void *addr;
addr = shmat(data->shmID, NULL, 0);
if( addr == (void *)(-1) ){
printf("Can't attach SR MEM.\n");
exit(-1);
}
struct tm time;
getTime(&time);
printf("Time in second thread %d:%d:%d\n",time.tm_hour, time.tm_min, time.tm_sec);
*( (struct tm *)addr ) = time;
pthread_mutex_unlock( data->mutex_ptr );
}
int main(int argc, char const *argv[])
{
struct tm time;
int shmid;
void *addr;
pthread_t tid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
if( pthread_mutex_init(&mutex, NULL) != 0 ){
printf("Can't create mutex.\n");
return -1;
}
shmid = shmget(IPC_PRIVATE, sizeof(struct tm), IPC_CREAT | IPC_EXCL | 0666);
if(shmid == -1){
printf("Can't create SR MEM.\n");
return -1;
}
addr = shmat(shmid, NULL, 0);
if( addr == (void *)(-1) ){
printf("Can't attach SR MEM.\n");
return -1;
}
struct thread_attrs attr;
attr.mutex_ptr = &mutex;
attr.shmID = shmid;
if( pthread_create(&tid, NULL, thread_func, &attr) == 0 ){
getTime(&time);
printf("Time in main thread %d:%d:%d\n", time.tm_hour, time.tm_min, time.tm_sec);
sleep(1);
while( pthread_mutex_trylock(&mutex) != 0 ){
printf("I'm still waiting!\n");
sleep(1);
}
time = *( (struct tm *)addr );
printf("Time from SR MEM %d:%d:%d\n", time.tm_hour, time.tm_min, time.tm_sec);
shmdt(addr);
}
else{
printf("Can't create thread.\n");
return -1;
}
return 0;
}
