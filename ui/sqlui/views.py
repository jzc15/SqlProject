from django.shortcuts import render
from django.shortcuts import redirect
import os

requestfilename = "input.sql"
responsefilename = "output.txt"
cmd = '../build/main --debug_off %s > %s' % (requestfilename, responsefilename)

# Create your views here.

def readResult(s):
    sl = s.split(" :: ")
    t = []
    for x in sl:
        t.append(x)
    print(t)
    return t

def index(request):
    context = {}
    requesttext = "SHOW DATABASES;\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        responsefile = open(outfile, "r")
        # line = responsefile.readline()
        context['dblist'] = []
        while(True):
            line = responsefile.readline()
            if(len(line) <= 0):
                break
            word = line.replace('\n', '').replace('\r', '')
            context['dblist'].append(word)
    else:
        context['dblist'] = ['error']
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def database(request):
    context = {}
    requesttext = "USE " +  request.GET.get('databaseName') + ";\n"
    requesttext += "SHOW TABLES;\n"

    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        responsefile = open(outfile, "r")
        # line = responsefile.readline()
        context['tblist'] = []
    
        while(True):
            line = responsefile.readline()
            if(len(line) <= 0):
                break
            word = line.replace('\n', '').replace('\r', '')
            context['tblist'].append(word)
    else:
        context['tblist'] = ['error']
    request.session['databaseName'] =  request.GET.get('databaseName')
    context['dir'] = '/' + request.GET.get('databaseName') + '/'
    context['databaseName'] = request.GET.get('databaseName')
    return render(request, "sqlui/table.html", context)

def table(request):
    context = {}

    context['tbgroup'] = []
    context['tableName'] = request.GET.get('tableName')
    request.session['tableName'] =  request.GET.get('tableName')
    context['dir'] = '/' + request.session.get('databaseName') + '/' + request.GET.get('tableName')
    return render(request, "sqlui/result.html", context)

def deldatabase(request):
    context = {}
    requesttext = "DROP DATABASE " + request.GET.get('databaseName') + ";\nSHOW DATABASES;\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        return redirect('/')
        # responsefile = open(outfile, "r")
        # # line = responsefile.readline()
        # context['dblist'] = []
        # while(True):
        #     line = responsefile.readline()
        #     if(len(line) <= 0):
        #         break
        #     word = line.replace('\n', '').replace('\r', '')
        #     context['dblist'].append(word)
    else:
        context['dblist'] = ['error']
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def adddatabase(request):
    context = {}
    
    requesttext = request.GET.get("sql") + ";\nSHOW DATABASES;\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        return redirect('/')
        # responsefile = open(outfile, "r")
        # # line = responsefile.readline()
        # context['dblist'] = []
        # while(True):
        #     line = responsefile.readline()
        #     if(len(line) <= 0):
        #         break
        #     word = line.replace('\n', '').replace('\r', '')
        #     context['dblist'].append(word)
    else:
        context['dblist'] = ['error']
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def deltable(request):
    context = {}
    
    requesttext = "USE " +  request.session.get('databaseName') + ";\n"
    requesttext += "DROP TABLE " + request.GET.get('tableName') + ";\n"
    requesttext += "SHOW TABLES;\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        return redirect('/database/database?databaseName='+request.session.get('databaseName'))
        # responsefile = open(outfile, "r")
        # # line = responsefile.readline()
        # context['tblist'] = []
    
        # while(True):
        #     line = responsefile.readline()
        #     if(len(line) <= 0):
        #         break
        #     word = line.replace('\n', '').replace('\r', '')
        #     context['tblist'].append(word)
    else:
        context['tblist'] = ['error']
    
    context['databaseName'] = request.session.get('databaseName')
    context['dir'] = '/' + context['databaseName'] + '/'
    return render(request, "sqlui/table.html", context)

def addtable(request):
    context = {}
    
    requesttext = "USE " +  request.session.get('databaseName') + ";\n"
    requesttext += request.GET.get("sql") + ";\n"
    requesttext += "SHOW TABLES;\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        return redirect('/database/database?databaseName='+request.session.get('databaseName'))
        # responsefile = open(outfile, "r")
        # # line = responsefile.readline()
        # context['tblist'] = []
    
        # while(True):
        #     line = responsefile.readline()
        #     if(len(line) <= 0):
        #         break
        #     word = line.replace('\n', '').replace('\r', '')
        #     context['tblist'].append(word)
    else:
        context['tblist'] = ['error']
    
    context['databaseName'] = request.session.get('databaseName')
    context['dir'] = '/' + context['databaseName'] + '/'
    return render(request, "sqlui/table.html", context)

def addsql(request):
    context = {}
    
    requesttext = "USE " +  request.session.get('databaseName') + ";\n"
    requesttext += request.GET.get("sql") + ";\n"
    requestfile = open(requestfilename, "w")
    requestfile.write(requesttext)
    requestfile.close()
    
    outfile = responsefilename
    ok = os.system(cmd) == 0

    if ok:
        responsefile = open(outfile, "r")
        # lines = responsefile.

        #line = responsefile.readline()
        #hword = line.replace('\n', '').replace('\r', '')
        context['tbgroup'] = []
        tbinfo = {}
        #tbinfo['thead'] = readResult(hword)
        tbinfo['tbinfo'] = []
        while(True):
            line = responsefile.readline()
            if(len(line) <= 0):
                break
            word = line.replace('\n', '').replace('\r', '')
            #if word in hword and hword in word:
            #    context['tbgroup'].append(tbinfo)
            #    tbinfo['tbinfo'] = []
            #else:
            tbinfo['tbinfo'].append({"tbitem":readResult(word)})
        context['tbgroup'].append(tbinfo)
    else:
        tbinfo = {}
        #tbinfo['thead'] = ['error']
        tbinfo['tbinfo'] = [{"tbitem":['error']}]
        context['tbgroup'] = [tbinfo]
    context['tableName'] = request.session.get('tableName')
    request.session['tableName'] =  request.session.get('tableName')
    context['dir'] = '/' + request.session.get('databaseName') + '/' + request.session.get('tableName')
    return render(request, "sqlui/result.html", context)