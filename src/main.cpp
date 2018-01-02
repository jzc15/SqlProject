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
            } else if (arg == "--debug_on")
            {
                debug_on = true;
            } else if (arg == "--debug_off")
            {
                debug_on = false;
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
        cerr << "Usage : " << argv[0] << " [--scan_debug] [--debug_on|--debug_off] file1" << endl;
        return 1;
    }

    for(const auto& s : files)
    {
        if (scan_debug)
        {
            cout << "Scan file '" << s << "'" << endl;
            scanFile(s.c_str());
        } else {
            if (debug_on) cout << "Parse file '" << s << "'" << endl;
            Program* p = parseFile(s.c_str());
            p->run();
            delete p;
        }
    }

    return 0;
}