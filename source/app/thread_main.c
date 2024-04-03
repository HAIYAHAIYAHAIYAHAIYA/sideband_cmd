// #include <unistd.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <stdio.h>
// #include <signal.h>

 
// pthread_mutex_t mutex;//自旋锁
// pthread_cond_t cond1,cond2;
// //线程1
// void *thread_fun1(void * arg)
// {
// 	int i=0;
// 	while(1) {
// 		pthread_mutex_lock(&mutex);//加锁

// 		pthread_cond_signal(&cond2);//唤醒另一个
// 		pthread_cond_wait(&cond1,&mutex);//等待被唤醒
// 		pthread_mutex_unlock(&mutex);//解锁
// 		sleep(1);
// 	}
// 	return NULL;
// }
 
// void *thread_fun2(void * arg)
// {
// 	int i=1;
// 	while(1) {
// 		pthread_mutex_lock(&mutex);//加锁

// 		pthread_cond_signal(&cond1);//唤醒另一个
// 		pthread_cond_wait(&cond2,&mutex);//等待被唤醒
// 		pthread_mutex_unlock(&mutex);//解锁
//         sleep(1);
// 	}
// 	return NULL;
// } 
 
// int main()
// {
// 	//创建线程
// 	pthread_t pth1,pth2;
// 	pthread_create(&pth1,NULL,thread_fun1,NULL);
// 	pthread_create(&pth2,NULL,thread_fun2,NULL);

// 	//回收线程和自旋锁
// 	pthread_join(pth1,NULL);
// 	pthread_join(pth2,NULL);
//     pthread_mutex_init(&mutex,NULL); //初始化lock这个锁	
// 	// pthread_mutex_destroy(&mutex);
// 	while(1);
// 	return 0;
// }