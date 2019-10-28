// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <stack>
#include "commands.h"
using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
    root = (new inode(file_type::DIRECTORY_TYPE, nullptr)) ->get_ptr();
    cwd = root;
    DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}
directory_ptr inode_state::get_cur_dir() {
    return dynamic_pointer_cast<directory> (cwd->contents);
}
inode_ptr  inode_state::get_inode_from_path(const string& path, bool ignore_last_node) {
    std::vector<std::string> sv;
    split(path, sv, '/');
    inode_ptr cursor = cwd;
    if(path.size() > 0 && path.at(0) == '/') {
        cursor = root;
    }
    size_t i = 0, j = sv.size();
    if(ignore_last_node) j--;

    for(; i < j; i++) {
        if(sv.at(i).empty() || sv.at(i) == ".") {
            continue;
        }
        directory_ptr dir = cursor->get_dir();
        if(dir->dirents.count(sv.at(i)) > 0) {
            cursor = dir->dirents[sv.at(i)];
        } else {
            return nullptr;
        }
    }
    return cursor;
}
directory_ptr  inode_state::get_dir_from_path(const string& path) {
    inode_ptr node = get_inode_from_path(path, false);
    return node == nullptr ? nullptr : node->get_dir();
};
void inode_state::update_pwd(const string &path) {
    std::vector<std::string> sv;
    split(path, sv, '/');
    if(path.size() > 0 && path.at(0) == '/') {
        pwd = path;
        return;
    } else {
        pwd += "/";
        pwd += path;
    }
}
string inode_state::get_name_from_path(const string& path) {
    std::vector<std::string> sv;
    split(path, sv, '/');
    if(sv.size() > 0) {
        return sv.at(sv.size() - 1);
    } else {
        return nullptr;
    }

}
void inode_state::update_prompt(const string& prompt) {
    prompt_ = prompt + " ";
}
void inode_state::split(const std::string& s, std::vector<std::string>& sv, const char delim = ' ') {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        sv.emplace_back(std::move(temp));
    }

    return;
}
const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type, inode_ptr parent): inode_nr (next_inode_nr++) {
    f_type = type;
    switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           inode_ptr ptr(this);
           contents = make_shared<directory>(ptr, parent == nullptr ? ptr : parent);
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
size_t inode::size() {
    return contents->size();
}
file_type inode::get_file_type() {
    return f_type;
}
directory_ptr inode::get_dir() {
    if(f_type == file_type::DIRECTORY_TYPE) {
        return dynamic_pointer_cast<directory> (contents);
    } else {
        return nullptr;
    }
}
plain_file_ptr inode::get_file() {
    if(f_type == file_type::PLAIN_TYPE) {
        return dynamic_pointer_cast<plain_file> (contents);
    } else {
        return nullptr;
    }
}
inode_ptr inode::get_ptr() {
    directory_ptr dict = get_dir();
    if(dict != nullptr) {
        return dict->dirents.at(".");
    } else {
        return nullptr;
    }
}

inode::~inode() {
    return;
}
void directory::ls(bool recursive) {
    map<string,inode_ptr>::iterator itr;
    for (itr = dirents.begin(); itr != dirents.end(); ++itr) {
        printf("%6d  %6zu  ", itr->second->get_inode_nr(), itr->second->size());
        cout<< "  "<< itr->first<< "\n";
    }
    if(recursive) {
        itr = dirents.begin();
        itr ++;
        itr ++;
        for (; itr != dirents.end(); ++itr) {
            if(itr->second->f_type == file_type::DIRECTORY_TYPE) {
                itr->second->get_dir()->ls(true);
            }
        }
    }
}

file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&, const wordvec& newdata) {
   throw file_error ("is a " + error_file_type());
}


size_t plain_file::size() const {
   size_t data_size = 0;
   if(data.size() == 0) return data_size;
   for(int i = 0; i < data.size(); i++) {
       data_size += data.at(i).size();
   }
   data_size += data.size() - 1;
   DEBUGF ('i', "size = " << data_size);
   return data_size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   data = words;
   DEBUGF ('i', words);
}

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}
directory::directory(inode_ptr cur, inode_ptr parent) {
    dirents["."] = cur;
    dirents[".."] = parent;
}
void directory::remove (const string& filename) {
    if(filename.empty() || filename.compare(".") == 0 || filename.compare("..") == 0) {
        throw command_error ("invalid argument ");
        return;
    }
    if(dirents.count(filename) <= 0) {
        throw command_error ("No such file or directory.");
    }
    if(dirents[filename]->get_file_type() == file_type::DIRECTORY_TYPE) {
        if(dirents[filename]->get_dir()->dirents.size() > 2) {
            throw command_error ("directory not empty");
        }

    }
    dirents.erase(filename);
    DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   if(dirname.empty()) {
       throw command_error ("invalid argument ");
   }
   if(dirents.count(dirname) > 0) {
       throw command_error("directory or file exists.");
   }
   inode_ptr ptr = (new inode(file_type::DIRECTORY_TYPE, dirents["."])) ->get_ptr();
   dirents[dirname] = ptr;
   DEBUGF ('i', dirname);
   return ptr;
}

inode_ptr directory::mkfile (const string& filename, const wordvec& newdata) {
   if(filename.empty()) {
       throw command_error ("invalid argument ");
   }
   if(dirents.count(filename) > 0) {
       if(dirents[filename]->f_type ==file_type::DIRECTORY_TYPE)
           throw command_error("directory with same name already exists.");
   } else {
       dirents[filename] = make_shared<inode>(file_type::PLAIN_TYPE, nullptr);
   }
   dirents[filename]->get_file()->writefile(newdata);
   DEBUGF ('i', filename);
   return nullptr;
}

