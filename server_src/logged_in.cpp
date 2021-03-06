#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include "server.hpp"
#include <iostream>
#include <fstream>

using namespace std;


int create_class(string classname, string username){
    /*
     * Create the entry for classname in "Classrooms"
     * Add instructor to "Classrooms/classname/instructors.txt" 
     * An empty personal directory for the course will be made, however, the instructor can access the course related data
     * of all students enrolled in the course
     * Create directories named Type 1 and Type 2
     */
    if(entry_exists("Classrooms/classrooms.txt", classname)){
        return 0;
    }
    create_entry("Classrooms", "Classrooms/classrooms.txt", classname);
    string classpath = "Classrooms/" + classname;
    create_file(classpath, "/instructors.txt");
    create_file(classpath, "/students.txt");
    add_to_file(classpath + "/instructors.txt", username);
    makedir(classpath, "type_1");
    makedir(classpath, "type_2");
    string personal_path = "Users/" + username;
    string courses_file = personal_path + "/courses.txt";
    create_entry(personal_path, courses_file, classname);
    return 1;
}

int enroll(string classname, string username){
    /*
     * Add student to "Classrooms/classname/students.txt" 
     * In "Users/username", create an entry for this course
     *
     * To be added:
     * Copy the contents of all assignments, projects, etc that have been created in the class
     */
    if(entry_exists("Classrooms/classrooms.txt", classname)){
        string classpath = "Classrooms/" + classname;
        add_to_file(classpath + "/students.txt", username);
        string personal_path = "Users/" + username;
        string courses_file = personal_path + "/courses.txt";
        create_entry(personal_path, courses_file, classname);
        personal_path += "/" + classname;
        create_file(personal_path, "categories.txt");
        classpath += "/type_1";
        vector<string> categories = list_of_entries(classpath + "/categories.txt");
        for(string cat : categories){
            create_entry(personal_path, personal_path + "/categories.txt", cat);
            vector<string> updates = list_of_entries(classpath + "/" + cat +"/updates.txt");
            create_file(personal_path + "/" + cat, "updates.txt");
            for(string upd : updates){
                create_entry(personal_path + "/" + cat, personal_path + "/" + cat + "/updates.txt", upd);
                create_file(personal_path + "/" + cat + "/" + upd, "submissions.txt");
                create_file(personal_path + "/" + cat + "/" + upd, "grades.txt");
            }
        }
        return 1;
    }
    return 0;
}

void logged_in(User *usr){
    cout<<"\nUser "<<usr->name<<" logged in\n";
    int cli_sock = usr -> cli_sock;
    string username = usr -> name;
    string login = LOGGED_IN;
    while(true){
        if((usr -> active) == false){
            break;
        }

        string header;
        string data;
        recv_data(cli_sock, header, data);
        vector<string> strings_list = split_string(header);
 
        if(strings_list[0] == "SEND"){
            int num = atoi(strings_list[1].c_str());
            if(num == 0){
                cout<<"\nCreating a course\n";
                int ret_val = create_class(data, username);
                if(ret_val == 0){
                    send_data(cli_sock, 0, login);
                    continue;
                }
                else{
                    send_data(cli_sock, 1, login);
                    continue;
                }
            }
            else if(num == 1){
                cout<<"\nEnrolling in a course\n";
                int ret_val = enroll(data, username);
                if(ret_val == 0){
                    send_data(cli_sock, 0, login);
                    continue;
                }
                else{
                    send_data(cli_sock, 1, login);
                    continue;
                }
            }
        }
        else if(strings_list[0] == "ASK"){
            int num = atoi(strings_list[1].c_str());
            if(num == 0){
                cout<<"\nDisplaying all courses\n";
                // Read classrooms.txt, append login to it
                string res = "\nList of all courses available-\n";
                res += file_contents("Classrooms/classrooms.txt");
                res += "\n--------------\n";
                res += LOGGED_IN;
                send_data(cli_sock, true, res);
            }
            else if(num == 1){
                // Enter a classroom
                cout<<"\nEntering a course\n";
                string classname = data;
                string user_courses = "Users/" + username + "/courses.txt";
                if(entry_exists(user_courses, classname)){
                    string instructor_path = "Classrooms/" + classname + "/instructors.txt";
                    if(entry_exists(instructor_path, username)){
                        send_data(cli_sock, true, INSTRUCTOR);
                        instructor(usr, classname);
                    }
                    string student_path = "Classrooms/" + classname + "/students.txt";
                    if(entry_exists(student_path, username)){
                        send_data(cli_sock, true, STUDENT);
                        student(usr, classname);
                    }
                }
                else{
                    send_data(cli_sock, false, LOGGED_IN);
                    continue;
                }
            }
            else if(num == 2){
                // Get a list of courses joined
                string res = "List of joined courses-\n";
                string filename = "Users/" + username + "/courses.txt";
                res += file_contents(filename);
                res += "\n--------------\n";
                res += LOGGED_IN;
                send_data(cli_sock, true, res);
                
            }
        }
    }
}
