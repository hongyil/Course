
/* Name:Hongyi Liang
 * Andrew ID:hongyil
 *
 * tsh - A tiny shell program with job control
 * The shell simulated the behaviors of some built-in commands and execute them or run
 * the provided tracejob(and the child process) based on the input job id or pid in the command line.
 *
 * There are two modes: background/foreground for our jobs to run,and the built-in
 * commands are associated with those jobs.
 *
 * Also some shell-specified signal handlers are rewritten, to understand how signal
 * works,and handle the signals like SIGINT,SIGTSTP.When the child process terminate,we 
 * also need to deal with SIGCHLD signal.
 * 
 * Follow the 15-213/18-213/15-513 style guide at
 * http://www.cs.cmu.edu/~213/codeStyle.html.>
 */

#include "tsh_helper.h"

/*
 * If DEBUG is defined, enable contracts and printing on dbg_printf.
 */
#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif

/* Function prototypes */
void eval(const char *cmdline);
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void sigquit_handler(int sig);
void list_jobs(struct cmdline_tokens *token);
void run_job(struct cmdline_tokens *token,char mode[2]);
bool builtin_command(struct cmdline_tokens *token);
bool is_valid_pid(pid_t pid);


/*
 *  The main function will parse the command line arguments, restore the 
 *  signal handler and provie the entry to execute our shell program.
 *  
 * "Each function should be prefaced with a comment describing the purpose
 *  of the function (in a sentence or two), the function's arguments and
 *  return value, any error cases that are relevant to the caller,
 *  any pertinent side effects, and any assumptions that the function makes."
 */
