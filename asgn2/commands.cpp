// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"
#include <string>
#include <iostream>
#include <sstream>

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"    , fn_rmr    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 1) {
       inode_ptr node= state.get_inode_from_path(words[1], false);
       if(node != nullptr && node->f_type == file_type::PLAIN_TYPE) {
           const wordvec &data = node->get_file()->readfile();
           cout << word_range (data.cbegin(), data.cend()) << endl;
           return;
       }
   }
   string err_msg = words[0] + ": ";
   if(words.size() > 1) {
       err_msg += words[1] + ": ";
   }
   err_msg += "No such file.";
   throw command_error (err_msg);
}

void fn_cd (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    if(words.size() > 1) {
       inode_ptr node= state.get_inode_from_path(words[1], false);
       if(node != nullptr && node->f_type == file_type::DIRECTORY_TYPE) {
           state.cwd = node;
           return;
       } else {
          string err_msg = words[0] + ": ";
          if(words.size() > 1) {
              err_msg += words[1] + ": ";
          }
          err_msg += "No such directory.";
          throw command_error (err_msg);
       }
    } else {
        state.cwd = state.root;
    }

}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    if (words.size() == 1) {
       state.get_cur_dir()->ls(false);
    } else {
       inode_ptr node = state.get_inode_from_path(words[1], false);
       if (node != nullptr && node->f_type == file_type::DIRECTORY_TYPE) {
           node->get_dir()->ls(false);
           return;
       }
        string err_msg = words[0] + ": ";
        if(words.size() > 1) {
            err_msg += words[1] + ": ";
        }
        err_msg += "No such dictionary.";
        throw command_error (err_msg);
    }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   directory_ptr cur_dir;
   if (words.size() == 1) {
       cur_dir = state.get_cur_dir();
       cur_dir->ls(true);
   } else {
       for(int i = 1; i < words.size(); i++) {
           inode_ptr node = state.get_inode_from_path(words[i], false);
           if (node == nullptr || node->f_type != file_type::DIRECTORY_TYPE) {
               string err_msg = words[0] + ": ";
               if (words.size() > 1) {
                   err_msg += words[1] + ": ";
               }
               err_msg += "No such dictionary.";
               throw command_error(err_msg);
           }
           cur_dir = node->get_dir();
           cur_dir->ls(true);
       }
   }


}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() > 1) {
       inode_ptr node = state.get_inode_from_path(words[1], true);
       if (node != nullptr && node->f_type == file_type::DIRECTORY_TYPE) {
           const string name = state.get_name_from_path(words[1]);
           wordvec data;
           copy(words.begin() + 2, words.end(), back_inserter(data));
           node->get_dir()->mkfile(name, data);
           return;
       }
   }
    string err_msg = words[0] + ": ";
    if(words.size() > 1) {
        err_msg += words[1] + ": ";
    }
    err_msg += "No such dictionary.";
    throw command_error (err_msg);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() > 1) {
       inode_ptr node = state.get_inode_from_path(words[1], true);
       if (node != nullptr && node->f_type == file_type::DIRECTORY_TYPE) {
           const string name = state.get_name_from_path(words[1]);
           node->get_dir()->mkdir(name);
           return;
       }
   }
    string err_msg = words[0] + ": ";
    if(words.size() > 1) {
        err_msg += words[1] + ": ";
    }
    err_msg += "No such dictionary.";
    throw command_error (err_msg);
}

void fn_prompt (inode_state& state, const wordvec& words){
   std::stringstream buffer;
   buffer << word_range (words.cbegin() + 1, words.cend());
   state.update_prompt(buffer.str());
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   cout << state.cwd->path << endl;
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 1) {
       inode_ptr node= state.get_inode_from_path(words[1], true);
       if(node != nullptr) {
           const string name = state.get_name_from_path(words[1]);
           node->get_dir()->remove(name, false);
           return;
       }
   }
    string err_msg = words[0] + ": ";
    if(words.size() > 1) {
        err_msg += words[1] + ": ";
    }
    err_msg += "No such file or dictionary";
    throw command_error (err_msg);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 1) {
       inode_ptr node= state.get_inode_from_path(words[1], true);
       if(node != nullptr) {
           const string name = state.get_name_from_path(words[1]);
           node->get_dir()->remove(name, true);
           return;
       }
   }
   string err_msg = words[0] + ": ";
   if(words.size() > 1) {
       err_msg += words[1] + ": ";
   }
   err_msg += "No such file or dictionary";
   throw command_error (err_msg);
}

