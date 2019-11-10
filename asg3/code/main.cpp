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
        line = line.substr(first, last - first + 1);
    }
}

void parse_line(string line, str_str_map &test) {
    str_str_map::iterator curr;
    smatch m;
    regex key ("^([^=]+)$");
    regex key_ ("([^=]+)=\\s*");
    regex key_value ("([^=]+)=(.+)");
    regex equal ("^[=]$");
    regex _value ("^=(.+)");


    trim(line);
    if (line.size() == 0) { // #
        return;
    } else if (line[0] == '#') {
        return;
    } else if (regex_search(line, m, equal)) { // =
        for(auto i = test.begin(); i != test.end(); ++i) {
            cout << i->first << " = " << i->second << endl;
        }
    } else if (regex_search(line, m, key_value)) { // key = value
        //cout << m[1] << "**" << m[2] << endl;
        str_str_pair pair(m[1], m[2]);
        curr = test.insert(pair);
        cout << curr->first + " = " + curr->second << endl;
    } else if (regex_search(line, m, key_)) { // key =
        curr = test.find(m[1]);
        if (curr != test.end()) {
            test.erase(curr);
        }
    } else if (regex_search(line, m, key)) {  // key
        curr = test.find(m[1]);
        if(curr == test.end()) {
            cout << m[1] <<": key not found" << endl;
        } else {
            cout << curr->first << " = " << curr->second << endl;

        }
    } else if (regex_search(line, m, _value)) { // = value
        for(auto i = test.begin(); i != test.end(); ++i){
            if(m[1] == i->second){
                cout << i->first << " = " << i->second << endl;
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
   str_str_map test;

   if (optind == argc) {
       while (getline(cin, line)) {
           count++;
           cout << "-" << ": " << count << ": " << line << endl;
           parse_line(line, test);

       }

   } else {
       for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
           ifstream infile;
           infile.open(*argp);
           if (infile.is_open()) {
               while (getline(infile, line)) {
                   count++;
                   cout << *argp << ": " << count << ": "
                   << line << endl;
                   parse_line(line, test);
               }
               infile.close();
           } else {
               cerr << "File not found" << endl;
           }

       }
   }
   //cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}

