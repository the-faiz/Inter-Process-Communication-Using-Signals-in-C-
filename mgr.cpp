#include<iostream>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<unistd.h>
#include<vector>
#include<time.h>
#include<cstring>

#define TableSize 11
using namespace std;

pid_t pid;

void parentsigHandler ( int sig )
{
    if(sig==SIGINT)
        kill(-pid,SIGKILL);
    else if(sig==SIGCONT)
        kill(-pid,SIGCONT);
    else if(sig==SIGTSTP)
        kill(-pid,SIGTSTP);
}



class Process{
public:
    string name;
    pid_t pid;
    pid_t group_id;
    string status;
    vector<string> args;
    Process(){}
    Process(const string& name, int pid, int group_id,const string& status,const vector<string>&args) :name(name), pid(pid),group_id(group_id),status(status),args(args){}
    friend class ProcessTable;
};



class ProcessTable{
public:
    Process PT[TableSize];
    int current_size{0};
    ProcessTable(){
        Process parent("mgr",getpid(),getgid(),"SELF",{});
        PT[0] = parent;
        current_size++;
    }
    void printTable(){
        cout<<"No \t PID \t PGID \t STATUS \t NAME"<<endl;
        for(int i=0;i<current_size;i++){
            cout<<i<<"\t"<<PT[i].pid<<"\t"<<PT[i].group_id<<"\t"<<PT[i].status<<"\t"<<PT[i].name<<endl;
        }
    }
    void addProcess(int pid, int group_id,const string& status,const string& name,const vector<string>&args){
        Process process(name,pid,group_id,status,args);
        PT[current_size] = process;
        current_size++; 
    }
};




class Manager{
private:
    ProcessTable* pt;
    void printTable(){
        pt->printTable();
    }
    void startJob(){
        if(pt->current_size==11){
            cout<<"Process Table is full. Quiting.."<<endl;
            quit();
        }
        char c = 'A' + rand() % 26;
        pid = fork();
        if(pid==0){
            setpgid(0,0); //Changing the process group id
            char* args[] = {(char*)"./job", (char*)string(1, c).c_str(), nullptr};
            execvp(args[0], args); //Takes c style strings
            exit(0);
        }else{
            string name = "Job "+string(1,c);
            waitAndUpdate(pid,name);
        }
    }
    void handleSuspendedJobs(const string& command){
        int ind;
        bool x = true;
        cout<<"Suspended Job  ";
        for(int i=0;i<pt->current_size;i++){
           if (pt->PT[i].status == "SUSPENDED"){
                cout<<i<<" ";
                x = false;
           }
        }
        if(x){cout<<"No suspended jobs found"<<endl;return;}
        cout<<"(Pick one): ";
        cin>>ind;
        pid_t pid = pt->PT[ind].pid;
        if(command=="continue") {
            kill(-pid,SIGCONT);
            waitAndUpdate(pid,ind);
        } 
        else if(command=="kill"){
            kill(-pid,SIGKILL);
            pt->PT[ind].status = "KILLED";
        }
    }
    void printHelpMessage(){
        cout<<"Commnad \t Action"<<endl;
        cout<<"c: \t Continue a Suspended Process"<<endl;
        cout<<"h: \t Print this help message"<<endl;
        cout<<"k: \t Kill a suspended process"<<endl;
        cout<<"p: \t Print the process table"<<endl;
        cout<<"q: \t quit"<<endl;
        cout<<"r: \t Run a new job"<<endl;
    }
    void quit(){
        for(int i=0;i<pt->current_size;i++){
            if (pt->PT[i].status == "SUSPENDED"){
                pid_t pid = pt->PT[i].pid;
                kill(-pid,SIGKILL);
            }
         }  
        exit(0);
    }

    void waitAndUpdate(pid_t pid,string name){
        int status;
        pid_t wpid = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        string x;
        if(WIFEXITED(status)) x = "FINISHED";
        if(WIFSIGNALED(status)) x = "TERMINATED";
        if(WIFSTOPPED(status)) x = "SUSPENDED";
        pt->addProcess(pid,pid,x,name,{});
    }
    void waitAndUpdate(pid_t pid, int ind){
        int status;
        pid_t wpid;
        string x;
        while ((wpid = waitpid(pid, &status, WUNTRACED | WCONTINUED)) > 0) {
            //Once continued change is detected and then next change is detected. That is why used here a loop rather than a single wait
            if (WIFEXITED(status)) {
                x = "FINISHED";
                break;
            } else if (WIFSTOPPED(status)) {
                x= "SUSPENDED";
                break;
            } else if(WIFSIGNALED(status)){
                x = "KILLED";
                break;
            }
        }
        pt->PT[ind].status = x;
    }

public:
    Manager(){
        pt = new ProcessTable();
    }
    void prompt(){
        cout<<"mgr>";
        char c;
        cin>>c;
        switch (c){
            case 'p':
                printTable();
                break;
            case 'r':
                startJob();
                break;
            case 'c':
                handleSuspendedJobs("continue");
                break;
            case 'k':
                handleSuspendedJobs("kill");
                break;
            case 'h':
                printHelpMessage();
                break;
            case 'q':
                quit();
                break;
            default:
                cout<<"Please enter a valid prompt"<<endl;
        }
    }
};

int main(){
    signal(SIGINT, parentsigHandler);
    signal(SIGTSTP, parentsigHandler); 
    signal(SIGKILL, parentsigHandler);  //These are not shared by the child processes since I am changing the group id of the child processes
    srand((unsigned int)time(NULL));
    Manager mgr;
    while(true){
        mgr.prompt();
    }
    return 0;
}