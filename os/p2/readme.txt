Project 2 
Batuhan Başkaya 64240

compilation:
gcc -o args args.c -lpthread -g

running with given parameters (seed as 12345):
./args 4 5 75 3 12345

I did not implement part 2.

My program starts by reading the arguments. The program must be given 5 arguments in this kind:
Need arguments like: ./args [n] [q] [p] [t] [seed]
n=# of speakers
q=# of questions
p=probability of answering. for example if you give 20, there will be %20 chance of answering
t=max speech duration
seed=for rand()
All arguments should be integers larger than 0 in order for program to work as intended.

The program then initializes mutexes and condition variables. Then it creates n speaker threads and a moderator thread. The speaker threads wait on a condition variable. The moderator thread asks the question, broadcasts on the condition variable that the speakers are waiting, then waits on a different condition variable. The speaker threads stop waiting on their condition variable since they were broadcasted, and they all try to acquire a mutex. The speaker that got the mutex answers, releases the mutex and waits on the condition variable again. Also the speaker increments a counter to check if it was the last speaker. One by one, each speaker answers, and the last speaker signals the moderator, and waits for the other question. This repeats for every question.
