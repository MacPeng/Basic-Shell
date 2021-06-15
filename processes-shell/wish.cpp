#include <iostream> // Me and Yan Wen talked about the ideas and structures of this assignment.
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>



using namespace std;

string in;
vector<string> inputStr;
vector<string> Path;

//prototype
void error(); //print error message
void execute(const vector<string> &commandStr, const vector<string> &Path); // string to char conversion
vector<string> convertStr(string in); //convert input to string
vector<vector<vector<string> >> process(vector<string> segment, const bool &sty);

void findPath(){
    Path.clear();
    Path.push_back("");
    for(size_t i = 1; i < inputStr.size(); i++){
        if(inputStr[i][0] != '/')
            inputStr[i] = "/" + inputStr[i];
        if(inputStr[i][inputStr[i].size()-1] != '/')
            inputStr[i] += "/";
        char buffer[4096];
        string cwd = getcwd(buffer, 4096);// get the pathname of the current working directory
        inputStr[i] = buffer + inputStr[i];
        Path.push_back(inputStr[i]);
    }
}

void findcd(){
    if(inputStr.size() != 2)
        error();
    else if(chdir((char*)inputStr[1].c_str()) == 0); //return zero on success and -1 on error
    else
        error();
}

void processing(vector<string> inputStr){
    bool sty;
    auto process1 = process(inputStr, sty);
    int childCount = 0;
    for(auto command : process1){
        if(command.size() == 0)
            continue;
        else{
            pid_t pid = fork();
            // child
            if(pid == 0){
                if(!(command.size() == 1 || command.size() == 3)){
                    error();
                    exit(0);
                }
                else{
                    if(command.size() == 3){
                        if(command[2].size() != 1);
                        else{
                            int oldfd = open(command[2][0].c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                            dup2(oldfd, 1);
                            dup2(oldfd, 2);
                            close(oldfd);
                            execute(command[0], Path);
                        }
                    }
                    else{
                        execute(command[0], Path);
                    }
                    error();
                    exit(0);
                }
            }
            // parent
            else{
                childCount++;
            }
        }
    }
    
    while(childCount != 0){
        wait(NULL);
        childCount--;
    }
}

int main(int argc, char *argv[]){
    Path.push_back("");
    Path.push_back("/bin/");
    
    auto interpretCommand = [&](const bool &sty){
        inputStr = convertStr(in);
        if(inputStr.size() == 0);
        else if(inputStr[0] == "exit"){
            if(inputStr.size() != 1)
                error();
            else
                exit(0);
        }
        
        else if(inputStr[0] == "cd"){
            findcd();
        }
        
        else if(inputStr[0] == "path"){
            findPath();
        }
        
        else{
            processing(inputStr);
        }
        
    };
    
    if(argc == 1){
        while(true){
            cout << "wish> ";
            getline(cin, in);
            interpretCommand(0);
        }
    }
    // batch
    else if(argc == 2){
        ifstream File(argv[1]);
        if(File.fail()){
            error();
            exit(1);
        }
        while(getline(File, in)){
            interpretCommand(0);
        }
        File.close();
    }
    else{
        error();
        exit(1);
    }
    exit(0);
}

void error(){
    cerr << "An error has occurred" << endl;
}



void execute(const vector<string> &commandStr, const vector<string> &Path){
    char *args[commandStr.size() + 1];
    for(size_t i = 0; i < commandStr.size(); i++){
        args[i] = new char[commandStr[i].size() + 1];
        strcpy(args[i], commandStr[i].c_str());
    }
    args[commandStr.size()] = NULL;
    
    for(size_t i = 0; i < Path.size(); i++){
        char commandPath[Path[i].size() + commandStr[0].size() + 1];
        strcpy(commandPath, (Path[i] + commandStr[0]).c_str());
        execv(commandPath, args);
        
    }
    error();
    exit(1);
}

vector<string> convertStr(string in){
    string word;
    istringstream strr(in);
    vector<string> convert;
    while(strr >> word){
        convert.push_back(word);
    }
    return convert;
}

vector<vector<vector<string> >> process(vector<string> segment, const bool &sty){
    
    vector<string> splitSegment; //split segment vector
    
    for(auto str : segment){
        size_t p1 = str.find(">");
        size_t p2 = str.find("&");
        while(p1 != string::npos || p2 != string::npos){
            if(p1 < p2 || p2 == string::npos){
                string string1 = str.substr(0, p1);
                str = str.substr(p1+1, str.size());
                string1.erase(remove_if( string1.begin(), string1.end(), [](char con){return isspace(con);}), string1.end());//isspace is checking if char is a white space; transforms the range [begin,end) into a range with all the elements for which pred(third argument) returns true removed, and returns an iterator to the new end of that range.
                if(!(string1.size() == 0))
                    splitSegment.push_back(string1);
                splitSegment.push_back(">");
            }
            else{
                string string1 = str.substr(0, p2);
                str = str.substr(p2+1, str.size());
                string1.erase(remove_if( string1.begin(), string1.end(), [](char con){return isspace(con);}), string1.end());
                //isspace is checking if char is a white space; transforms the range [begin,end) into a range with all the elements for which pred(third argument) returns true removed, and returns an iterator to the new end of that range.
                if(!(string1.size() == 0))
                    splitSegment.push_back(string1);
                splitSegment.push_back("&");
            }
            
            p1 = str.find(">");
            p2 = str.find("&");
        }
        if(!(str.size() == 0)) splitSegment.push_back(str);
    }
    
    
    vector<vector<string> > commandList;
    auto i = splitSegment.begin(), j = splitSegment.begin();
    while(j != splitSegment.end()){
        if(*j == "&" || *j == ">"){
            if(i != j){
                vector<string> command(i, j);
                commandList.push_back(command);
            }
            vector<string> op(j, j+1);// using each of the elements in the range [j,j+1)
            commandList.push_back(op);
            i = j+1;
        }
        j++;
    }
    if(i != splitSegment.end()){
        vector<string> command(i, splitSegment.end());
        commandList.push_back(command);
    }
    
    // check how many threads are needed
    vector<vector<vector<string> > > threadList;
    auto k = commandList.begin(), m = commandList.begin();
    while(m != commandList.end()){
        if((*m).front() == "&"){
            vector<vector<string>> command(k, m);
            threadList.push_back(command);
            k = m+1;
        }
        m++;
    }
    vector<vector<string>> command(k, commandList.end());
    threadList.push_back(command);
    return threadList;
}
