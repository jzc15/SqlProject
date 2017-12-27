#include <iostream>
#include <frontend/helper.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[])
{
    vector<string> files;
    for(int i = 1; i < argc; i ++)
    {
        string arg = argv[i];
        if (arg.substr(0, 2) == "--")
        {
            if (arg == "--scan_debug")
            {
                scan_debug = true;
            } else {
                cerr << "Unknow arg : " << arg << endl;
                return 1;
            }
        } else {
            files.push_back(arg);
        }
    }
    if (files.size() == 0u)
    {
        cout << "Usage : " << argv[0] << " [--scan_debug] [file1] [file1 ...]" << endl;
    }

    for(const auto& s : files)
    {
        if (scan_debug)
        {
            cout << "Scan file '" << s << "'" << endl;
            scanFile(s.c_str());
        } else {
            cout << "Parse file '" << s << "'" << endl;
            Program* p = parseFile(s.c_str());
            p->run();
            delete p;
        }
    }

    return 0;
}