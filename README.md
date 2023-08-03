1. Describe how your exchange works.
Program requires setup. program forks then launches each trader successively. All pipes are confirmed to be connected to each trader. After this the program will enter the main stage of the program.

When not processing orders/disconnects my program is waiting for signals. when a signals detected it will then perform the action necessary to deal with it. If a SIGCHLD is received program will identify trader and disconnect. In the case of SIGUSR1 the program will identify trader via pid, read relevant pipe from trader and take necessary steps to process order depending on type. If it is a buy/sell order it will be see if a match is available according to the price-time algorithm as described in the notification.  

Whenever a trader disconnects, program will check to the number of active traders left, if 0, program will free all allocated memory and close. 

2. Describe your design decisions for the trader and how it's fault-tolerant.

Since it is known that signals are unreliable I used epoll to make sure that any disconnects from the system didn't go unnoticed by the exchange. This is to ensure that the program knows when to terminate, even if the traders themselves don't notify the exchange that they are doing so. (the flag used for epoll is the EPOLLUP flag which means that epoll essentially only monitors for when the traders close their end of the pipes). 

Furthermore I employed linkedlists throughout my program instead of malloced arrays due to the ease in which I can add and delete 

I constructed a function called free_all, essentially, any exit out of the program (either through an error or by planned termination) the program will call free_all() to free all dynamically allocated memory items before terminating the program. This is to minimise the possibility of my program having memory leaks in it. 

3. Describe your tests and how to run them.

The make file has been modified to allow it to be tested by using the make file. simply type "make test" into command line. This will cause a bash file to execute with tests inside. After the compilation of the code is printed the results of the testcases will appear. before each test case you will see a title:
"test <test number>"
followed by 2 equals bars. 
and a note saying the test is finished. 
e.g. 
test 1
==========================
==========================
test 1 finished

if there is anything printing inside between the '=' bars than that means that the test had failed and that something has gone wrong, if it appears as the example than the test has been passed. 

My tests focus on the startup phase to see if the pipes are properly activated and connected to their coresponding traders.