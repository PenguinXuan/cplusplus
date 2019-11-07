// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;
typedef listmap<string, string, xless<string>>::iterator itor;

void trim(string &line) {
    size_t first = line.find_first_not_of(' ');
    size_t last = line.find_last_not_of(' ');
    if (first != string::npos && last != string::npos) {
        line = line.substr(first, last-first+1);
    }
}

void parse_line(string line, str_str_map &test) {
    smatch m;
    regex key_value ("(\\w+)=(\\w+)");
    regex key_ ("(\\w+)=\\s");
    regex key ("^(\\w+)$");
    regex _value ("=(\\w+)");


    trim(line);
    if (line.at(0) == '#') { // #
        return;
    } else if (line.size() == 0) {
        return;
    } else if (line.at(0) == '=') { // =
        for(auto itor = test.begin(); itor != test.end(); ++itor) {
            cout << itor->first << " = " << itor->second << endl;
        }
    } else if (regex_search(line, m, key_value)) { // key = value
        str_str_pair pair(m[0], m[1]);
        test.insert(pair);
        cout << m[0] << " = " << m[1] << endl;
    } else if (regex_search(line, m, key_)) { // key =
        itor i = test.find(m[0]);
        if(i != test.end()) {
            i.erase();
        }
    } else if (regex_search(line, m, key)) {  // key
        itor i = test.find(m[0]);
        if(i == test.end()) {
            cout << line <<": key not found" << endl;
        } else {
            cout << i->first << " = " << i->second << endl;

        }
    } else if (regex_search(line, m, _value)) { // = value
        for(auto itor = test.begin();itor != test.end(); ++itor){
            if(m[0] == itor->second){
                cout << itor->first << " = " << itor->second << endl;
            }
        }
    }
}

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   string line = "";
   int count = 0;
   string fileName = "";

   str_str_map test;
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
       fileName = *argp;
       if (fileName.compare("-")) {
           while(!cin.eof()) {
               getline(cin,line);
               count++;
               cout << fileName << ": " << count << ": " << line <<endl;
               parse_line(line, test);
           }
       } else {
           ifstream in(fileName);
           if (in.fail()) {
               cout << "Can't open the file" << endl;
           }
           while(!in.eof()) {
               getline(in,line);
               count++;
               cout << *argp << ": " << count << ": " << line << endl;
               parse_line(line, test);
           }
       }
   }
   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}

