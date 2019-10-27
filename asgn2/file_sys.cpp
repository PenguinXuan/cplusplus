// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <stack>
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
inode_ptr  inode_state::get_inode_from_path(const string& path) {
    std::vector<std::string> sv;
    split(path, sv, '/');
    inode_ptr cursor = cwd;
    if(path.size() > 0 && path.at(0) == '/') {
        cursor = root;
    }
    size_t i = 0;

    for(; i < sv.size(); i++) {
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
    inode_ptr node = get_inode_from_path(path);
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
    inode_ptr ptr(this);
    switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
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
void directory::ls() {
    map<string,inode_ptr>::iterator itr;
    for (itr = dirents.begin(); itr != dirents.end(); ++itr) {
        printf("%6d  %6zu  ", itr->second->get_inode_nr(), itr->second->size());
        cout<< "  "<< itr->first<< "\n";
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

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}


size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
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
        printf("invalid argument.\n");
        return;
    }
    if(dirents.count(filename) <= 0) {
        printf("file not exists.\n");
        return;
    }
    if(dirents[filename]->get_file_type() == file_type::DIRECTORY_TYPE) {
        if(dirents[filename]->get_dir()->dirents.size() > 2) {
            printf("directory not empty.\n");
            return;
        }

    }
    dirents.erase(filename);
    DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   if(dirname.empty()) {
       printf("invalid argument.\n");
       return nullptr;
   }
   if(dirents.count(dirname) > 0) {
       printf("directory exists.\n");
       return dirents[dirname];
   }
   inode_ptr ptr = (new inode(file_type::DIRECTORY_TYPE, dirents["."])) ->get_ptr();
   dirents[dirname] = ptr;
   DEBUGF ('i', dirname);
   return ptr;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   return nullptr;
}

