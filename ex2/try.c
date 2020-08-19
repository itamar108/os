


int main()
{
    struct itimerval timer;

    int nums[5] = {1,2,3,4,5};



    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = 3;	// following time intervals, seconds part
    timer.it_interval.tv_usec = 0;	// following time intervals, microseconds part



    for (int i = 0; i < 5; ++i) {
        timer.it_value.tv_sec = i;		// first time interval, seconds part
        timer.it_value.tv_usec = 0;		// first time interval, microseconds part


        setitimer(ITIMER_VIRTUAL, &timer, NULL);
    }
}