int main(int argc, char **argv){
    char c;
    char cmdline[MAXLINE_TSH];  // Cmdline for fgets
    bool emit_prompt = true;    // Emit prompt (default)

    // Redirect stderr to stdout (so that driver will get all output
    // on the pipe connected to stdout)
    Dup2(STDOUT_FILENO, STDERR_FILENO);

    // Parse the command line
    while ((c = getopt(argc, argv, "hvp")) != EOF)
    {
        switch (c)
        {
        case 'h':                   // Prints help message
            usage();
            break;
        case 'v':                   // Emits additional diagnostic info
            verbose = true;
            break;
        case 'p':                   // Disables prompt printing
            emit_prompt = false;  
            break;
        default:
            usage();
        }
    }

    // Install the signal handlers
    Signal(SIGINT,  sigint_handler);   // Handles ctrl-c
    Signal(SIGTSTP, sigtstp_handler);  // Handles ctrl-z
    Signal(SIGCHLD, sigchld_handler);  // Handles terminated or stopped child

    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    Signal(SIGQUIT, sigquit_handler); 

    // Initialize the job list
    initjobs(job_list);

    // Execute the shell's read/eval loop
    while (true)
    {
        if (emit_prompt)
        {
            printf("%s", prompt);
            fflush(stdout);
        }

        if ((fgets(cmdline, MAXLINE_TSH, stdin) == NULL) && ferror(stdin))
        {
            app_error("fgets error");
        }

        if (feof(stdin))
        { 
            // End of file (ctrl-d)
            printf ("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }
        
        // Remove the trailing newline
        cmdline[strlen(cmdline)-1] = '\0';
        
        // Evaluate the command line
        eval(cmdline);
        
        fflush(stdout);
    } 
    
    return -1; // control never reaches here
}


/* Handy guide for eval:
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg),
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.
 * Note: each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */

/* 
 * eval() function:
 * Main routine that parses and interprets the command line
 * if built-in command:execute it
 * if not built-in command:fork a child
 * use sigprocmask() to block SIGCHLD SIGINT SIGTSTP
 * fork() and check I/O redirection
 * after fork(),before Execve(),setpgid(0,0)
 * use Sigsuspend() to wait until foreground job is done
 *
 * reference: csapp textbook 2e/3e and ppt from class
 */
void eval(const char *cmdline){

    int fd;
    int jid;
    int state; //job state
    pid_t pid;
    struct cmdline_tokens token;
    sigset_t ourmask,prev_one;
    parseline_return parse_result; 

    // Parse command line
    parse_result = parseline(cmdline, &token);

    //ignore empty or error lines,store the parse_result
    if(parse_result == PARSELINE_ERROR || parse_result == PARSELINE_EMPTY)
    {
        return;
    }else if(parse_result==PARSELINE_FG){
        state=FG;
    }else{
        state=BG;
    }

    Sigemptyset(&ourmask);
    Sigaddset(&ourmask,SIGCHLD);
    Sigaddset(&ourmask,SIGINT);
    Sigaddset(&ourmask,SIGTSTP);    
    Sigprocmask(SIG_BLOCK,&ourmask,&prev_one);

    //not built-in cmd,fork a child
    if(!builtin_command(&token)){
        if((pid=Fork())==0){
            Setpgid(0,0);
            Sigprocmask(SIG_UNBLOCK,&ourmask,NULL);
            // I/O redirection
            if(token.outfile){
                fd=Open(token.outfile,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                Dup2(fd,1);
                Close(fd);
            }
            if(token.infile){
                fd=Open(token.infile,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                Dup2(fd,0);
                Close(fd);
            }
            if(execve(token.argv[0],token.argv,environ)<0){
                printf("%s: Command not found\n",token.argv[0]);
                return;
            }       
        }

        addjob(job_list,pid,state,cmdline);

        if(state==FG){
            while(pid==fgpid(job_list)){
                Sigsuspend(&prev_one);
            }
        }else{
            jid=pid2jid(job_list,pid);
            printf("[%d] (%d) %s\n", jid,pid,cmdline);
        }       
    }
    Sigprocmask(SIG_UNBLOCK, &ourmask, NULL);  
    return;
}


/*
 * check if built-in command and execute it
 * quit:terminates the shell
 * jobs:lists all background jobs
 * bg:sending a SIGCONT signal,run in background
 * fg:sending a SIGCONT signal,run in foreground
 */
bool builtin_command(struct cmdline_tokens *token){

    bool is_builtin=true;

    switch (token->builtin){    
    case BUILTIN_QUIT:
        exit(0);
        break;    
    case BUILTIN_JOBS:
        list_jobs(token);
        break;    
    case BUILTIN_BG:
        run_job(token,"bg");
        break;
    case BUILTIN_FG:
        run_job(token,"fg");
        break;
    //case none:do nothing
    case BUILTIN_NONE:
        ;
    //not built-in command
    default:
        is_builtin=false;
    }

    return is_builtin;
}

//command jobs: to list all bg jobs
void list_jobs(struct cmdline_tokens *token){

    int fd;

    if (token->outfile==NULL){
        listjobs(job_list,0);
    }else{
        fd=open(token->outfile,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        listjobs(job_list,fd);
        Close(fd);
    }
    
    return;
}

//check if valid pid
bool is_valid_pid(pid_t pid) {

    int new_jid;
    bool valid=true;

    new_jid=pid2jid(job_list,pid);

    if (new_jid==0){
        valid=false;
    }

    return valid;
}

/* 
 * id pattern: ''pid'' or ('%'+'jid'),use isdigit() to recognize the id
 * find a job_list by pid or jid
 * send a SICONT signal and run in background/foreground
 * use -pid to kill all the foreground jobs in Kill()
 * use Sigsuspend() to wait until bg job terminates
 */
void run_job(struct cmdline_tokens *token,char mode[2]){

    int jid;
    pid_t pid;
    struct job_t *jl;
    char *tmp=token->argv[1];
    sigset_t mask_one;

    if ((token->argc)<2){
        Sio_puts("not enough arguments\n");
        return;
    }

    //get the job by pid or jid
    if (isdigit(token->argv[1][0])!=0){
        pid=atoi(tmp);
        if (!is_valid_pid(pid)){
            Sio_puts("Error: not valid pid\n");
            return;
        }
        jl=getjobpid(job_list,pid);
        if (jl==NULL){
            Sio_puts("Error: job not found\n");
            return;
        }
        jid=pid2jid(job_list,pid);
    }else if (token->argv[1][0]=='%'){
        if (isdigit((token->argv[1][1]))!=0){
            //points to the the element right after '%'
            jid=atoi(&tmp[1]);
            jl=getjobjid(job_list,jid);
            if (jl==NULL){
                Sio_puts("Error:job not found\n");
                return;
            }
        }
    }else{
        Sio_puts("Error: Invalid pid or jid\n");
        return;
    }

    pid=jl->pid;
    Kill(-pid,SIGCONT);

    if (!is_valid_pid(pid)){
        Sio_puts("Not valid pid!\n");
        return;
    } 

    if (strcmp(mode,"bg")==0){
        jl->state=BG;
        Sio_puts("[");
        Sio_putl(jl->jid);
        Sio_puts("] (");
        Sio_putl((jl->pid));   
        Sio_puts(")");
        Sio_puts(jl->cmdline);
        Sio_puts("\n");             
    }else{
        jl->state=FG;
        while(pid==fgpid(job_list)){
            Sigsuspend(&mask_one);
        }
    }

    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler()-catches SIGCHILD signals
 * handle signals when child process terminates
 * block the signals first
 * child inherit signal handler
 * need to handle SIGINT and SIGTSTP
 * use -pid to kill all the foreground jobs in Kill()
 * 
 */
void sigchld_handler(int sig){

    int jid;
    int status;  
    int olderrno=errno;
    pid_t pid;
    struct job_t *jl=NULL;
    sigset_t ourmask;

    Sigemptyset(&ourmask);
    Sigaddset(&ourmask, SIGCHLD);
    Sigaddset(&ourmask, SIGINT);
    Sigaddset(&ourmask, SIGTSTP);
    Sigprocmask(SIG_BLOCK, &ourmask, NULL);

    pid=fgpid(job_list);
    jid=pid2jid(job_list, pid);
    jl=getjobpid(job_list,pid);

    if (verbose){
        Sio_puts("Caught SIGCHLD!\n");
        return;
    }

    while((pid=waitpid(-1,&status,WUNTRACED|WNOHANG))>0){ 
        //WUNTRACED->WIFSTOPPED, SIGTSTP         
        if(WIFSTOPPED(status)){  
            Sio_puts("Job [");
            Sio_putl(jid);
            Sio_puts("] (");
            Sio_putl(pid);
            Sio_puts(") stopped by signal 20\n");
            jl->state=ST;
            Kill(-pid,sig);
        }else if(WIFSIGNALED(status)){ //stopped by SIGINT
            Sio_puts("Job [");
            Sio_putl((long)jid);
            Sio_puts("] (");
            Sio_putl((long)pid);
            Sio_puts(") terminated by signal 2\n");
            deletejob(job_list,pid);
        }else{
            deletejob(job_list,pid);
        }
    }

    Sigprocmask(SIG_UNBLOCK,&ourmask,NULL); 
    errno=olderrno; 
    return;
}

/* 
 * sigint_handler():Catches SIGINT(ctrl-c) signals
 * block the signals first
 * use -pid to kill all the foreground jobs in Kill()
 */
void sigint_handler(int sig){

    int olderror=errno;
    pid_t pid;
    sigset_t ourmask;

    Sigemptyset(&ourmask);
    Sigaddset(&ourmask,SIGCHLD);
    Sigaddset(&ourmask,SIGINT);
    Sigaddset(&ourmask,SIGTSTP);    
    Sigprocmask(SIG_BLOCK,&ourmask,NULL);

    pid=fgpid(job_list);

    if (verbose){
        Sio_puts("Caught SIGINT!\n");
        return;
    }  

    if (!is_valid_pid(pid)){
        return;
    }else{
        Kill(-pid,sig);
    }

    Sigprocmask(SIG_UNBLOCK,&ourmask,NULL);
    errno=olderror;
    return;
}

/*
 * Catches SIGTSTP (ctrl-z) signals
 * block the signals first
 * use -pid to kill all the foreground jobs in Kill()
 */
void sigtstp_handler(int sig){

    pid_t pid;
    int olderror=errno; 
    sigset_t ourmask;

    Sigemptyset(&ourmask);
    Sigaddset(&ourmask,SIGCHLD);
    Sigaddset(&ourmask,SIGINT);
    Sigaddset(&ourmask,SIGTSTP);    
    Sigprocmask(SIG_BLOCK,&ourmask,NULL);

    pid=fgpid(job_list);

    if (verbose){
        Sio_puts("Caught SIGTSTP!\n");
        return;
    }

    if (!is_valid_pid(pid)){
        return;
    }else{
        Kill(-pid,sig); 
    }

    Sigprocmask(SIG_UNBLOCK,&ourmask,NULL);
    errno=olderror;
    return;
}

