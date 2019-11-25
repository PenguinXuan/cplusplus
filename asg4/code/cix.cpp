// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $
// By: Zhuoxuan Wang (zwang437@ucsc.edu)
// and Xiong Lou (xlou2@ucsc.edu)

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"get" , cix_command::GET },
   {"put",  cix_command::PUT },
   {"rm",   cix_command::RM  },

};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_get (client_socket& server, string filename) {
    if (filename.size() >= FILENAME_SIZE)
        outlog << filename << ": filename is too long.\n";
    if (filename.find('/') != string::npos)
        outlog << filename <<
        ": filename can't have any slash characters in it.\n";

    cix_header header;
    header.command = cix_command::GET;
    strcpy(header.filename, filename.c_str());
    outlog << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    recv_packet (server, &header, sizeof header);
    outlog << "received header " << header << endl;
    if (header.command != cix_command::FILEOUT) {
        outlog << "sent GET, server did not return FILEOUT" << endl;
        outlog << "server returned " << header << endl;
    } else {
        ofstream outfile (filename, std::ios::out | std::ios::binary);
        auto buffer = make_unique<char[]> (header.nbytes + 1);
        recv_packet (server, buffer.get(), header.nbytes);
        outlog << "received " << header.nbytes << " bytes" << endl;
        buffer[header.nbytes] = '\0';
        //cout << buffer.get();
        outfile.write(buffer.get(), header.nbytes);
        outfile.close();

        outlog << "Request GET was successfully completed." << endl;
    }

}

void cix_put (client_socket& server, string filename) {
    if (filename.size() >= FILENAME_SIZE) {
        outlog << filename << ": filename is too long.\n";
        return;
    }
    if (filename.find('/') != string::npos) {
        outlog << filename <<
        ": filename can't have any slash characters in it.\n";
        return;
    }

    ifstream infile (filename, std::ios::in | ios::binary);
    const char* file = filename.c_str();
    struct stat stat_buf;
    int status = stat (file, &stat_buf);
    if (status != 0) {
        outlog << filename << ": "
               << strerror (errno) << endl;
        return;
    }

    auto buffer = make_unique<char[]> (stat_buf.st_size + 1);
    infile.read(buffer.get(), stat_buf.st_size);
    buffer[stat_buf.st_size] = '\0';

    cix_header header;
    header.command = cix_command::PUT;
    strcpy(header.filename, filename.c_str());
    header.nbytes = stat_buf.st_size;

    outlog << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    send_packet (server, &buffer, stat_buf.st_size);
    recv_packet(server, &header, sizeof header);
    outlog << "sent " << stat_buf.st_size << " bytes" << endl;
    infile.close();

    if (header.command == cix_command::NAK) {
        outlog << "sent PUT, server did not return ACK" << endl;
        outlog << "server returned " << header << endl;
    }
    if (header.command == cix_command::ACK) {
        outlog << "Request PUT was successfully completed." << endl;
    }

}

void cix_rm (client_socket& server, string filename) {
    if (filename.size() >= FILENAME_SIZE)
        outlog << filename << ": filename is too long.\n";
    if (filename.find('/') != string::npos)
        outlog << filename <<
        ": filename can't have any slash characters in it.\n";

    cix_header header;
    header.command = cix_command::RM;
    strcpy(header.filename, filename.c_str());
    header.nbytes = 0;
    outlog << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    recv_packet (server, &header, sizeof header);
    outlog << "received header " << header << endl;
    if (header.command == cix_command::NAK) {
        outlog << "sent RM, server did not return ACK" << endl;
        outlog << "server returned " << header << endl;
    }
    if (header.command == cix_command::ACK) {
        outlog << "Request RM was successfully completed." << endl;
    }
}

void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

void trim(string &line) {
    size_t first = line.find_first_not_of(' ');
    size_t last = line.find_last_not_of(' ');
    if (first != string::npos && last != string::npos) {
        line = line.substr(first, last - first + 1);
    }
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         outlog << "command " << line << endl;
         string filename;
         string command;
         smatch m;
         regex tokens ("(\\w+)\\s+(.+)");

         trim(line);
         outlog << line << endl;
         if (regex_search(line, m, tokens)) {
             command = m[1];
             filename = m[2];
         } else {
             command = line;
         }
         const auto& itor = command_map.find (command);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::GET:
               cix_get(server, filename);
               break;
            case cix_command::PUT:
               cix_put(server, filename);
               break;
            case cix_command::RM:
               cix_rm(server, filename);
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

