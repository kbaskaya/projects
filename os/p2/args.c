#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

int pthread_sleep(double seconds){
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL)){
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

struct timeval st;
pthread_t tid[1024] = {0};
pthread_mutex_t mutexspeak;
pthread_mutex_t mutexcounter;
pthread_cond_t cond1;
pthread_cond_t cond2;
sem_t semaphore1;

int q=5;
int n=5;
int p=30;
int t=5;
int seed=123;
int counter=0;

struct targ{
	int mid;
	int probability;
};
void *tfunc (void* id){  //speaker thread
	pthread_mutex_t mutexc1;
	if(pthread_mutex_init(&mutexc1,NULL)!=0)printf("MUTEXC1 FAILED\n");
	int i=0;
	int mid=(int)id;
	long int min=0;
	long int msec=0;
	long int sec=0;
	struct timeval tv;
	for(i=0;i<q;i++){
		//printf("SPEAKER THREAD %d WAITING QUESTION %d.\n",mid,i+1);
		pthread_mutex_lock(&mutexc1);
		pthread_cond_wait(&cond1,&mutexc1);
		pthread_mutex_unlock(&mutexc1);
		//printf("SPEAKER THREAD %d RECEIVING QUESTION %d.\n",mid,i+1);
		if((rand()%100)<p){
			pthread_mutex_lock(&mutexspeak);
			gettimeofday(&tv,NULL);
			msec=tv.tv_usec-st.tv_usec;
			sec=tv.tv_sec-st.tv_sec;
			if(msec<0){
				msec=st.tv_usec-tv.tv_usec;
				sec--;
			}
			if(sec>59){
				sec=sec%60;
				min++;
			}
			double rtime = ((double) rand() / RAND_MAX) * (t - 1) + 1;
			printf("[%ld:%ld.%ld] Commentator #%d generates response for %f second\n",min,sec,msec,mid,rtime);
			pthread_sleep(rtime);
			pthread_mutex_unlock(&mutexspeak);
		}
		//printf("SPEAKER THREAD %d LOCKING +COUNTER FOR %d.\n",mid,i+1);
		pthread_mutex_lock(&mutexcounter);
		//printf("SPEAKER THREAD %d +COUNTERING FOR %d.\n",mid,i+1);
		counter++;
		//printf("counter: %d\n",counter);
		if (counter==n){
			counter=0;
			pthread_cond_signal(&cond2);
			//printf("signaling mod.\n");
		}
		pthread_mutex_unlock(&mutexcounter);
		//printf("SPEAKER THREAD %d END OF CYCLE %d\n",mid,i+1);
	}
	
	pthread_sleep(4);
	//printf("SPEAKER THREAD %d EXITING.\n",mid);
	//pthread_exit(0);
	return 0;
}
void *mfunc (void* id){  //moderator thread
	pthread_mutex_t mutexc2;
	if(pthread_mutex_init(&mutexc2,NULL)!=0)printf("MUTEXC1 FAILED\n");
	int i=0;
	long int min=0;
	long int msec=0;
	long int sec=0;
	int curq=0;
	int mid=(int)id;
	struct timeval tv;
	gettimeofday(&st,NULL);
	while(curq<q){
		//pthread_sleep(4);
		gettimeofday(&tv,NULL);
		msec=tv.tv_usec-st.tv_usec;
		sec=tv.tv_sec-st.tv_sec;
		if(msec<0){
			msec=st.tv_usec-tv.tv_usec;
			sec--;
		}
		if(sec>59){
			sec=sec%60;
			min++;
		}
		printf("[%ld:%ld.%ld] Moderator asks question %d\n",min,sec,msec,curq+1);
		pthread_cond_broadcast(&cond1);
		curq++;
		pthread_mutex_lock(&mutexc2);
		pthread_cond_wait(&cond2,&mutexc2);
		//printf("mod signaled.\n");
		pthread_mutex_unlock(&mutexc2);
	}
	return 0;
}





int main(int argc, char *argv[]){

	

	int i=0;
	if(argc!=6){
		printf("Need arguments like: ./program [n] [q] [p] [t] [seed]\nn=# of speakers\nq=# of questions\np=probability of answering such as p%\nt=max speech duration\nseed=for rand()\nAll should be larger than 0\n");
		return -1;
	}
	
	n=atoi(argv[1]);
	q=atoi(argv[2]);
	p=atoi(argv[3]);
	t=atoi(argv[4]);
	seed=atoi(argv[5]);
	srand(seed);
	
	if(pthread_mutex_init(&mutexspeak,NULL)!=0)printf("MUTEXSPEAK FAILED\n");
	if(pthread_mutex_init(&mutexcounter,NULL)!=0)printf("MUTEXCOUNTER FAILED\n");
	if(pthread_cond_init(&cond1, NULL)!=0)printf("CONDVAR1 FAILED\n");
	if(pthread_cond_init(&cond2, NULL)!=0)printf("CONDVAR2 FAILED\n");
	if(sem_init(&semaphore1, 0, 1)!=0)printf("SEMAPHORE FAILED\n");
	
 	
	for (i=0; i<n; i++){	
		pthread_create(&tid[i], NULL, tfunc, (void*) i);
	}	
	pthread_create(&tid[n], NULL, mfunc, (void*) n);
		
	
	
	
		
	for (i=0; i<n+1; i++){
		pthread_join(tid[i], NULL);
		//printf("JOINED %d\n",i);
	}
		
	return 0;
}


