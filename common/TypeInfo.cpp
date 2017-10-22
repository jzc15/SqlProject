#include "TypeInfo.h"
#include <map>
#include <assert.h>

using namespace std;

static map<string, size_t> typeSize = {
    {INT_TYPE, 4},
    {CHAR_TYPE, 1},
    {VARCHAR_TYPE, 1}
};
static map<string, bool> typeFixed = {
    {INT_TYPE, true},
    {CHAR_TYPE, true},
    {VARCHAR_TYPE, false},
};

size_t TypeSize(const string& typeName)
{
    assert(typeSize.find(typeName) != typeSize.end());
    return typeSize[typeName];
}

bool IsTypeFixed(const string& typeName)
{
    assert(typeFixed.find(typeName) != typeFixed.end());
    return typeFixed[typeName];
}