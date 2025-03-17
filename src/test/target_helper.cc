//standard template library
#include <optional>
#include <string>
#include <sstream>

//system headers
#include <unistd.h>
#include <signal.h>
#include <linux/limits.h>
#include <sys/wait.h>

//external libraries
#include <doctest/doctest.h>

//local headers
#include "target_helper.hh"

//test target headers
#include "../lib/scancry.h"



//globals
static enum target_map_state target_state;


//signal handlers
static void _sigusr1_handler(int signal) {

    /*
     *  We need to wait for the child process to complete initialisation
     *  before continuing the test. The child will send a SIGUSR1 signal
     *  once its initialisation is finished.
     */

    //update target state to set
    target_state = INIT;

    //std::cerr << "<!> received signal from unit_target.\n";
    return;
}


//helpers
std::optional<int> clean_targets() {

    int ret;

    //build the kill command    
    std::stringstream command_ss;
    command_ss << "kill $(pidof " << target_name
               << ") > /dev/null 2> /dev/null";

    //use system() to kill all existing targets
    ret = system(command_ss.str().c_str());
    if (ret == -1) return std::nullopt;

    return 0;
}


pid_t start_target() {

    int ret;
    __sighandler_t ret_s;

    pid_t target_pid, parent_pid;
    char pid_buf[8];
    
    char * argv[3] = {(char *) target_name, 0, 0};
    enum target_map_state old_state;


    //setup initial state
    target_state = old_state = UNINIT;

    //get current pid to pass to target
    parent_pid = getpid();
    std::string pid_str = std::to_string(parent_pid);
    argv[1] = (char *) pid_str.c_str();

    //register signal handler
    ret_s = signal(SIGUSR1, _sigusr1_handler);
    CHECK_NE((void *) ret_s, (void *) SIG_ERR);

    //fork a new process
    target_pid = fork();
    CHECK_NE(target_pid, -1);

    //change image to target in child
    if (target_pid == 0) {
        ret = execve(target_name, argv, NULL);
        CHECK_NE(ret, -1);

    //parent waits for child to complete initialisation
    } else {
        while (target_state == old_state) {}
    }
    
    return target_pid;
}


void end_target(pid_t pid) {

    int ret;
    __sighandler_t ret_s;

    pid_t ret_p;

    
    //unregister signal handler
    ret_s = signal(SIGUSR1, SIG_DFL);

    //terminate target process
    ret = kill(pid, SIGTERM);
    CHECK_EQ(ret, 0);

    //wait for it to terminate
    ret_p = waitpid(pid, NULL, 0);
    CHECK_EQ(ret, 0);

    return;
}
